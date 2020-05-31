/*********************************************
Current "Engines": 
1) Simple Play (basic keyboard)
    Knob 1 = NA
    Knob 2 = NA
2) Octave Arpeggiator 
    Knob 1 = Delay
    Knob 2 = NA

Waveforms (change with key 23 + 25): 
1)?
2) Sine
3) Triangle

Pitch Shift: 
1) ON (key 22 + 25) = Knob 3 to control
2) OFF (key 24 + 25)

Volume is on Knob 4

Screen displays (can be run without screen): 
  Mode
  Pitch (on/off)
  Knob function
  Waveform
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
//Variable and Hardware Pin Setup Here

//Setup function keys
# define KEY_25 10 //function button is key 25 connected to pin 10
int pin25;
int old25; 
int secondFunc; 
int octaveShift;
int octaveDisplay;  

//storage variables
int pitchMode; 
int mode;
char *modeString[] = {"Simple Play", "Octave Arp"};  
char *pitchString[] = {"Pitch = OFF", "Pitch = ON"}; 
char *knob1String[] = {"1 = NA", "1 = Delay"}; 
char *knob2String[] = {"2 = NA", "2 = NA"};
char *waveString[] = {"Wave = ???", "Wave = Sine", "Wave = Tri"}; 

//Setup LED
#define PP_LED 8   // LED on pocket piano

//Setup counters
int i;
int j;
int k; 
int m; 

//***************************************************
//Audio Stuff here
// define pins to read buttons
#define MUX_SEL_A 4
#define MUX_SEL_B 3
#define MUX_SEL_C 2
#define MUX_OUT_0 7
#define MUX_OUT_1 6
#define MUX_OUT_2 5

//This has low C set to F#6
// here are some 12 tone equal temperament pitches.  We will scale these with a 'tune' knob
//Set so that low C is set to C6
uint32_t miditof[] = {
  0, //need this 0 in order to have a silen note when not playing
  4188,
  4436,
  4700,
  4980,
  5276,
  5588,
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
  15804
};

//this will be a reference for octave shifting
//uint32_t miditofBase[] = {
//  0, //need this 0 in order to have a silen note when not playing
//  4188,
//  4436,
//  4700,
//  4980,
//  5276,
//  5588,
//  5920,
//  6272,
//  6645,
//  7040,
//  7459,
//  7902,
//  8372,
//  8870,
//  9397,
//  9956,
//  10548,
//  11175,
//  11840,
//  12544,
//  13290,
//  14080,
//  14918,
//  15804,
//};
// this 32 bit number holds the states of the 24 buttons, 1 bit per button
uint32_t buttons = 0xFFFFFFFF;

// holds frequency value used for oscillator in phase steps
// this is an integer proportial to Hertz in the following way:
// frequency  = (FrequencyInHertz * 65536) / SampleRate, here sample rate is 15625
//uint32_t frequency = 0;

// this is the wave form of the oscillators 1 = sine wave, 2 = triangle, 0 = sawtooth
uint8_t waveForm = 2;

// tuning knob 
int pitchScale;

//knob 1
int knob1; 

//knob 2
int knob2; 

// holds frequency value used for oscillator in phase steps
// this is an integer proportial to Hertz in the following way:
// frequency  = (FrequencyInHertz * 65536) / SampleRate, here sample rate is 15625
uint32_t frequency[] = {0, 0, 0, 0}; //setting for 4 note poly synth

// voices (up to keys held down)
uint8_t key[] = {0, 0, 0, 0}; //setting for 4 note poly synth

// octave multiplier
uint8_t octave = 0;

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
  mode = 0; 
  pitchMode = 0; 
  octaveShift = 2; 
  octaveDisplay = 0; 

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

    //Uncomment the below if you want to check if the screen is working, not necessary if not using screen
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
  
  //get starting screen
  screenUpdate(); 

  //We can print a serial message to say hi as well (useful for debugging)
  Serial.println("Hello from your synthesizer");

  //******************************************************
  //Turn LED on 
  digitalWrite(PP_LED, 0);
}


void loop(void)
{
  //knob 1 and 2 are mode dependent
  knob1 = analogRead(0); 
  knob2 = analogRead(1); 

  //pitch knob can be turned on and off
  if (pitchMode == 1) { 
    //knob 3 is for the pitch
    pitchScale = analogRead(2);
  } else { 
    pitchScale = 128; //this lets us scale to C6 when not tuning using pitch knob
  }
  
  //get the state of the buttons
  getButtons();

  //Get up to 4 keys
  k = 0;  
  key[0] = key[1] = key[2] = key[3] = 0;
  for (i = 0; i < 25; i++){             // read through buttons
    if ( !((buttons >> i) & 1) ){
      key[k] = i + 1;
      //Serial.println(key[0]);  
      k++;
      k &= 0x3;
    }
  }  

  //Find out if button 25 has been pressed
  if (pin25==1 && old25 == 0) {
    //Flash LED 
    digitalWrite(PP_LED, 1);
    delay(100);
    digitalWrite(PP_LED, 0);

    //determine if a secondary function key has been pressed for pitch or waveform
    secondFunc = key[0]; 

    if (secondFunc == 24){ //turn pitch tuning off
      pitchMode = (pitchMode+1) %2;  
    } else if (secondFunc == 22) { //change waveform
      waveForm = (waveForm+1) %3; 
//    } else if (secondFunc == 1) { 
//        octaveShift = octaveShift - 1; 
//        if (octaveShift < 0) { 
//          octaveShift = 0 ;
//        }
//        octShiftFunc();
//    } else if (secondFunc == 3) {
//        octaveShift = octaveShift + 1; 
//        if (octaveShift > 4) { 
//          octaveShift = 4 ;
//        }
//        octShiftFunc();       
    } else { //change mode if only button 25 is pressed
      //increase counter
      mode = (mode+1) %2; //counter only goes 0,1 now! 
    }
    screenUpdate();
  }

//  if (octaveDisplay > 0) { 
//    miditof[key[0]] = miditofBase[key[0]] << octaveDisplay; 
//    miditof[key[1]] = miditofBase[key[1]] << octaveDisplay; 
//    miditof[key[2]] = miditofBase[key[2]] << octaveDisplay; 
//    miditof[key[3]] = miditofBase[key[3]] << octaveDisplay; 
//  } //else if (octaveDisplay < 0) { 
//    miditof[key[0]] = miditofBase[key[0]] >> abs(octaveDisplay); 
//    miditof[key[1]] = miditofBase[key[1]] >> abs(octaveDisplay);
//    miditof[key[2]] = miditofBase[key[2]] >> abs(octaveDisplay);
//    miditof[key[3]] = miditofBase[key[3]] >> abs(octaveDisplay);
//  } else { 
//    miditof[key[0]] = miditofBase[key[0]];
//    miditof[key[1]] = miditofBase[key[1]];
//    miditof[key[2]] = miditofBase[key[2]];
//    miditof[key[3]] = miditofBase[key[3]];
//  } 
  
  //actually playing the notes! 
  if (mode == 0) { //simple play mode, only pitch is affected
    frequency[0] = (((miditof[key[0]]) * (pitchScale))>> 9);  // scale frequency by tuning knob and octave
    frequency[1] = (((miditof[key[1]])  * (pitchScale))>> 9);
    frequency[2] = (((miditof[key[2]])  * (pitchScale))>> 9);
    frequency[3] = (((miditof[key[3]])  * (pitchScale))>> 9); 
    delay(10);   // wait 10 ms
  
  } else { //octave arp mode    
    // do octave increase
    octave++;
    
    // only from 0 - 3
    octave &= 3;
  
    frequency[0] = ((((miditof[key[0]]) * (pitchScale)) >> 9)>> octave);  // scale frequency by tuning knob and octave
    frequency[1] = ((((miditof[key[1]]) * (pitchScale)) >> 9)>> octave);
    frequency[2] = ((((miditof[key[2]]) * (pitchScale)) >> 9)>> octave);
    frequency[3] = ((((miditof[key[3]]) * (pitchScale)) >> 9)>> octave); 
    delay(knob1 >> 2);   // sweep speed
  }
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
  //buttons &= ~0x1000000;
 
  Serial.println(buttons);
  old25 = pin25; 
  pin25 = digitalRead(KEY_25);
}

//void octShiftFunc(void) { 
//  if (octaveShift == 0) { 
//    octaveDisplay = -2; 
//  } else if (octaveShift == 1) { 
//    octaveDisplay = -1; 
//  } else if (octaveShift == 2) { 
//    octaveDisplay = 0; 
//  } else if (octaveShift == 3) { 
//    octaveDisplay = 1; 
//  } else { 
//    octaveDisplay = 2; 
//  } 
//} 

void screenUpdate(void) { 
  //Draw starting mode
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextWrap(1); 
  display.println(modeString[mode]);
  display.print(pitchString[pitchMode]);
  display.print("   "); 
  display.print("Oct: "); 
  display.println(octaveDisplay); 
  display.print(knob1String[mode]); 
  display.print("   "); 
  display.println(knob2String[mode]);
  display.println(waveString[waveForm]);  
  display.display();
}
