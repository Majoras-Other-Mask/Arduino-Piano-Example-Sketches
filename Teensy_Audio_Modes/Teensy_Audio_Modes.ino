#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine1;          //xy=251,681
AudioSynthWaveform       waveform2;      //xy=255,359
AudioSynthWaveform       waveform1;      //xy=257,287
AudioSynthWaveform       waveform3;      //xy=260,434
AudioSynthWaveform       waveform4;      //xy=262,521
AudioEffectEnvelope      envelope1;      //xy=413,286
AudioEffectEnvelope      envelope2;      //xy=414,359
AudioEffectEnvelope      envelope3;      //xy=415,432
AudioAnalyzeRMS          rms1;           //xy=414,682
AudioEffectEnvelope      envelope4;      //xy=415,523
AudioMixer4              mixer1;         //xy=657,377
AudioAmplifier           amp1;           //xy=783,378
AudioEffectBitcrusher    bitcrusher1;    //xy=987,376
AudioOutputAnalogStereo  dacs1;          //xy=1232,377
AudioConnection          patchCord1(sine1, rms1);
AudioConnection          patchCord2(waveform2, envelope2);
AudioConnection          patchCord3(waveform1, envelope1);
AudioConnection          patchCord4(waveform3, envelope3);
AudioConnection          patchCord5(waveform4, envelope4);
AudioConnection          patchCord6(envelope1, 0, mixer1, 0);
AudioConnection          patchCord7(envelope2, 0, mixer1, 1);
AudioConnection          patchCord8(envelope3, 0, mixer1, 2);
AudioConnection          patchCord9(envelope4, 0, mixer1, 3);
AudioConnection          patchCord10(mixer1, amp1);
AudioConnection          patchCord11(amp1, bitcrusher1);
AudioConnection          patchCord12(bitcrusher1, 0, dacs1, 1);
AudioConnection          patchCord13(bitcrusher1, 0, dacs1, 0);
// GUItool: end automatically generated code



AudioSynthWaveform *waves[4] = {&waveform1, &waveform2, &waveform3, &waveform4}; 

//create an array of envelopes to use later
AudioEffectEnvelope *envs[4] = {&envelope1, &envelope2, &envelope3, &envelope4};  


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
String modeString[] = {"Vibrato Synth", "Harmonic Sweeper", "Octave Arpeggiator", "Octave Cascade", "Bitcrusher"};  
String pitchString[] = {"Pitch = OFF", "Pitch = ON "}; 
String knob1String[] = {"1 = Depth", "1 = Rate", "1 = Rate", "1 = Rate", "1 = Bits"}; 
String knob2String[] = {"2 = Rate", "2 = Env.", "2 = Env.", "2 = Decay", "2 = Rate"};
String waveString[] = {"SINE", "SAW", "REVERSE SAW", "SQUARE", "TRIANGLE", "TRIANGLE VARIABLE",
                      "PULSE"}; 

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

float freqBase[] = { 
  0, 
  261.63, 
  277.18, 
  293.66, 
  311.13, 
  329.63, 
  349.23, 
  369.99, 
  392.00, 
  415.30, 
  440.00, 
  466.16, 
  493.88, 
  523.25, 
  554.37, 
  587.33, 
  622.25, 
  659.25, 
  698.46, 
  739.99, 
  783.99, 
  830.61, 
  880.00, 
  932.33, 
  987.77
//  1046.50, 
//  1108.73,
//  1174.66, 
//  1244.51, 
//  1318.51, 
//  1396.91, 
//  1479.98, 
//  1567.98, 
//  1661.22, 
//  1760.00, 
//  1864.66, 
//  1975.53
};

// this 32 bit number holds the states of the 24 buttons, 1 bit per button
uint32_t buttons = 0xFFFFFFFF;

// this is the wave form of the oscillators 
int waveForm; 

// tuning knob 
float pitchScale;

//knob 1
int knob1; 

//knob 2
int knob2; 

// holds frequency value used to play
float freqPlay[] = {0, 0, 0, 0}; //setting for 4 note poly synth
float freqLast[] = {0, 0, 0, 0}; //save last fequency

// voices (up to keys held down)
uint8_t key[] = {0, 0, 0, 0}; //setting for 4 note poly synth

// octave multiplier
uint8_t octave = 0; 

// arpeggiator counter
int arp = 0; 

// harmonic sweep counter
int sweep = 0; 

float two12th = 1.059463; //used for arp. calculations

float vibrato = 0.0; //used for vibrato effect 

//envelope variables
float ADSR_a;
float ADSR_d;
float ADSR_s;
float ADSR_r; 
float ADSR_hold; 
float envMult; 


//******************************************************
int current_waveform=0;

extern const int16_t myWaveform[256];  // defined in myWaveform.ino

//bit crusher
int bitsPassthrough = 16; 
int bitsRatePassthrough = 44100; 

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
  Serial.begin(57600);

  //******************************************************
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(40);
  for (int z=0; z<4; z++) { 
    waves[z]->frequency(0); 
    waves[z]->amplitude(1.0); 
    waves[z]->begin(current_waveform); 
  }
  
  //mixer1 is for mixing notes being played
  mixer1.gain(0, 0.25); 
  mixer1.gain(1, 0.25); 
  mixer1.gain(2, 0.25); 
  mixer1.gain(3, 0.25); 

  amp1.gain(2); 

  //default envelope variables
  ADSR_a = 9.2; 
  ADSR_hold = 2.1; 
  ADSR_d = 31.4; 
  ADSR_s = 0.6; 
  ADSR_r = 200; 
  // Setup Enevelopes
  for (int z=0; z<4; z++) {
    envs[i]->attack(ADSR_a);
    envs[i]->hold(ADSR_hold);
    envs[i]->decay(ADSR_d);
    envs[i]->sustain(ADSR_s);
    envs[i]->release(ADSR_r); 
  } 

  // Setup Sine wave for vibrato mode
  sine1.amplitude(1); 
  sine1.frequency(5);
  
  //setup bitcrusher
  bitcrusher1.bits(bitsPassthrough); 
  bitcrusher1.sampleRate(bitsRatePassthrough); 
  
  //******************************************************
  //Screen Setup happens here

  //Uncomment the below if you want to check if the screen is working, not necessary if not using screen
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
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
    pitchScale = 0.25 + 3.75*(analogRead(A2)/1023.0);
  } else { 
    pitchScale = 1.0; //this lets us scale to C6 when not tuning using pitch knob
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
  for (n = 0; n < 4; n++) { 
    freqLast[n] = freqPlay[n]; 
  }

    
  //apply octave shift
  if (octaveDisplay > 0) { 
    for (n = 0; n < 4; n++) { 
      freqPlay[n] = freqBase[key[n]] * 2 * octaveDisplay;
    }
  } else if (octaveDisplay < 0) { 
    for (n = 0; n < 4; n++) { 
      freqPlay[n] = freqBase[key[n]] / (2* abs(octaveDisplay));
    }
  } else { 
    for (n = 0; n < 4; n++) { 
      freqPlay[n] = freqBase[key[n]];
    }
  }

  if (mode == 0) { 
    //Vibrato Synth
    //knob 1 = depth
    //knob 2 = rate

    sine1.amplitude(knob1 / 1023.0); 
    sine1.frequency(10 * knob2 / 1023.0);  

    vibrato = 1.0 - rms1.read(); 

    for (int z=0; z<4; z++) { 
      waves[z]->amplitude(vibrato); 
    }
    
    
    delay(10); 

  } else if (mode == 1) { 
    // Harmonic Sweeper
    //knob 1 = rate
    //knob 2 = envelope scaled
    sweep = (sweep + 1) % 8; 
    for (n=0; n<4; n++) { 
      if (sweep == 1) { 
        freqPlay[n] = freqPlay[n]*2.0; 
      } else if (sweep == 2) { 
        freqPlay[n] = freqPlay[n]*3.0;
      } else if (sweep == 3) { 
        freqPlay[n] = freqPlay[n]*4.0;
      } else if (sweep == 4) { 
        freqPlay[n] = freqPlay[n]*5.0;
      } else if (sweep == 5) { 
        freqPlay[n] = freqPlay[n]*6.0;
      } else if (sweep == 6) { 
        freqPlay[n] = freqPlay[n]*7.0; 
      } else if (sweep == 7) { 
        freqPlay[n] = freqPlay[n]*8.0; 
      }
    } 


    Serial.println(float(ADSR_a*envMult)); 
    for (n = 0; n < 4; n++) { 
      freqPlay[n] = freqPlay[n] * (octave+1); 
    }

    //control envelope with knob 2
    envMult = knob2 / 1023.0;
    envelope1.attack(80* envMult); 
    envelope2.attack(80 * envMult);
    envelope3.attack(80 * envMult);
    envelope4.attack(80 * envMult);
    envelope1.hold(24 * envMult);
    envelope2.hold(24 * envMult);
    envelope3.hold(24 * envMult);
    envelope4.hold(24 * envMult);
    envelope1.decay(100 * envMult);
    envelope2.decay(100 * envMult);
    envelope3.decay(100 * envMult);
    envelope4.decay(100 * envMult);

    //control sweep speed with knob 1
    delay(int(knob1 >> 2) + 50);   // sweep speed

  } else if (mode == 2) { 
    // Octave Arpeggiator
    //knob 1 = rate
    //knob 2 = envelope scaled
    arp = (arp + 1) % 7; 
    for (n=0; n<4; n++) { 
      if (arp == 1) { 
        freqPlay[n] = freqPlay[n]*pow(two12th,4); 
      } else if (arp == 2) { 
        freqPlay[n] = freqPlay[n]*pow(two12th,7);
      } else if (arp == 3) { 
        freqPlay[n] = freqPlay[n]*2.0;
      } else if (arp == 4) { 
        freqPlay[n] = freqPlay[n]*2*pow(two12th,4);
      } else if (arp == 5) { 
        freqPlay[n] = freqPlay[n]*2*pow(two12th,7);
      } else if (arp == 6) { 
        freqPlay[n] = freqPlay[n]*4; 
      }
    } 

    
    for (n = 0; n < 4; n++) { 
      freqPlay[n] = freqPlay[n] * (octave+1); 
      envs[n]->decay(255 * knob2 / 1023.0);     
      }
      
    //control envelope with knob 2
    envMult = knob2 / 1023.0;
    envelope1.attack(80* envMult); 
    envelope2.attack(80 * envMult);
    envelope3.attack(80 * envMult);
    envelope4.attack(80 * envMult);
    envelope1.hold(24 * envMult);
    envelope2.hold(24 * envMult);
    envelope3.hold(24 * envMult);
    envelope4.hold(24 * envMult);
    envelope1.decay(100 * envMult);
    envelope2.decay(100 * envMult);
    envelope3.decay(100 * envMult);
    envelope4.decay(100 * envMult);

    //control sweep speed with knob 1
    delay(int(knob1 >> 2) + 50);   // sweep speed

        
  } else if (mode == 3) { 
    // Octave Cascade
    //knob 1 = rate
    //knob 2 = envelope decay
    octave = (octave + 1) % 3;
    for (n = 0; n < 4; n++) { 
      freqPlay[n] = freqPlay[n] * (octave+1); 
      envs[n]->decay(255 * knob2 / 1023.0);     
      }
    delay(int(knob1 >> 2) + 50);   // sweep speed
  } else if (mode == 4) { 
    //bit crusher
    //knob 1 = bits
    //knob 2 = sampling rate
    //we change the bits/sample rate based on knob 1 and 2
    bitcrusher1.bits(int(15 * knob1/1023.0)+1); 
    bitcrusher1.sampleRate(int(44095 * knob2 / 1023.0)+5); 
    delay(10); 
  }

  AudioNoInterrupts();
  for (n = 0; n < 4; n++) { 
    freqPlay[n] = freqPlay[n]*pitchScale;
    if (freqLast[n] != freqPlay[n]){ 
      envs[n] -> noteOff(); 
      waves[n] -> frequency(freqPlay[n]);
      envs[n] ->noteOn();      
    }
  }
  AudioInterrupts(); 
}
