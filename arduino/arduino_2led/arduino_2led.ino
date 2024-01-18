
String recived_str;

int pwm_prescalar = 8; // 16000000 / 8 / 256 = 7812.5 Hz
// 0, 8, 32, 64, 128, 256, 1024

int duty;
float pulse_fps;
float pulse_period_ms;
bool is_cam_1_active = true;
bool only_one_wl = false;
bool turn_off_blink = false;

float pulse_lengt_ms = 1; 

int pulse_pin = 4;
int pulse_pin2 = 5;

int pulse_cam1 = 6;
int pulse_cam2 = 7;


void setup() {
  Serial.begin(9600); 

  DDRD = _BV(3); // pin 3 - pwm
  DDRD |= (_BV(pulse_pin) | _BV(pulse_pin2)); // pin pulse_pin
  DDRD |= (_BV(pulse_cam1) | _BV(pulse_cam2)); // pin pulse_pin
  
  PORTD &= ~ _BV(3);
  PORTD &= ~ (_BV(pulse_pin) | _BV(pulse_pin2));
  PORTD &= ~ (_BV(pulse_cam1) | _BV(pulse_cam2));

  delay(100);
  
  
}

void loop() {

  while(Serial.available() > 0 ){
    
    recived_str = Serial.readString();

    if (recived_str.substring(0,3) == "ok?"){
      Serial.println("ok");
    }

    else if (recived_str.substring(0,4) == "toff"){
      only_one_wl = false;
      
      TCCR1A = 0; TCCR1B = 0;
      PORTD &= ~ (_BV(pulse_pin) | _BV(pulse_pin2));
      PORTD &= ~ (_BV(pulse_cam1) | _BV(pulse_cam2));
      
      Serial.println("toff");
      continue;
    }
    else if (recived_str.substring(0,1) == "t"){
      only_one_wl = false;
      
      pulse_fps = recived_str.substring(1).toFloat();

      Serial.println("t" + String(pulse_fps));
      pulse_period_ms = 1000.0/pulse_fps;
      pulse_lengt_ms = 0.5 * pulse_period_ms;
      

      PORTD &= ~ (_BV(pulse_pin) | _BV(pulse_pin2));
      PORTD &= ~ (_BV(pulse_cam1) | _BV(pulse_cam2));
      
      TCCR1A = 0; TCCR1B = 0; TIMSK1 = 0;
      

      // OCR1B = int(62.5 * pulse_period_ms);   // 62.5* => tick is 1ms with 256 prescalar - max 1048ms  
      OCR1B = int(250 * (pulse_period_ms - pulse_lengt_ms));   // 250* tick is 1ms with 64 prescalar  - max 262ms  
      
      //OCR1A = int(62.5 * (pulse_period_ms + pulse_lengt_ms));   // 62.5* => tick is 1ms with 256 prescalar - max 1048ms 
      OCR1A = int(250 * (pulse_period_ms));   // 250* tick is 1ms with 64 prescalar  - max 262ms //A nuluje takze musí být vetsi!!!!

      PORTD &= ~ (_BV(pulse_pin) | _BV(pulse_pin2));
      PORTD &= ~ (_BV(pulse_cam1) | _BV(pulse_cam2));
      TIMSK1 |= _BV(OCIE1A); // enable interupt  Output Compare A
      TIMSK1 |= _BV(OCIE1B); // enable interupt  Output Compare B

      PORTD &= ~ (_BV(pulse_pin) | _BV(pulse_pin2));
      PORTD &= ~ (_BV(pulse_cam1) | _BV(pulse_cam2));
      // TCCR1B |= _BV(CS12);  // prescaler 256 - timer1   
      TCCR1B |= _BV(CS11) | _BV(CS10); // prescaler 64 - timer1
      TCCR1B |= _BV(WGM12); // CTC setting timer 1

      PORTD &= ~ (_BV(pulse_pin) | _BV(pulse_pin2));
      PORTD &= ~ (_BV(pulse_cam1) | _BV(pulse_cam2));
      
      cli();
      sei();

      
      
      continue;
    }
    else if(recived_str.substring(0,4) == "loff"){
      
      TCCR2A = 0; 
      PORTD &= ~ _BV(3);
      Serial.println("loff");
      continue;
    }
    else if (recived_str.substring(0,1) == "l"){

      duty = recived_str.substring(1).toInt();
      Serial.println("l" + String(duty));

      TCCR2A = 0; 
      TCCR2A |= _BV(WGM21) | _BV(WGM20); // fast PWM mode
      TCCR2A |= _BV(COM2B1); // non-inverted PWM for outputs B (pin 3)
      
      TCCR2B = 0;
      switch (pwm_prescalar){
          case 0:
            TCCR2B |= _BV(CS20);
          case 8:
            TCCR2B |= _BV(CS21);
          case 32:
            TCCR2B |= _BV(CS20) | _BV(CS21);
          case 64:
            TCCR2B |= _BV(CS22); 
          case 128:
            TCCR2B |= _BV(CS20) | _BV(CS22);
          case 256:
            TCCR2B |= _BV(CS21) | _BV(CS22);
          case 1024:
            TCCR2B |= _BV(CS20) | _BV(CS21) | _BV(CS22);
      }
    
      OCR2B = int((float(duty)/100.0)*256.0 - 1.0);
      
      cli();
      sei();
  
      
      
      
      continue;
    }
    else if(recived_str.substring(0,3) == "son"){
      only_one_wl = true;
    }
    else if(recived_str.substring(0,4) == "soff"){
      only_one_wl = false;
    }
    else if(recived_str.substring(0,4) == "boff"){
      turn_off_blink = true;
      //Serial.println("boff ok");
    }
    else if(recived_str.substring(0,3) == "bon"){
      turn_off_blink = false;
      //Serial.println("bon ok");
    }
    
    else{
      Serial.println("unk: " + recived_str );
    }
    
    }

}


ISR(TIMER1_COMPA_vect){
  PORTD &= ~(_BV(pulse_pin) | _BV(pulse_pin2));
}

ISR(TIMER1_COMPB_vect){

  if (only_one_wl) 
  {
    PORTD &= ~ _BV(pulse_cam2);
    PORTD |= (_BV(pulse_cam1));
  }
  else
  {
    if (!turn_off_blink){
      if (is_cam_1_active){
        PORTD &= ~ _BV(pulse_cam2);
        PORTD |= (_BV(pulse_cam1)); 
        is_cam_1_active = false;
      }
      else{
        PORTD &= ~ _BV(pulse_cam1);
        PORTD |= (_BV(pulse_cam2)); 
        is_cam_1_active = true;
      }
      //Serial.println("on on");
    }
    else{
      PORTD &= ~ _BV(pulse_cam1);
      PORTD &= ~ _BV(pulse_cam2);
      //Serial.println("off off");
    }
  }
  PORTD |= (_BV(pulse_pin) | _BV(pulse_pin2)); 
  
}
  
