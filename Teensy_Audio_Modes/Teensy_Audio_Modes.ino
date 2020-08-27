#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine1;          //xy=110,783
AudioSynthWaveformModulated waveformMod4;   //xy=222,639
AudioSynthWaveformModulated waveformMod2;   //xy=232,323
AudioSynthWaveformModulated waveformMod1;   //xy=234,188
AudioSynthWaveformModulated waveformMod3;   //xy=233,478
AudioSynthWaveform       waveform4;      //xy=237,575
AudioSynthWaveform       waveform2;      //xy=239,271
AudioSynthWaveform       waveform3;      //xy=244,423
AudioSynthWaveform       waveform1;      //xy=249,134
AudioAnalyzeRMS          rms1;           //xy=324,778
AudioMixer4              mixer5;         //xy=444,589
AudioMixer4              mixer4;         //xy=445,439
AudioMixer4              mixer2;         //xy=447,163
AudioMixer4              mixer3;         //xy=447,296
AudioEffectEnvelope      envelope4;      //xy=639,585
AudioEffectEnvelope      envelope2;      //xy=642,308
AudioEffectEnvelope      envelope1;      //xy=643,212
AudioEffectEnvelope      envelope3;      //xy=647,434
AudioMixer4              mixer1;         //xy=892,375
AudioFilterStateVariable filter1;        //xy=1015,380
AudioAmplifier           amp1;           //xy=1142,374 
AudioOutputAnalog        dac1;           //xy=1280,371
AudioConnection          patchCord1(sine1, rms1);
AudioConnection          patchCord2(sine1, 0, waveformMod4, 0);
AudioConnection          patchCord3(sine1, 0, waveformMod3, 0);
AudioConnection          patchCord4(sine1, 0, waveformMod2, 0);
AudioConnection          patchCord5(sine1, 0, waveformMod1, 0);
AudioConnection          patchCord6(waveformMod4, 0, mixer5, 1);
AudioConnection          patchCord7(waveformMod2, 0, mixer3, 1);
AudioConnection          patchCord8(waveformMod1, 0, mixer2, 1);
AudioConnection          patchCord9(waveformMod3, 0, mixer4, 1);
AudioConnection          patchCord10(waveform4, 0, mixer5, 0);
AudioConnection          patchCord11(waveform2, 0, mixer3, 0);
AudioConnection          patchCord12(waveform3, 0, mixer4, 0);
AudioConnection          patchCord13(waveform1, 0, mixer2, 0);
AudioConnection          patchCord14(mixer5, envelope4);
AudioConnection          patchCord15(mixer4, envelope3);
AudioConnection          patchCord16(mixer2, envelope1);
AudioConnection          patchCord17(mixer3, envelope2);
AudioConnection          patchCord18(envelope4, 0, mixer1, 3);
AudioConnection          patchCord19(envelope2, 0, mixer1, 1);
AudioConnection          patchCord20(envelope1, 0, mixer1, 0);
AudioConnection          patchCord21(envelope3, 0, mixer1, 2);
AudioConnection          patchCord22(mixer1, 0, filter1, 0);
AudioConnection          patchCord23(filter1, 2, amp1, 0);
AudioConnection          patchCord24(amp1, dac1); 
// GUItool: end automatically generated code



AudioSynthWaveformModulated *wavesFM[4] = {&waveformMod1, &waveformMod2, &waveformMod3, &waveformMod4}; 

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
int pin1; 
int old1; 
int holdState;
int ledState; //0 = ON, 1 = OFF 
float vib1; 
float vib2; 

//storage variables
int pitchMode; 
int mode;
String modeString[] = {"ADS Synth", "Vibrato Synth", "Harmonic Sweeper", "Octave Arpeggiator", "Octave Cascade", "FM Synth", "FM Arpeggiator"};  
String pitchString[] = {"P: OFF", "P: ON "}; 
String knob1String[] = {"1: Att.", "1: Depth", "1: Rate", "1: Rate", "1: Rate", "1: Mod.", "1: Rate"}; 
String knob2String[] = {"2: Dec.", "2: Rate", "2: Env.", "2: Env.", "2: Decay", "2: Env.", "2: Env."};
String waveString[] = {"SINE", "SAW", "REVERSE SAW", "SQUARE", "TRIANGLE", "TRIANGLE VARIABLE",
                      "PULSE"}; 
String holdString[] = {"", "HOLD"};                     

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

float freqBase[] = { //base frequencies for keys
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
};

// this 32 bit number holds the states of the 24 buttons, 1 bit per button
uint32_t buttons = 0xFFFFFFFF;

// this is the wave form of the oscillators 
int waveForm; 

// tuning value storage 
float pitchScale;

//knob 1
int knob1; 
//knob 2
int knob2;
//knob 3
float knob3;  

// holds frequency value used to play
float freqPlay[] = {0, 0, 0, 0}; //setting for 4 note poly synth
float freqLast[] = {0, 0, 0, 0}; //save last fequency

// voices (up to keys held down)
uint8_t key[] = {0, 0, 0, 0}; //setting for 4 note poly synth
uint8_t holdKey[] {0, 0, 0, 0}; //stores key when hold button is pressed

// octave multiplier
uint8_t octave = 0; 

// arpeggiator counter
int arp = 0; 

// harmonic sweep counter
int sweep = 0; 

//used for calculations in code
float two12th = 1.059463; //used for arp. calculations
float vibrato = 0.0; //used for vibrato effect (changes with knobs)

//envelope variables for default values
float ADSR_a;
float ADSR_d;
float ADSR_s;
float ADSR_r; 
float ADSR_hold; 
float envMult; 

int current_waveform = 0; //stores what current waveform we are on

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
  pin1 = 1; 
  holdState = 0; 
  mode = 0; 
  pitchMode = 0; 
  octaveShift = 2; 
  octaveDisplay = 0; 
  waveForm = 0; //starting waveform 
  ledState = 1; 
  

  //flash led to let us know it is turning on
  ledOnOff(); 
  delay(100);
  ledOnOff();
  delay(100);
  ledOnOff();
  delay(100);
  ledOnOff();

  //Turning on serial for debugging
  Serial.begin(57600);


  pinMode(1, INPUT_PULLUP);   
  //******************************************************
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(100);
  for (int z=0; z<4; z++) { //setup the waves and modulated waves
    waves[z]->frequency(0); 
    waves[z]->amplitude(1.0); 
    waves[z]->begin(current_waveform); 
    wavesFM[z]->frequency(0); 
    wavesFM[z]->amplitude(1.0); 
    wavesFM[z]->begin(current_waveform); 
    wavesFM[z]->frequencyModulation(5); 
  }

  //dac1.analogReference(INTERNAL); 
  
  //mixer1 is for mixing notes being played
  mixer1.gain(0, 0.25); 
  mixer1.gain(1, 0.20); 
  mixer1.gain(2, 0.20); 
  mixer1.gain(3, 0.20); 

  //mixers 2-5 are used to switch between regular wave and modulated wave
  mixer2.gain(0,1); 
  mixer2.gain(1,1); 
  mixer3.gain(0,1); 
  mixer3.gain(1,1); 
  mixer4.gain(0,1); 
  mixer4.gain(1,1); 
  mixer5.gain(0,1); 
  mixer5.gain(1,1); 

  //amp can adjust final output before dac if needed
  amp1.gain(1); 

  //I haven't messed with the filter yet but here to work with later
  filter1.frequency(0); 

  dac1.analogReference(INTERNAL); 

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

  //this runs the cleanup function that makes sure everything is setup for the ADS Synth
  modeCleanUp(); 
  

  //******************************************************
  //Screen Setup happens here

  //Uncomment the below if you want to check if the screen is working, not necessary if not using screen
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;) {
      // Don't proceed, loop forever if screen not found
      ledOnOff(); 
      delay(100);
      ledOnOff(); 
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
  //digitalWrite(PP_LED, 0);
  ledOnOff(); 
}

void loop() {
//  Serial.print("amp"); 
//  Serial.print(amp1.processorUsage()); 
//  Serial.print("all=");
//  Serial.print(AudioProcessorUsage());
//  Serial.print(",");
//  Serial.print(AudioProcessorUsageMax());
//  Serial.print("    ");
//  Serial.print("Memory: ");
//  Serial.print(AudioMemoryUsage());
//  Serial.print(",");
//  Serial.println(AudioMemoryUsageMax());
  
  //******************************************************
  //Read the knob values
  //knob 1 and 2 are mode dependent
  knob1 = analogRead(A0); 
  knob2 = analogRead(A1); 
  knob3 = analogRead(A2); 

  //pitch control can be turned on and off
  if (pitchMode == 1) { 
    //knob 3 is for the pitch
    pitchScale = 0.25 + 3.75*(knob3/1023.0);
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

  //if we are in hold mode we set the current keys to be the hold keys
  if (holdState == 1) { 
    key[0] = holdKey[0]; 
    key[1] = holdKey[1]; 
    key[2] = holdKey[2]; 
    key[3] = holdKey[3]; 
  }

  //******************************************************
  //store the last frequencies that were played (check to see if note should turn on/off/change)
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

  //shape sound based on what mode is selected
  if (mode == 0) { 
    //ADS Synth
    //knob1 = Attack
    //knob2 = Decay
    //knob3 = Sustain
    
    //control envelope with knobs
    envelope1.attack(knob1); 
    envelope2.attack(knob1);
    envelope3.attack(knob1);
    envelope4.attack(knob1);
    envelope1.decay(knob2);
    envelope2.decay(knob2);
    envelope3.decay(knob2);
    envelope4.decay(knob2);
    envelope1.sustain(knob3 / 1023.0); 
    envelope2.sustain(knob3 / 1023.0); 
    envelope3.sustain(knob3 / 1023.0); 
    envelope4.sustain(knob3 / 1023.0); 

    delay(10); 
    
  } else if (mode == 1) { 
    //Vibrato Synth
    //knob 1 = depth
    //knob 2 = rate   

    sine1.frequency(5 * (knob2 + 1) / 1024.0);  
    vibrato = 0.95 - rms1.read(); 
    float depth = .25*(knob1)/1023.0;   

    for (int z=0; z<4; z++) { 
      if (freqPlay[z] > 0) { 
        freqPlay[z] = freqPlay[z] + freqPlay[z] * depth * vibrato - freqPlay[n]*depth / 2;   
      } 
    }   
    
    delay(10);
    
  } else if (mode == 2) {  
    // Harmonic Sweeper
    //knob 1 = rate
    //knob 2 = envelope scaled

    //turn led on then off each loop
    ledOnOff();    

    // sweep counter
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
    // Octave Arpeggiator
    //knob 1 = rate
    //knob 2 = envelope scaled

    //turn led on then off each loop
    ledOnOff(); 
    
    //arp sweeper
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

        
  } else if (mode == 4) {  
    // Octave Cascade
    //knob 1 = rate
    //knob 2 = envelope decay
    
    //turn led on then off each loop
    ledOnOff(); 
    
    octave = (octave + 1) % 3;
    for (n = 0; n < 4; n++) { 
      freqPlay[n] = freqPlay[n] * (octave+1); 
      envs[n]->decay(255 * knob2 / 1023.0); //knob 2 affects decay
      }
    
    //knob 1 controls rate
    delay(int(knob1 >> 2) + 50);   // sweep speed

    
  } else if (mode == 5) { 
    // FM Synth
    //knob 1 = modulation
    //knob 2 = envelope
    sine1.amplitude(knob1 / 1023.0); 
    sine1.frequency(20.3 * knob2 / 1023.0);    
    delay(10); 
    
  } else if (mode == 6) { 
    // FM Arpeggiator
    //knob 1 = rate
    //knob 2 = envelope scaled
    
    //turn led on then off each loop
    ledOnOff(); 

    //arp sweeper
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
  }

  //uncomment below line to print freq of lowest key being played  
  //Serial.println(freqPlay[0]); 
  
  //Play the notes
  AudioNoInterrupts();
  if (mode == 5 || mode == 6) { //these modes use the modulated waveform
   for (n = 0; n < 4; n++) { 
      freqPlay[n] = freqPlay[n]*pitchScale;
      if (freqLast[n] != freqPlay[n]){ 
        envs[n] -> noteOff(); 
        wavesFM[n] -> frequency(freqPlay[n]);
        envs[n] ->noteOn();      
      }
    } 
  } else { //these modes don't
    for (n = 0; n < 4; n++) { 
      freqPlay[n] = freqPlay[n]*pitchScale;
      if (freqLast[n] != freqPlay[n]){ 
        envs[n] -> noteOff(); 
        waves[n] -> frequency(freqPlay[n]);
        envs[n] ->noteOn();      
      }
    }
  } 
  AudioInterrupts(); 
}
