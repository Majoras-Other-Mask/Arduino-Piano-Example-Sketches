// Waveform Example

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

AudioSynthWaveform       waveform1;      //xy=171,84
AudioOutputAnalogStereo  dacs1;          //xy=372,173
AudioConnection          patchCord1(waveform1, 0, dacs1, 0);
AudioConnection          patchCord2(waveform1, 0, dacs1, 1);

//*********************************************/
//Screen Setup Here
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
String waveString[] = {"SINE", "SAW", "REVERSE SAW", "SQUARE", "TRIANGLE", "TRIANGLE VARIABLE",
                      "ARBITRARY", "PULSE"}; 

//Setup LED
#define PP_LED 8   // LED on pocket piano

//Setup counters
int i;
int j;
int k; 
int n; 

//***************************************************
//Audio Backend Stuff Here
// define pins to read buttons
#define MUX_SEL_A 4
#define MUX_SEL_B 3
#define MUX_SEL_C 2
#define MUX_OUT_0 7
#define MUX_OUT_1 6
#define MUX_OUT_2 5

uint32_t freqBase[] = { 
  0, 
  1047, 
  1109,
  1175, 
  1245, 
  1319, 
  1397, 
  1480, 
  1568, 
  1661, 
  1760, 
  1865, 
  1976, 
  2093, 
  2217, 
  2349, 
  2489, 
  2637,
  2794, 
  3136, 
  3322,
  3520, 
  3729, 
  3951
};

// this 32 bit number holds the states of the 24 buttons, 1 bit per button
uint32_t buttons = 0xFFFFFFFF;

// this is the wave form of the oscillators 
int waveForm; 

// tuning knob 
int pitchScale;

//knob 1
int knob1; 

//knob 2
int knob2; 

// holds frequency value used to play
uint32_t freqPlay[] = {0, 0, 0, 0}; //setting for 4 note poly synth

// voices (up to keys held down)
uint8_t key[] = {0, 0, 0, 0}; //setting for 4 note poly synth

// octave multiplier
uint8_t octave = 0; 


//******************************************************
int current_waveform=0;

extern const int16_t myWaveform[256];  // defined in myWaveform.ino

void setup() {
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
  waveForm = 0; //starting waveform 

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
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(10);

  // Confirgure both to use "myWaveform" for WAVEFORM_ARBITRARY
  waveform1.arbitraryWaveform(myWaveform, 172.0);

  // configure both waveforms for 440 Hz and maximum amplitude
  waveform1.frequency(0);
  waveform1.amplitude(1.0);
  
  current_waveform = WAVEFORM_SINE;
  waveform1.begin(current_waveform);
    
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

void loop() {
  //******************************************************
  //Read the knob values
  //knob 1 and 2 are mode dependent
  knob1 = analogRead(A0); 
  knob2 = analogRead(A1); 

  //pitch control can be turned on and off
  if (pitchMode == 1) { 
    //knob 3 is for the pitch
    pitchScale = analogRead(A2);
  } else { 
    pitchScale = 128; //this lets us scale to C6 when not tuning using pitch knob
  }

  //******************************************************
  //get the state of the buttons, outputs are ints 0-24
  //and info on button 25
  getButton();

  //Get up to 4 keys that were pressed down
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

  //******************************************************
  //get frequencies we are going to play
    
  //apply octave shift
  if (octaveDisplay > 0) { 
    for (n = 0; n < 4; n++) { 
      freqPlay[n] = freqBase[key[n]] << octaveDisplay;
    }
  } else if (octaveDisplay < 0) { 
    for (n = 0; n < 4; n++) { 
      freqPlay[n] = freqBase[key[n]] >> abs(octaveDisplay);
    }
  } else { 
    for (n = 0; n < 4; n++) { 
      freqPlay[n] = freqBase[key[n]];
    }
  }
  
  // Read the buttons and knobs, scale knobs to 0-1.0
 // button0.update();
//  float knob_A2 = (float)analogRead(A2) / 1023.0;

  AudioNoInterrupts();
  // use Knob A2 to adjust the frequency of both waveforms
//  waveform1.frequency(100.0 + knob_A2 * 900.0);
  waveform1.frequency(freqPlay[0]);
  AudioInterrupts();
  
}
