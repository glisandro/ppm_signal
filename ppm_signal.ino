//////////////////////CONFIGURATION///////////////////////////////
//#define DEBUG_SERIAL = true;
#define CHANNEL_NUMBER 5  //set the number of chanels
#define CHANNEL_DEFAULT_VALUE 1500  //set the default servo value
#define FRAME_LENGTH 22500  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PULSE_LENGTH 300  //set the pulse length
#define onState 0  //set polarity of the pulses: 1 is positive, 0 is negative
#define sigPin 10  //set PPM signal output pin on the arduino
#define giroPin1 2  //Giroscopio pin 1
#define giroPin2 3  //Giroscopio pin 2

/*this array holds the servo values for the ppm signal
 change theese values in your code (usually servo values move between 1000 and 2000)*/
int ppm[CHANNEL_NUMBER];

void setup(){  
  #ifdef DEBUG_SERIAL
    Serial.begin(9600);
  #endif
  
  //analogReference (EXTERNAL); //Referencia analógica PIN AREF

  //initiallize default ppm values
  for(int i=0; i<CHANNEL_NUMBER; i++){
      ppm[i]= CHANNEL_DEFAULT_VALUE;
  }

  pinMode(sigPin, OUTPUT);
  pinMode(giroPin1, INPUT_PULLUP);
  pinMode(giroPin2, INPUT_PULLUP);
  digitalWrite(sigPin, !onState);  //set the PPM signal pin to the default state (off)
  
  cli();
  TCCR1A = 0; // set entire TCCR1 register to 0
  TCCR1B = 0;
  
  OCR1A = 100;  // compare match register, change this
  TCCR1B |= (1 << WGM12);  // turn on CTC mode
  TCCR1B |= (1 << CS11);  // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();
  //Serial.begin(9600);
}
int motor;
int motorActive = 0;
int giroValue = 2012;
void loop(){
  
  /*
    Here modify ppm array and set any channel to value between 1000 and 2000. 
    Timer running in the background will take care of the rest and automatically 
    generate PPM signal on output pin using values in ppm array
  */
  motor = analogRead(A3);
  
  /*if(motorActive == 0 && motor > 1000) {
      motorActive = 1;
  } else if(motorActive == 1 && motor == 0) {
      motorActive = 2;
  } else if( motorActive < 2 || motor < 150) {
    motor = 0;
  }*/

  // Gyroscope
  if(digitalRead(giroPin1) == LOW) {
    giroValue = 0;
  } else if(digitalRead(giroPin2) == LOW) {
    giroValue = 2012;
  } else {
    giroValue = 1006;
  }

  ppm[0] = map(analogRead(A0), 0, 1023, 988, 2012); // Aleron
  ppm[1] = map(analogRead(A1), 0, 1023, 988, 2012); // Elevador
  ppm[2] = map(motor, 150, 1023, 988, 2012);          // Motor  
  ppm[3] = map(analogRead(A2), 0, 1023, 988, 2012); // Timón
  ppm[4] = giroValue;

  #ifdef DEBUG_SERIAL

    Serial.print("giroValue ");
    Serial.print(giroValue);

    Serial.print(" - ppm 0: ");
    Serial.print(ppm[0]);
    Serial.print(" Aleron: ");
    Serial.print(analogRead(A0));

    Serial.print(" - ppm 1: ");
    Serial.print(ppm[1]);
    Serial.print(" , Elevador: ");
    Serial.print(analogRead(A1));

    Serial.print(" - ppm 2: ");
    Serial.print(ppm[2]);
    Serial.print(" , Motor: ");
    Serial.print(analogRead(A2));

    Serial.print(" - ppm 3: ");
    Serial.print(ppm[3]);
    Serial.print(" , Timon: ");
    Serial.println(analogRead(A3));
  #endif
}

ISR(TIMER1_COMPA_vect){  //leave this alone
  static boolean state = true;
  
  TCNT1 = 0;
  
  if (state) {  //start pulse
    digitalWrite(sigPin, onState);
    OCR1A = PULSE_LENGTH * 2;
    state = false;
  } else{  //end pulse and calculate when to start the next pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;
  
    digitalWrite(sigPin, !onState);
    state = true;

    if(cur_chan_numb >= CHANNEL_NUMBER){
      cur_chan_numb = 0;
      calc_rest = calc_rest + PULSE_LENGTH;// 
      OCR1A = (FRAME_LENGTH - calc_rest) * 2;
      calc_rest = 0;
    }
    else{
      OCR1A = (ppm[cur_chan_numb] - PULSE_LENGTH) * 2;
      calc_rest = calc_rest + ppm[cur_chan_numb];
      cur_chan_numb++;
    }     
  }
}
