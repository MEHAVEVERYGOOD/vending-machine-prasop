#include <WiFi.h>
#include <PubSubClient.h>

// --- 1. ตั้งค่า Wi-Fi (แก้ตรงนี้) ---
const char* ssid = "1T1M";      // ชื่อ Wi-Fi
const char* password = "12345678";  // รหัส Wi-Fi

// --- 2. ตั้งค่า MQTT Broker (ใช้ EMQX) ---
const char* mqtt_server = "broker.emqx.io"; 
const int mqtt_port = 1883;

// --- 3. ตั้งชื่อ Topic (ช่องสัญญาณ) ---
// ถ้ากดไม่ติด ให้ลองเปลี่ยนชื่อ Topic ให้ไม่ซ้ำคนอื่น เช่นเติมเลขท้าย
const char* topic_slot1 = "1T1M_Project_Dev/slot1"; 
const char* topic_slot2 = "1T1M_Project_Dev/slot2";
const char* topic_status = "1T1M_Project_Dev/status";

// --- Hardware Pin (LED จำลอง Stepper) ---
const int slot1Pin = 26; 
const int slot2Pin = 27;

WiFiClient espClient;
PubSubClient client(espClient);

// ตัวแปรจับเวลา (Timer)
unsigned long slot1Timer = 0;
unsigned long slot2Timer = 0;
bool isSlot1Running = false;
bool isSlot2Running = false;
const long runTime = 3000; // 3 วินาที

// ==========================================
// Function สั่งงาน (รอ Stepper)
// ==========================================
void funcSlot1_Start() {
  if (!isSlot1Running) {
    digitalWrite(slot1Pin, HIGH); // เปิดไฟ
    isSlot1Running = true;
    slot1Timer = millis();
    Serial.println("Slot 1: STARTED");
    client.publish(topic_status, "Slot 1 Processing...");
  }
}

void funcSlot1_Stop() {
  digitalWrite(slot1Pin, LOW); // ปิดไฟ
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

// ==========================================
// รับคำสั่งจากหน้าเว็บ
// ==========================================
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Msg arrived ["); Serial.print(topic); Serial.print("]: ");
  Serial.println(message);

  if (String(topic) == topic_slot1 && (message == "1" || message == "ON")) {
    funcSlot1_Start();
  }
  else if (String(topic) == topic_slot2 && (message == "1" || message == "ON")) {
    funcSlot2_Start();
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(topic_slot1);
      client.subscribe(topic_slot2);
      client.publish(topic_status, "ESP32 Online & Ready");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
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

  // เช็คเวลาเพื่อปิด
  if (isSlot1Running && (millis() - slot1Timer >= runTime)) funcSlot1_Stop();
  if (isSlot2Running && (millis() - slot2Timer >= runTime)) funcSlot2_Stop();
}