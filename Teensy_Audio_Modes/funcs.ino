//update screen info
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
  display.print("Wave = "); 
  display.println(waveString[waveForm]);  
  display.display();
}

// this funcion reads the buttons and stores their states in the global 'buttons' variable
void getButton(void) {
  buttons = 0;
  for (j = 0; j < 8; j++) {
    digitalWrite(MUX_SEL_A, j & 1);
    digitalWrite(MUX_SEL_B, (j >> 1) & 1);
    digitalWrite(MUX_SEL_C, (j >> 2) & 1);
    delayMicroseconds(50); 
    buttons |= digitalRead(MUX_OUT_2) << j;
  }
  buttons <<= 8;
  for (j = 0; j < 8; j++) {
    digitalWrite(MUX_SEL_A, j & 1);
    digitalWrite(MUX_SEL_B, (j >> 1) & 1);
    digitalWrite(MUX_SEL_C, (j >> 2) & 1);
    delayMicroseconds(50); 
    buttons |= digitalRead(MUX_OUT_1) << j;
  }
  buttons <<= 8;
  for (j = 0; j < 8; j++) {
    digitalWrite(MUX_SEL_A, j & 1);
    digitalWrite(MUX_SEL_B, (j >> 1) & 1);
    digitalWrite(MUX_SEL_C, (j >> 2) & 1);
    delayMicroseconds(50); 
    buttons |= digitalRead(MUX_OUT_0) << j;
  }
  buttons |= 0x1000000;  // for the 25th button
  //buttons &= ~0x1000000;
 
  old25 = pin25; 
  pin25 = digitalRead(KEY_25);
}

//determine if function key (key 25) has been pressed
//if pressed complete function based on key combination 
void key25Func(void) { 
  //Find out if button 25 has been pressed
  if (pin25==1 && old25 == 0) {
    //Flash LED 
    digitalWrite(PP_LED, 1);
    delay(100);
    digitalWrite(PP_LED, 0);

    //determine if a secondary function key has been pressed for pitch or waveform

    if (secondFunc == 4){ //turn pitch tuning off
      pitchMode = (pitchMode+1) %2;  
    } else if (secondFunc == 2) { //change waveform
      waveForm = (waveForm+1) %7; 
      waveformSwitch(); 
    } else if (secondFunc == 1) { 
        octaveShift = octaveShift - 1; 
        if (octaveShift < 0) { 
          octaveShift = 0 ;
        }
        octShiftFunc();
    } else if (secondFunc == 3) {
        octaveShift = octaveShift + 1; 
        if (octaveShift > 4) { 
          octaveShift = 4 ;
        }
        octShiftFunc();       
    } else { //change mode if only button 25 is pressed
      //increase counter 
      mode = (mode+1) %5; //counter goes 0-4
      modeCleanUp(); //run this to cleanup any mode specific settings
    }
    screenUpdate();
  }
}

//shift octave variable 
void octShiftFunc(void) { 
  if (octaveShift == 0) { 
    octaveDisplay = -2; 
  } else if (octaveShift == 1) { 
    octaveDisplay = -1; 
  } else if (octaveShift == 2) { 
    octaveDisplay = 0; 
  } else if (octaveShift == 3) { 
    octaveDisplay = 1; 
  } else { 
    octaveDisplay = 2; 
  } 
} 

void waveformSwitch(void) { 
  switch (current_waveform) {
      case WAVEFORM_SINE:
        current_waveform = WAVEFORM_SAWTOOTH;
        Serial.println("Sawtooth");
        break;
      case WAVEFORM_SAWTOOTH:
        current_waveform = WAVEFORM_SAWTOOTH_REVERSE;
        Serial.println("Reverse Sawtooth");
        break;
      case WAVEFORM_SAWTOOTH_REVERSE:
        current_waveform = WAVEFORM_SQUARE;
        Serial.println("Square");
        break;
      case WAVEFORM_SQUARE:
        current_waveform = WAVEFORM_TRIANGLE;
        Serial.println("Triangle");
        break;
      case WAVEFORM_TRIANGLE:
        current_waveform = WAVEFORM_TRIANGLE_VARIABLE;
        Serial.println("Variable Triangle");
        break;
      case WAVEFORM_TRIANGLE_VARIABLE:
        current_waveform = WAVEFORM_PULSE;
        Serial.println("Arbitary Waveform");
        break;
      case WAVEFORM_PULSE:
        current_waveform = WAVEFORM_SINE;
        Serial.println("Sine");
        break;
  } 
  AudioNoInterrupts();
  waveform1.begin(current_waveform);
  waveform2.begin(current_waveform);
  waveform3.begin(current_waveform);
  waveform4.begin(current_waveform);
  AudioInterrupts();
}

void modeCleanUp(void) { 
    if (mode == 0) { 
    //Vibrato Synth Starting
    //knob 1 = depth
    //knob 2 = rate
    
    //we set the bit crusher to pass through
    bitcrusher1.bits(bitsPassthrough); 
    bitcrusher1.sampleRate(bitsRatePassthrough); 


  } else if (mode == 1) { 
    // Harmonic Sweeper
    //knob 1 = rate
    //knob 2 = envelope scaled
    AudioNoInterrupts();
    //reset amplitude to 1
    waveform1.amplitude(1);
    waveform2.amplitude(1);
    waveform3.amplitude(1);
    waveform4.amplitude(1);
    envelope1.sustain(0.1); 
    envelope2.sustain(0.1);
    envelope3.sustain(0.1);
    envelope4.sustain(0.1); 
    AudioInterrupts();
    
  } else if (mode == 2) { 
    // Octave Arpeggiator
    //knob 1 = rate
    //knob 2 = envelope scaled

//    AudioNoInterrupts();
//    envelope1.sustain(0.1); 
//    envelope2.sustain(0.1);
//    envelope3.sustain(0.1);
//    envelope4.sustain(0.1);  
//    AudioInterrupts();
    
  } else if (mode == 3) { 
    // Octave Cascade
    //knob 1 = rate
    //knob 2 = envelope decay
    AudioNoInterrupts();
    envelope1.attack(ADSR_a); 
    envelope2.attack(ADSR_a); 
    envelope3.attack(ADSR_a);
    envelope4.attack(ADSR_a);      
    envelope1.hold(ADSR_hold); 
    envelope2.hold(ADSR_hold); 
    envelope3.hold(ADSR_hold); 
    envelope4.hold(ADSR_hold); 
    //use a low sustain so we can hear the change in decay 
    //controlled by knob 2
    envelope1.sustain(0.1); 
    envelope2.sustain(0.1);
    envelope3.sustain(0.1);
    envelope4.sustain(0.1);  
    AudioInterrupts();
    
  } else if (mode == 4) { 
    //bit crusher
    //knob 1 = bits
    //knob 2 = sampling rate
    AudioNoInterrupts();
    envelope1.decay(ADSR_d);
    envelope2.decay(ADSR_d);
    envelope3.decay(ADSR_d);
    envelope4.decay(ADSR_d);
    envelope1.sustain(ADSR_s); 
    envelope2.sustain(ADSR_s);
    envelope3.sustain(ADSR_s);
    envelope4.sustain(ADSR_s); 
    AudioInterrupts();
  }
}
