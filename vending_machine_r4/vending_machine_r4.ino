#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// --- ตั้งค่าจอ LCD ---
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// --- ขา Stepper Motor ---
const int stepPin1 = 2; // แกน X
const int dirPin1 = 5;
const int stepPin2 = 3; // แกน Y
const int dirPin2 = 6;
const int enPin = 8;

// --- ขา Input ---
const int btn1_Pin = 9;   
const int btn2_Pin = 10;  
const int btn3_Pin = 11;  
const int coinSensor_Pin = A0; 

// --- ตัวแปรสำหรับเก็บสถานะระบบ ---
int selectedItem = 0;   // 0=ไม่เลือก, 1=เลือกของ1, 2=เลือกของ2
bool hasCoin = false;   // false=ไม่มีเหรียญ, true=มีเหรียญ
const int stepsPerRev = 200; // จำนวน Step ต่อ 1 รอบ (ถ้าใส่ Jumper ต้องแก้เป็น 3200)

void setup() {
  Serial.begin(9600);
  
  // Init LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Vending Ready");

  // Init Pins
  pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW); // เปิดการทำงานมอเตอร์

  pinMode(btn1_Pin, INPUT_PULLUP);
  pinMode(btn2_Pin, INPUT_PULLUP);
  pinMode(btn3_Pin, INPUT_PULLUP);
  pinMode(coinSensor_Pin, INPUT_PULLUP);

  delay(5000);
  lcd.clear();
}

void loop() {
  // ------------------------------------------------
  // 1. รับค่าจากปุ่มเลือกสินค้า (Button 1 & 2)
  // ------------------------------------------------
  if (digitalRead(btn1_Pin) == LOW) {
    selectedItem = 1; // จำว่าเลือก 1
    delay(200);       // กันปุ่มเบิ้ล
  }
  
  if (digitalRead(btn2_Pin) == LOW) {
    selectedItem = 2; // จำว่าเลือก 2
    delay(200);
  }

  // ------------------------------------------------
  // 2. รับค่าจากเซนเซอร์เหรียญ (Coin Sensor)
  // ------------------------------------------------
  // ถ้าเจอเหรียญ (LOW) ให้เปลี่ยนสถานะเป็น true ทันที
  // และจะค้างสถานะนี้ไว้จนกว่าจะซื้อเสร็จ
  if (digitalRead(coinSensor_Pin) == LOW) {
    hasCoin = true;
    delay(100); // กันอ่านค่ารัวเกินไป
  }

  // ------------------------------------------------
  // 3. แสดงผลหน้าจอ LCD (Real-time Display)
  // ------------------------------------------------
  // บรรทัดที่ 0: แสดงสินค้าที่เลือก
  lcd.setCursor(0, 0);
  lcd.print("Select: ");
  if (selectedItem == 0) {
    lcd.print("-   "); // ยังไม่เลือก
  } else {
    lcd.print(selectedItem); // โชว์เลข 1 หรือ 2
    lcd.print("   ");    // ลบตัวอักษรเก่าที่อาจค้าง
  }

  // บรรทัดที่ 1: แสดงสถานะเหรียญ
  lcd.setCursor(0, 1);
  lcd.print("Coin: ");
  if (hasCoin) {
    lcd.print("Yes "); // มีเหรียญแล้ว
  } else {
    lcd.print("No  "); // ยังไม่มี
  }

  // ------------------------------------------------
  // 4. ปุ่มยืนยัน (Button 3) - หัวใจหลัก
  // ------------------------------------------------
  if (digitalRead(btn3_Pin) == LOW) {
    delay(200); // Debounce

    // กรณี A: เงื่อนไขครบ (เลือกแล้ว + มีเหรียญ) -> จ่ายของ
    if (selectedItem != 0 && hasCoin == true) {
      
      // ขึ้นสถานะกำลังทำงาน
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Processing...");
      
      // สั่งมอเตอร์หมุน
      if (selectedItem == 1) {
        rotateMotor(stepPin1, dirPin1);
      } else if (selectedItem == 2) {
        rotateMotor(stepPin2, dirPin2);
      }

      // ทำงานเสร็จ ขึ้นขอบคุณ
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("   Thank You!   ");
      delay(2000); // โชว์ค้างไว้ 2 วิ

      // รีเซ็ตค่าระบบ เพื่อเริ่มรอบใหม่
      selectedItem = 0;
      hasCoin = false;
      lcd.clear();

    } 
    // กรณี B: ยังไม่ได้เลือกของ
    else if (selectedItem == 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Please Select");
      lcd.setCursor(0, 1);
      lcd.print("Item 1 or 2");
      delay(1500); // เตือน 1.5 วิ แล้วกลับไปหน้าหลัก
      lcd.clear();
    }
    // กรณี C: เลือกของแล้ว แต่ยังไม่มีเหรียญ
    else if (hasCoin == false) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Please Insert");
      lcd.setCursor(0, 1);
      lcd.print("Coin first!");
      delay(1500); // เตือน 1.5 วิ แล้วกลับไปหน้าหลัก
      lcd.clear();
    }
  }
}

// ------------------------------------------------
// ฟังก์ชันแยก สำหรับหมุนมอเตอร์ 1 รอบ
// ------------------------------------------------
void rotateMotor(int stepPin, int dirPin) {
  // กำหนดทิศทาง (HIGH/LOW แล้วแต่การติดตั้งว่าหมุนทางไหนคือจ่ายของ)
  digitalWrite(dirPin, HIGH); 

  // วนลูปตามจำนวน Step (200 step = 1 รอบ สำหรับ 1.8 องศา)
  // ถ้าคุณใส่ Jumper microstep ต้องแก้เลข 200 เป็นค่าอื่น เช่น 3200
  for(int x = 0; x < stepsPerRev; x++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000); // ปรับความเร็วตรงนี้ (ค่าน้อย=เร็ว)
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1000);
  }
}