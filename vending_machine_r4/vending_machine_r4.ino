#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>

const int stepsPerRev = 2048; 

Stepper motor1(stepsPerRev, 2, 4, 3, 5);
Stepper motor2(stepsPerRev, 6, 8, 7, 9);

LiquidCrystal_I2C lcd(0x27, 16, 2); 

const int btn1_Pin = 10;   
const int btn2_Pin = 11;  
const int btn3_Pin = 12;  
const int coinSensor_Pin = A0; 

int selectedItem = 0;   
bool hasCoin = false;   

void setup() {
  Serial.begin(9600);
  motor1.setSpeed(12);
  motor2.setSpeed(12);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("   Boooy TU   ");
  pinMode(btn1_Pin, INPUT_PULLUP);
  pinMode(btn2_Pin, INPUT_PULLUP);
  pinMode(btn3_Pin, INPUT_PULLUP);
  pinMode(coinSensor_Pin, INPUT_PULLUP);

  delay(3000);
  lcd.clear();
}

void loop() {
  if (digitalRead(btn1_Pin) == LOW) {
    selectedItem = 1;
    delay(200); 
  }
  if (digitalRead(btn2_Pin) == LOW) {
    selectedItem = 2;
    delay(200);
  }
  if (digitalRead(coinSensor_Pin) == LOW) {
    hasCoin = true;
    delay(100); 
  }

  lcd.setCursor(0, 0);
  lcd.print("Select: ");
  if (selectedItem == 0) lcd.print("-   ");
  else {
    lcd.print(selectedItem);
    lcd.print("   ");
  }

  lcd.setCursor(0, 1);
  lcd.print("Coin: ");
  if (hasCoin) lcd.print("Yes ");
  else lcd.print("No  ");

  if (digitalRead(btn3_Pin) == LOW) {
    delay(200);

    if (selectedItem != 0 && hasCoin == true) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Processing...");
      lcd.setCursor(0, 1);
      lcd.print("Please Wait");

      if (selectedItem == 1) {
        motor1.step(stepsPerRev);
        disableMotorPins(1);
      } else if (selectedItem == 2) {
        motor2.step(stepsPerRev);
        disableMotorPins(2);
      }

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("   Thank You!   ");
      delay(2000); 

      selectedItem = 0;
      hasCoin = false;
      lcd.clear();

    } else if (selectedItem == 0) {
      lcd.clear();
      lcd.print("Please Select");
      delay(1500); lcd.clear();
    } else if (hasCoin == false) {
      lcd.clear();
      lcd.print("Insert Coin!");
      delay(1500); lcd.clear();
    }
  }
}

void disableMotorPins(int motorNum) {
  if (motorNum == 1) {
    digitalWrite(2, LOW); digitalWrite(3, LOW);
    digitalWrite(4, LOW); digitalWrite(5, LOW);
  } else {
    digitalWrite(6, LOW); digitalWrite(7, LOW);
    digitalWrite(8, LOW); digitalWrite(9, LOW);
  }
}