#include <WiFi.h>
#include <PubSubClient.h>

// --- ส่วนสำคัญ: เพิ่มไลบรารีแก้ปัญหาไฟตก (Brownout) ---
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// --- 1. ตั้งค่า Wi-Fi ---
const char* ssid = "1T1M";      // ใส่ชื่อ Wi-Fi
const char* password = "12345678";  // ใส่รหัส Wi-Fi

// --- 2. MQTT Broker (EMQX) ---
const char* mqtt_server = "broker.emqx.io"; 
const int mqtt_port = 1883;

// --- 3. Topic ---
const char* topic_slot1 = "1T1M_Project_Dev/slot1"; 
const char* topic_slot2 = "1T1M_Project_Dev/slot2";
const char* topic_status = "1T1M_Project_Dev/status";

// --- 4. Hardware Pin (เปลี่ยนขาหนีขาเสีย!) ---
// ห้ามใช้ 26, 27 แล้วนะครับ มันอาจจะพังไปแล้ว
const int slot1Pin = 18; 
const int slot2Pin = 19;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long slot1Timer = 0;
unsigned long slot2Timer = 0;
bool isSlot1Running = false;
bool isSlot2Running = false;
const long runTime = 3000; 

// ==========================================
// Function สั่งงาน
// ==========================================
void funcSlot1_Start() {
  if (!isSlot1Running) {
    digitalWrite(slot1Pin, HIGH); 
    isSlot1Running = true;
    slot1Timer = millis();
    Serial.println("Slot 1: STARTED");
    client.publish(topic_status, "Slot 1 Processing...");
  }
}

void funcSlot1_Stop() {
  digitalWrite(slot1Pin, LOW); 
  isSlot1Running = false;
  Serial.println("Slot 1: STOPPED");
  client.publish(topic_status, "Ready");
}

void funcSlot2_Start() {
  if (!isSlot2Running) {
    digitalWrite(slot2Pin, HIGH);
    isSlot2Running = true;
    slot2Timer = millis();
    Serial.println("Slot 2: STARTED");
    client.publish(topic_status, "Slot 2 Processing...");
  }
}

void funcSlot2_Stop() {
  digitalWrite(slot2Pin, LOW);
  isSlot2Running = false;
  Serial.println("Slot 2: STOPPED");
  client.publish(topic_status, "Ready");
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  if (String(topic) == topic_slot1 && (message == "1" || message == "ON")) {
    funcSlot1_Start();
  }
  else if (String(topic) == topic_slot2 && (message == "1" || message == "ON")) {
    funcSlot2_Start();
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting MQTT...");
    String clientId = "ESP32Survival-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(topic_slot1);
      client.subscribe(topic_slot2);
      client.publish(topic_status, "System Recovered!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(3000);
    }
  }
}

void setup() {
  // --- ส่วนสำคัญ: ปิดตัวตรวจจับไฟตก ---
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  
  Serial.begin(115200);
  
  // ตั้งค่าขาใหม่
  pinMode(slot1Pin, OUTPUT);
  pinMode(slot2Pin, OUTPUT);
  digitalWrite(slot1Pin, LOW);
  digitalWrite(slot2Pin, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (isSlot1Running && (millis() - slot1Timer >= runTime)) funcSlot1_Stop();
  if (isSlot2Running && (millis() - slot2Timer >= runTime)) funcSlot2_Stop();
}