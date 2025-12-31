volatile int rate[10];                   //array to hold last ten IBI values
volatile unsigned int sampleCounter = 0; //used to determine pulse timing
volatile unsigned int sampleTimingMs = 2; //rate for reading analog - microsec
volatile unsigned int lastBeatTime = 0;  //used to find IBI
volatile int P =540;                 // used to find peak in pulse wave, seeded
volatile int T = 540;				// used to find trough in pulse wave, seeded
volatile int threshSetting = 600;			
volatile int thresh = 600;           // find instant moment of heart beat, seeded
volatile int amp = 100;              // hold amplitude of pulse waveform, seeded
volatile bool firstBeat = true;   // seed rate array so we startup with reasonable BPM
volatile bool secondBeat = false; // seed rate array so we startup with reasonable BPM

long ultimoValor = 0;
double alpha = 0.85;

double coef1 = -0.0000000259340;
double coef2 = 0.0001049656215;
double coef3 = 0.9032840665333;
double coef4 = 204.6428253556780;

//hw_timer_t * timer = NULL;

void lepulso2mili();


//rotima de interrupt ou the polling/no loop
void lepulso2mili(){

//void ISRTr(){ // triggered when timer fires....
//	cli();
//int	gap_set = 300;

 double Signal = analogRead(34);
 //equação de compensação - polinomial de 3 ordem
 Signal = (coef1*pow(Signal, 3)) + (coef2*pow(Signal, 2)) + (coef3*Signal) + coef4;
 
 //Serial.print("Signal: ");
 //Serial.println(Signal);
  /*
  if (Signal >= gap_set) { 
	  Signal -= gap_set;
  }  else {
	  Signal = gap_set - Signal;
  }
  */
  // read the Pulse Sensor on pin 34 3.3v sensor power...default ADC setup...
  Signal = map(Signal, 0, 4095, 0, 1023);
 
 // uint16_t Signal3 = long(Signal);
 // Signal2 = int(map(Signal3, 0, 1023, 0, 255));

 //filtro de ruído
 double valorSignal = (alpha * ultimoValor) + ((1 - alpha) * Signal);
 ultimoValor = long(valorSignal);
 Signal2 = int(map(ultimoValor, 0, 1023, 0, 255));
  
  sampleCounter += sampleTimingMs;        // keep track of the time in mS
  int N = sampleCounter - lastBeatTime;   // time since the last beat to avoid noise

    //  find the peak and trough of the pulse wave
  if(Signal < thresh && N > (IBI/5)*3){   // avoid dichrotic noise by waiting 3/5 of last IBI
    if (Signal < T){                      // T is the trough
      T = Signal;                         // keep track of lowest point in pulse wave 
    }
  }

  if(Signal > thresh && Signal > P){   // thresh condition helps avoid noise
    P = Signal;                        // P is the peak
  }                                    // keep track of highest point in pulse wave

  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
  if (N > 250){                                   // avoid high frequency noise
    if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) ){        
      Pulse = true;                       // set the Pulse flag when we think there is a pulse
      IBI = sampleCounter - lastBeatTime; // measure time between beats in mS
      lastBeatTime = sampleCounter;       // keep track of time for next pulse

      if(secondBeat){             // if this is the second beat, if secondBeat == TRUE
        secondBeat = false;       // clear secondBeat flag
        for(int i=0; i<=9; i++){  // seed the running total to get a realisitic BPM at startup
          rate[i] = IBI;                      
        }
      }

      if(firstBeat){                         // if it's the first time we found a beat, if firstBeat == TRUE
        firstBeat = false;                   // clear firstBeat flag
        secondBeat = true;                   // set the second beat flag
       // sei();                               // enable interrupts again
        return;                              // IBI value is unreliable so discard it
      }   
											// keep a running total of the last 10 IBI values
      word runningTotal = 0;                // clear the runningTotal variable    

      for(int i=0; i<=8; i++){                // shift data in the rate array
        rate[i] = rate[i+1];                  // and drop the oldest IBI value 
        runningTotal += rate[i];              // add up the 9 oldest IBI values
      }

      rate[9] = IBI;			// add the latest IBI to the rate array
	  runningTotal += rate[9];   // add the latest IBI to runningTotal
      runningTotal /= 10;        // average the last 10 IBI values 
      BPM = 60000/runningTotal;  // how many beats can fit into a minute? that's BPM!
      QS = true;                 // set Quantified Self flag 
								 // QS FLAG IS NOT CLEARED INSIDE THIS ISR
    }                       
  }

  if (Signal < thresh && Pulse == true){// when the values are going down, the beat is over
    Pulse = false;                      // reset the Pulse flag so we can do it again
    amp = P - T;  						// get amplitude of the pulse wave
    thresh = amp/2 + T;                 // set thresh at 50% of the amplitude
    P = thresh;                         // reset these for next time
    T = thresh;
  }

  if (N > 2500){                        // if 2.5 seconds go by without a beat
	Pulse = false;
	IBI = 600;
	BPM = 0;
	QS = false;
    thresh = threshSetting;            // set thresh default
    P = 540;                           // set P default
    T = 540;                           // set T default
    lastBeatTime = sampleCounter;      // bring the lastBeatTime up to date        
    firstBeat = true;                  // set these to avoid noise
    secondBeat = false;				   // when we get the heartbeat back
	amp = 100;
  }

 // sei();
}// end isr

/*
void interruptSetup() {
	// Use 1st timer of 4 (counted from zero).
	// Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
	// info).
	timer = timerBegin(0, 80, true);

	// Initializes Timer to run the ISR to sample every 2mS as per original Sketch.
	// Attach ISRTr function to our timer.
	timerAttachInterrupt(timer, &ISRTr, true);

	// Set alarm to call isr function every 2 milliseconds (value in microseconds).
	// Repeat the alarm (third parameter)
	timerAlarmWrite(timer, 2000, true);
	//timerAlarmWrite(timer, 500000, true);

	// Start an alarm
	timerAlarmEnable(timer);
}
	*/