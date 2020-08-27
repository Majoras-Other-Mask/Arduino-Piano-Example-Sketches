/*********************************************
   Plays Sine Tone on Arduino Pocket Piano
   Author: Owen Osborn, Copyright: GPL
 *********************************************/
//Screen Setup Here
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     A3 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//***************************************************
//Misc. Setup Here

//Setup function key
# define KEY_25 10 //function button is key 25 connected to pin 10
int pin25;
int old25; 
int counter;  

//Setup LED
#define PP_LED 8   // LED on pocket piano

//Setup counters
int i;
int j;

//***************************************************
//Audio Stuff here
// define pins to read buttons
#define MUX_SEL_A 4
#define MUX_SEL_B 3
#define MUX_SEL_C 2
#define MUX_OUT_0 7
#define MUX_OUT_1 6
#define MUX_OUT_2 5

// here are some 12 tone equal temperament pitches.  We will scale these with a 'tune' knob
uint32_t miditof[] = {
  5920,
  6272,
  6645,
  7040,
  7459,
  7902,
  8372,
  8870,
  9397,
  9956,
  10548,
  11175,
  11840,
  12544,
  13290,
  14080,
  14918,
  15804,
  16744,
  17740,
  18794,
  19912,
  21096,
  22350,
  23680,
  25088
};

// this 32 bit number holds the states of the 24 buttons, 1 bit per button
uint32_t buttons = 0xFFFFFFFF;

// holds frequency value used for oscillator in phase steps
// this is an integer proportial to Hertz in the following way:
// frequency  = (FrequencyInHertz * 65536) / SampleRate, here sample rate is 15625
uint32_t frequency = 0;



void setup() {
  //******************************************************
  //Audio Setup happens here and needs to be first

  //Timer2 setup  This is the audio rate timer, fires an interrupt at 15625 Hz sampling rate
  TIMSK2 = 1 << OCIE2A; // interrupt enable audio timer
  OCR2A = 127;
  TCCR2A = 2;               // CTC mode, counts up to 127 then resets
  TCCR2B = 0 << CS22 | 1 << CS21 | 0 << CS20; // different for atmega8 (no 'B' i think)

  SPCR = 0x50;   // set up SPI port
  SPSR = 0x01;
  DDRB |= 0x2E;       // PB output for DAC CS, and SPI port
  PORTB |= (1 << 1); // CS high

  sei();			// global interrupt enable

  // configure pins for multiplexer
  pinMode(MUX_SEL_A, OUTPUT);  // these are the select pins
  pinMode(MUX_SEL_B, OUTPUT);
  pinMode(MUX_SEL_C, OUTPUT);


  pinMode(MUX_OUT_0, INPUT);
  pinMode(MUX_OUT_1, INPUT);
  pinMode(MUX_OUT_2, INPUT);

  digitalWrite(MUX_SEL_A, 1);   // multiplexer outputs, 8 each
  digitalWrite(MUX_SEL_B, 1);
  digitalWrite(MUX_SEL_C, 1);

  //******************************************************
  //Misc. Setup happens here
  pinMode(PP_LED, OUTPUT); //setup LED
  //pinMode(KEY_25, INPUT); //pin works as an input but if I uncomment this line everything breaks ¯\_(ツ)_/¯ 
  i = 0; 
  j = 0; 
  pin25 = 0; 
  counter = 0; 

  //flash led to let us know it is turning on
  digitalWrite(PP_LED, 1);
  delay(100);
  digitalWrite(PP_LED, 0);
  delay(100);
  digitalWrite(PP_LED, 1);
  delay(100);
  digitalWrite(PP_LED, 0);
  delay(100);
  digitalWrite(PP_LED, 1);

  Serial.begin(9600);

  //******************************************************
  //Screen Setup happens here

  //  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;) {
      // Don't proceed, loop forever if screen not found
      digitalWrite(PP_LED, 1);
      delay(100);
      digitalWrite(PP_LED, 0);
      delay(100);
    }
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw welcome text so we know we are in business
  display.setCursor(5, 5);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextWrap(1); 
  display.println("Welcome to synthesizer");
  display.display();

  //We can print a serial message to say hi as well
  Serial.println("hello");
  Serial.println("welcome to synthesizer");

  //******************************************************
  //Turn LED on 
  digitalWrite(PP_LED, 0);

  //Draw starting mode
  display.clearDisplay();
  display.setCursor(5, 5);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextWrap(1); 
  display.println(counter);
  display.display();

}


void loop(void)
{
  //get the state of the buttons
  getButtons();

  //Find out if button 25 has been pressed
  if (pin25==1 && old25 == 0) {
    //Flash LED 
    digitalWrite(PP_LED, 1);
    delay(100);
    digitalWrite(PP_LED, 0);
    
    //increase counter
    counter = (counter+1) %3; //counter only goes 0,1,2 now! 
    // Clear the buffer
    display.clearDisplay();
  
    // Draw mode text
    display.setCursor(5, 5);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextWrap(1); 
    display.println(counter);
    display.display();
  }

  //play notes! 
  for (i = 0; i < 25; i++) {
    if ( !((buttons >> i) & 1)) {   // check status of each bit in the buttons variable, if it is 0, then a button  is down
      frequency = miditof[i]  >> 2;
      break;   // leave the loop if a button was down
    }
    else
      frequency = 0;
  }

  delay(10);   // wait 10 ms
}

// this funcion reads the buttons and stores their states in the global 'buttons' variable
void getButtons(void) {
  buttons = 0;
  for (j = 0; j < 8; j++) {
    digitalWrite(MUX_SEL_A, j & 1);
    digitalWrite(MUX_SEL_B, (j >> 1) & 1);
    digitalWrite(MUX_SEL_C, (j >> 2) & 1);
    buttons |= digitalRead(MUX_OUT_2) << j;
  }
  buttons <<= 8;
  for (j = 0; j < 8; j++) {
    digitalWrite(MUX_SEL_A, j & 1);
    digitalWrite(MUX_SEL_B, (j >> 1) & 1);
    digitalWrite(MUX_SEL_C, (j >> 2) & 1);
    buttons |= digitalRead(MUX_OUT_1) << j;
  }
  buttons <<= 8;
  for (j = 0; j < 8; j++) {
    digitalWrite(MUX_SEL_A, j & 1);
    digitalWrite(MUX_SEL_B, (j >> 1) & 1);
    digitalWrite(MUX_SEL_C, (j >> 2) & 1);
    buttons |= digitalRead(MUX_OUT_0) << j;
  }
  buttons |= 0x1000000;  // for the 25th button

  old25 = pin25; 
  pin25 = digitalRead(KEY_25);
}
