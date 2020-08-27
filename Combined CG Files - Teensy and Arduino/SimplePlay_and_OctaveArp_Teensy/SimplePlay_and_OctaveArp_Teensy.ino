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
//Create interval timer object
IntervalTimer myTimer; 

 //*********************************************/
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
//IDK why I need this but it currently is not working else a digital output 
//pin is connected to amp Vin
#define RAND 0

//Slave Select Pin on Chip
#define slaveSelectPin 9 

//Setup function keys
#define KEY_25 10 //function button is key 25 connected to pin 10
int pin25;
int old25; 
int secondFunc; 
int octaveShift;
int octaveDisplay;  

//storage variables
int pitchMode; 
int mode;
String modeString[] = {"Simple Play", "Octave Arp", "FM"};  
String pitchString[] = {"Pitch = OFF", "Pitch = ON "}; 
String knob1String[] = {"1 = NA", "1 = Delay", "1 = Harm." }; 
String knob2String[] = {"2 = NA", "2 = NA", "2 = Depth"};
String waveString[] = {"Wave = Saw", "Wave = Sine", "Wave = Triangle", "Wave = Square", "Wave = Pulse", "Wave = Fuzz",
                      "Wave = Digital", "Wave = Tan"}; 

//Setup LED
#define PP_LED 8   // LED on pocket piano

//Setup counters
int i;
int j;
int k; 
int n; 
 

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
uint32_t miditofBase[] = {
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
  15804,
};
// this 32 bit number holds the states of the 24 buttons, 1 bit per button
uint32_t buttons = 0xFFFFFFFF;

// holds frequency value used for oscillator in phase steps
// this is an integer proportial to Hertz in the following way:
// frequency  = (FrequencyInHertz * 65536) / SampleRate, here sample rate is 15625
//uint32_t frequency = 0;

// this is the wave form of the oscillators 
uint8_t waveForm;

// tuning knob 
int pitchScale;

//knob 1
int knob1; 

//knob 2
int knob2; 

//gain
uint8_t gn = 0xff; 

// holds frequency value used for oscillator in phase steps
// this is an integer proportial to Hertz in the following way:
// frequency  = (FrequencyInHertz * 65536) / SampleRate, here sample rate is 15625
uint32_t freq[] = {0, 0, 0, 0}; //setting for 4 note poly synth

// voices (up to keys held down)
uint8_t key[] = {0, 0, 0, 0}; //setting for 4 note poly synth

// octave multiplier
uint8_t octave = 0;

void setup() {
  //******************************************************
  //Timer Setup
  myTimer.priority(0); 
  myTimer.begin(sendNote, 64); 
  
  //******************************************************
  //SPI Setup
  pinMode(RAND, OUTPUT);
  pinMode(slaveSelectPin, OUTPUT); 
  SPI.usingInterrupt(myTimer);  
  SPI.begin();  

  //******************************************************
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
  pinMode(KEY_25, OUTPUT); 
  //pinMode(KEY_25, INPUT); //pin works as an input but if I uncomment this line everything breaks ¯\_(ツ)_/¯ 
  i = 0; 
  j = 0; 
  pin25 = 0; 
  mode = 0; 
  pitchMode = 0; 
  octaveShift = 2; 
  octaveDisplay = 0; 
  waveForm = 1; //starting waveform 

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

  //Turning on serial for debugging
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

  //******************************************************
  //We can print a serial message to say hi as well (useful for debugging)
  Serial.println("Hello from your synthesizer");

  //Turn LED on to let us know we are ready to start
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

  //secondFunc key is alwasys the lowest note played
  secondFunc = key[0]; 
  //call function key function
  key25Func(); 
  
  //apply octave shift
  if (octaveDisplay > 0) { 
    for (n = 0; n < 4; n++) { 
      miditof[key[n]] = miditofBase[key[n]] << octaveDisplay;
    }
  } else if (octaveDisplay < 0) { 
    for (n = 0; n < 4; n++) { 
      miditof[key[n]] = miditofBase[key[n]] >> abs(octaveDisplay);
    }
  } else { 
    for (n = 0; n < 4; n++) { 
      miditof[key[n]] = miditofBase[key[n]];
    }
  } 
  
  //actually playing the notes! 
  if (mode == 1) { //octave arp mode    
    // do octave increase
    octave++;
    // only from 0 - 3
    octave &= 3;
    for (n = 0; n < 4; n++) { 
      freq[n] =  ((((miditof[key[n]]) * (pitchScale)) >> 9)>> octave);
    }
    delay(knob1 >> 2);   // sweep speed
  } else if (mode == 2) { //simple FM synth
    for (n = 0; n < 4; n++) {  
      freq[n] =  ((miditof[key[n]]) * (pitchScale)) >> 12;
    }
    delay(10); 
  } else { //simple play mode, only pitch is affected
        for (n = 0; n < 4; n++) { 
      freq[n] =  ((miditof[key[n]]) * (pitchScale)) >> 9;
    }
    delay(10);   // wait 10 ms 
  }
}
