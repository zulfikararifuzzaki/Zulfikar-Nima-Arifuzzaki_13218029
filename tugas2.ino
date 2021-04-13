int STATE = 0;
int LAST_STATE = 0;
int INCR = 0;
int x1 = 0;
int x2 = 0;
int x3 = 0;
int x4 = 0;

// VARIABEL WAKTU
int s_clock = 0;
int m_clock = 0;
int h_clock = 0;
bool en_hour = false;
bool en_clock = false;

// VARIABEL ALARM
bool en_alarm_set = false;
bool en_alarm = false;
bool en_buzzer = false;
int m_alarm = 0;
int h_alarm = 0;

// VARIABEL STOPWATCH
int stopwatch = 0;
bool en_stopwatch = false;

// VARIABEL DISPLAY
int front = 0;
int rear = 0;

void setup() {
  Serial.begin(9600);
  DDRD = 0b00000000;
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  DDRD |= 0b11110011 ;
  DDRB = 0b111111;
  DDRC = 0b111111;

  //PENGATURAN TIMER
  noInterrupts(); // stop interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  //set prescaler 1024
  TIMSK1=0b00000001;
  TCCR1B=0b00000101;
  TCNT1   = 49911;
  //Enable Interrupt
  interrupts();//allow interrupts

  // PENGATURAN INTERRUPT STATE BUTTON
  attachInterrupt(digitalPinToInterrupt(2), state, FALLING);

  // PENGATURAN INTERRUPT INCREMENT BUTTON
  attachInterrupt(digitalPinToInterrupt(3), incr, FALLING);

}

// TIMER
ISR(TIMER1_OVF_vect){
  // STOPWATCH INCREMENT
  if(en_stopwatch == true){
    stopwatch += 1;
    }

  // CLOCK INCREMENT
  if (en_clock == true){
    s_clock += 1;
    if (s_clock >= 60){
      s_clock = 0;
      m_clock += 1;
      }
    if (m_clock >= 60){
      m_clock = 0;
      h_clock += 1;
      }
    if (h_clock >= 24){
      s_clock = 0;
      m_clock = 0;
      h_clock = 0;
      }
    }

  // SETTING BUZZER FOR ALARM
  if ((s_clock == 0) & (m_clock == m_alarm) & (h_clock == h_alarm) & (en_alarm == true)){
    LAST_STATE = STATE;
    STATE = 99;
    }
  TCNT1   = 49911;
}

void state(){
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // Abaikan interrupt jika  muncul kembali setelah kurang dari 200ms
  if (interrupt_time - last_interrupt_time > 300){
    if ((STATE >= 8) & (STATE != 99)) { 
      STATE = 0; }
    else if (STATE == 3){ 
      if (en_stopwatch == false){
        STATE = 4;}
      else {
        STATE = 5;}
        }
    else if (STATE == 5){
      if (en_alarm_set == false){ 
        STATE = 8;
        en_alarm = false;
        }
      else { STATE = 6; } 
      }
   else if (STATE == 7){
      STATE = 8;
      en_alarm = true;
    }
    else if (STATE == 99){
      INCR = 1;
      }
    else{ STATE += 1; }
    }
  last_interrupt_time = interrupt_time;
  }

void incr(){
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // Abaikan interrupt jika  muncul kembali setelah kurang dari 200ms
  if (interrupt_time - last_interrupt_time > 300){
    if (STATE == 3){
      en_stopwatch = !en_stopwatch;
      }
    else if (STATE == 4){
      STATE = 3;
      }
    else if (STATE == 5){
      en_alarm_set = !en_alarm_set;
      }
    else if (STATE == 8){
      en_hour = !en_hour;
      }
    else{
      INCR = 1;
      }
  last_interrupt_time = interrupt_time;}
  }

// STATE 0 - 2 : SETTING CLOCK TIME
void state0(){
  en_clock = false;
  s_clock += INCR;
  INCR = 0;
  if(s_clock >= 60){
    s_clock = 0;
    m_clock += 1;
    }
  rear = s_clock;
  front = m_clock;
  ssegment(front*100 + rear);
  }
void state1(){
  en_clock = false;
  m_clock += INCR;
  INCR = 0;
  if(m_clock >= 60){
    m_clock = 0;
    h_clock += 1;
    }
  rear = s_clock;
  front = m_clock;
  ssegment(front*100 + rear);
  }
void state2(){
  en_clock = false;
  h_clock += INCR;
  INCR = 0;
  if (h_clock >= 24){
    h_clock = 0;
    }
  rear = h_clock;
  front = 0;
  ssegment(front*100 + rear);
  }

// STATE 3 - 4 : CONTROLLING STOPWATCH
void state3(){
  en_clock = true;
  ssegment(stopwatch);
  }
void state4(){
  stopwatch = 0;
  en_stopwatch = false;
  ssegment(0);
  }

// STATE 5 : ALARM SWITCHING
void state5(){
  ssegment(h_alarm*100 + m_alarm);
  }

// STATE 6 - 7 : CONTROLLING ALARM TIME
void state6(){
  m_alarm += INCR;
  INCR = 0;
  if(m_alarm >= 60){ 
    m_alarm = 0;
    h_alarm += 1; }
  ssegment(h_alarm*100 + m_alarm);
  }
  
void state7(){
  h_alarm += INCR;
  INCR = 0;
  if(h_alarm >= 60){
    h_alarm = 0; }
  ssegment(h_alarm*100 + m_alarm);
  }

// STATE 8 : SHOWING TIME
void state8(){
  if (en_hour == true){
    front = h_clock;
    rear = m_clock;
    }
  else{
    front = m_clock;
    rear = s_clock;
    }
  ssegment(front*100 + rear);
  }

// STATE 99 : TURNING ON THE BUZZER
void state99(){
  en_alarm = false;
  en_buzzer = true;
  front = h_alarm;
  rear = m_alarm;
  while (en_buzzer == true){
    PORTC |= (1<<PC2);
    ssegment(front*100 + rear);
    if (INCR == 1){en_buzzer = false;}
    }
  INCR = 0;
  en_alarm = true;
  STATE = LAST_STATE;
  LAST_STATE = 0;
  }
  
void ssegment(int number){
  x1 = number/1000;
  x2 = (number%1000)/100;
  x3 = (number%100)/10;
  x4 = (number%10);

  PORTC = (1<<PC1)|(0<<PC0);
  PORTD = (1<<PB5)|(1<<PB4);
  PORTD |= (convert(x4)<<6);
  PORTB = convert(x4)>>2;
  delay(1);

  PORTC = (0<<PC1)|(1<<PC0);
  PORTD = (1<<PB5)|(1<<PB4);
  PORTD |= (convert(x3)<<6);
  PORTB = convert(x3)>>2;
  delay(1);

  PORTC = (1<<PC1)|(1<<PC0);
  PORTD = (1<<PB5)|(0<<PB4);
  PORTD |= (convert(x2)<<6);
  PORTB = convert(x2)>>2;
  delay(1);

  PORTC = (1<<PC1)|(1<<PC0);
  PORTD = (0<<PB5)|(1<<PB4);
  PORTD |= (convert(x1)<<6);
  PORTB = convert(x1)>>2;
  delay(1);
  }

void loop() {
  while(STATE == 0){
    state0();
    Serial.println(STATE);
    }
  while(STATE == 1){
    state1();
    Serial.println(STATE);
  }
  while(STATE == 2){
    state2();
    Serial.println(STATE);
    }
  while(STATE == 3){
    state3();
    Serial.println(STATE);
    }
  while (STATE == 4){
    state4();
    Serial.println(STATE);
    }
  while (STATE == 5){
    state5();
    Serial.println(STATE);
    }
  while (STATE == 6){
    state6();
    Serial.println(STATE);
    }
  while (STATE == 7){
    state7();
    Serial.println(STATE);
    }
  while(STATE == 8){
    state8();
    Serial.println(STATE);
    }
  while (STATE == 99){
    state99();
    Serial.println(STATE);
    }
}

unsigned int convert(int number){
  switch (number){
      case 0 :
        return 0b00111111; 
      case 1 :
        return 0b00000110;
      case 2 :
        return 0b01011011;
      case 3 :
        return 0b01001111;
      case 4 :
        return 0b01100110;
      case 5 :
        return 0b01101101;
      case 6 :
        return 0b01111101;
      case 7 :
        return 0b00000111;
      case 8 :
        return 0b01111111;
      case 9 :
        return 0b01101111;
      }
}
