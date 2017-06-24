/*



*/
#include "EncoderCode.h"

//volatile int counter = 0;

boolean A_set;
boolean B_set;

#define encoder0PinA HALL_SENS
#define encoder0PinB HALL_DIR

void startEncoder(void){

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
void doEncoderA(void){
    digitalWrite(LED1_PIN,LOW);
  // Low to High transition?
  if (digitalRead(encoder0PinA) == HIGH) {
    A_set = true;
    if (!B_set) {
      mainCnt--;
      if(mainCnt<0){
        mainCnt=9999;
//        Serial.println("O");
      }
      Serial.print(mainCnt);
      Serial.print("\n");
    }
  }

  // High-to-low transition?
  if (digitalRead(encoder0PinA) == LOW) {
    A_set = false;
  }

}


// Interrupt on B changing state
void doEncoderB(void){
  digitalWrite(LED1_PIN,HIGH);
  // Low-to-high transition?
  if (digitalRead(encoder0PinB) == HIGH) {
    B_set = true;
    if (!A_set) {
      mainCnt++;
      if(mainCnt>9999){
        mainCnt=0;
  //      Serial.println("U");
      }
      Serial.print(mainCnt);
      Serial.print("\n");
    }
  }

  // High-to-low transition?
  if (digitalRead(encoder0PinB) == LOW) {
    B_set = false;
  }
}
