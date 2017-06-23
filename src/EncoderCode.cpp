/*



*/
#include "EncoderCode.h"

volatile int counter = 0;

boolean A_set;
boolean B_set;

#define encoder0PinA HALL_SENS
#define encoder0PinB HALL_DIR

void startEncode(void){

  pinMode(encoder0PinA,INPUT);
  pinMode(encoder0PinB,INPUT);

// encoder pin on interrupt 0 (pin 2)
  attachInterrupt(encoder0PinA, doEncoderA, CHANGE);
  
// encoder pin on interrupt 1 (pin 3)
  attachInterrupt(encoder0PinB, doEncoderB, CHANGE);

// initalize counter from EEPROM location
//  counter=EEPROMReadInt(memaddr);
}

 // Interrupt on A changing state
void doEncoderA(){
    digitalWrite(LED1_PIN,LOW);
  // Low to High transition?
  if (digitalRead(encoder0PinA) == HIGH) {
    A_set = true;
    if (!B_set) {
      counter++;
      if(counter>9999){
        counter=0;
        Serial.println("O");
      }
      Serial.println(counter);
    }
  }

  // High-to-low transition?
  if (digitalRead(encoder0PinA) == LOW) {
    A_set = false;
  }

}

 
// Interrupt on B changing state
void doEncoderB(){
  digitalWrite(LED1_PIN,HIGH);
  // Low-to-high transition?
  if (digitalRead(encoder0PinB) == HIGH) {
    B_set = true;
    if (!A_set) {
      counter--;
      if(counter<0){
        counter=9999;
        Serial.println("U");
      }
      Serial.println(counter);
    }
  }

  // High-to-low transition?
  if (digitalRead(encoder0PinB) == LOW) {
    B_set = false;
  }
}
