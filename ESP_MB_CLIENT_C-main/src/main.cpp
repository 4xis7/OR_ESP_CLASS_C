#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <esp_now.h>
#include <SPIFFS.h>

// =======================================================
// CLIENT C NODE
// =======================================================

#define RXD1 25
#define TXD1 26
#define BUTTON_PIN 35  // <-- เพิ่มพินปุ่มกดตามที่ต้องการ

// ================== LED ==================
const int LED_wifi  = 18; // ไฟสีเหลือง (ESP-NOW Status)
const int LED_RS    = 17; // ไฟสีเขียว (RS232 Status)
const int LED_power = 19; // ไฟสีแดง (Power Status)

// =====================================================
// AP CONFIG
// =====================================================
const char* AP_SSID     = "ESP32_CONFIG_C";
const char* AP_PASSWORD = "12345678";

// =====================================================
// GLOBAL
// =====================================================
Preferences prefs;
WebServer   server(80);

uint8_t peerMac[6];
bool    peerReady = false;
bool    configMode = false; // ตัวแปรเช็คว่าอยู่ในโหมดตั้งค่าหรือไม่

esp_now_peer_info_t peerInfo;

TaskHandle_t Task1;
TaskHandle_t Task2;

void Task1code(void * pvParameters);
void Task2code(void * pvParameters);

// (คงฟังก์ชัน isValidMac และ macStringToBytes ไว้เหมือนเดิมของคุณ)
bool isValidMac(String mac) {
    if (mac.length() != 17) return false;
    for (int i = 0; i < 17; i++) {
        if (i == 2 || i == 5 || i == 8 || i == 11 || i == 14) {
            if (mac[i] != ':') return false;
        } else {
            if (!isxdigit(mac[i])) return false;
        }
    }
    return true;
}

bool macStringToBytes(String macStr, uint8_t *mac) {
    if (macStr.length() != 17) return false;
    int values[6];
    if (sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x", &values[0], &values[1], &values[2], &values[3], &values[4], &values[5]) != 6) {
        return false;
    }
    for (int i = 0; i < 6; ++i) mac[i] = (uint8_t)values[i];
    return true;
}

// ปรับปรุงฟังก์ชันโหลด PEER เพื่อคืนค่าสถานะว่ามี MAC ไหม
bool loadPeer() {
    prefs.begin("config", true);
    String macStr = prefs.getString("peer", "");
    prefs.end();

    if (macStr == "" || !isValidMac(macStr)) {
        Serial.println("NO PEER OR INVALID MAC");
        return false; 
    }

    Serial.print("PEER = ");
    Serial.println(macStr);

    if (!macStringToBytes(macStr, peerMac)) {
        return false;
    }

    memcpy(peerInfo.peer_addr, peerMac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
        Serial.println("PEER ADDED");
        peerReady = true;
        return true;
    } else {
        Serial.println("ADD PEER FAIL");
        return false;
    }
}

// Web Handlers คงเดิม...
void handleRoot() {
    if (!SPIFFS.exists("/index.html")) { server.send(404, "text/plain", "index.html not found"); return; }
    File f = SPIFFS.open("/index.html", "r");
    String html = "";
    while (f.available()) html += (char)f.read();
    f.close();
    prefs.begin("config", true);
    String savedPeer = prefs.getString("peer", "NOT SET");
    prefs.end();
    html.replace("%MY_MAC%", WiFi.macAddress());
    html.replace("%PEER_MAC%", savedPeer);
    server.send(200, "text/html", html);
}

void handleSave() {
    if (!server.hasArg("mac")) { server.send(400, "text/plain", "No MAC"); return; }
    String mac = server.arg("mac");
    mac.trim(); mac.toUpperCase();
    if (!isValidMac(mac)) { server.send(200, "text/plain", "ERROR: Invalid MAC Format"); return; }
    prefs.begin("config", false);
    prefs.putString("peer", mac);
    prefs.end();
    Serial.println("MAC SAVED: " + mac);
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
}

void handleRestart() {
    server.send(200, "text/plain", "RESTARTING");
    delay(1000);
    ESP.restart();
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        Serial.println("OK");
        digitalWrite(LED_wifi, HIGH); // เงื่อนไขที่ 6: ส่งสำเร็จ ไฟสีเหลืองติดค้าง
    } else {
        Serial.println("FAIL");
        digitalWrite(LED_wifi, LOW);  // เงื่อนไขที่ 7: ส่งไม่สำเร็จ ไฟสีเหลืองดับ
    }
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    String dataIn = "";
    for (int i = 0; i < len; i++) dataIn += (char)incomingData[i];
    Serial.print(dataIn);
    Serial1.print(dataIn);
    Serial.flush();
}

// =====================================================
// SETUP
// =====================================================
void setup() {
    Serial.begin(9600);
    Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1);

    pinMode(LED_power, OUTPUT);
    pinMode(LED_wifi,  OUTPUT);
    pinMode(LED_RS,    OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP); // กำหนดปุ่มกด GPIO 35 แบบใช้ internal pullup 

    digitalWrite(LED_power, HIGH); // เงื่อนไขที่ 1: เปิดเครื่องไฟสีแดงติดค้าง
    digitalWrite(LED_wifi,  LOW);
    digitalWrite(LED_RS,    LOW);

    if (!SPIFFS.begin(true)) Serial.println("SPIFFS ERROR");

    WiFi.mode(WIFI_AP_STA); // เปิดทั้งสองโหมดเพื่อให้ยืดหยุ่น

    // เช็คเงื่อนไขที่ 3: หากกดปุ่ม GPIO 35 ค้างไว้ตอนเปิดเครื่อง ให้เข้าสู่โหมดตั้งค่าทันที
    // (หรือกดระหว่างทำงานปกติก็ได้ โดยเราจะไปตรวจใน loop)
    if (digitalWrite(LED_power, HIGH) && digitalRead(BUTTON_PIN) == LOW) {
        configMode = true;
        Serial.println("Forced to Web Server Mode via Button");
    }

    // เช็คเงื่อนไขที่ 2: ถ้าโหลดค่าพีเนียร์/MAC ไม่สำเร็จ ให้เข้าโหมด Web Server
    if (!loadPeer()) {
        configMode = true;
    }

    if (configMode) {
        // เริ่มต้นโหมด Web Server
        WiFi.softAP(AP_SSID, AP_PASSWORD);
        Serial.println("CONFIG MODE ACTIVE");
        Serial.println(WiFi.softAPIP());

        server.on("/",        HTTP_GET,  handleRoot);
        server.on("/save",    HTTP_POST, handleSave);
        server.on("/restart", HTTP_POST, handleRestart);
        server.begin();
        
        // **ตัดการทำงานส่วนอื่นออก** ไม่รัน Task FreeRTOS จนกว่าจะเซ็ตค่าเสร็จและสั่งรีสตาร์ท
        return; 
    }

    // หากมีค่า MAC ปกติ และไม่ได้กดปุ่ม ให้เริ่มทำงานส่วนส่งข้อมูลตามปกติ
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP NOW ERROR");
        return;
    }

    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);

    xTaskCreatePinnedToCore(Task1code, "Task1", 10000, NULL, 0, &Task1, 0);
    delay(500);
    xTaskCreatePinnedToCore(Task2code, "Task2", 10000, NULL, 0, &Task2, 1);
    delay(500);
}

// =====================================================
// TASK1 - Serial Relay -> ESP-NOW
// =====================================================
void Task1code(void * pvParameters) {
    for (;;) {
        if (Serial1.available() > 0) {
            // เงื่อนไขที่ 5: มีข้อมูลเข้า ให้ไฟสถานะสีเขียวกะพริบอย่างรวดเร็ว
            for(int i=0; i<3; i++) { // สั่งให้กะพริบเร็วๆ 3 ครั้งตอนมีข้อมูลเข้า
                digitalWrite(LED_RS, HIGH); delay(50);
                digitalWrite(LED_RS, LOW);  delay(50);
            }

            String msg = Serial1.readStringUntil('\r');
            String data_send = "MB3L:" + msg + "\r\n";

            if (peerReady) {
                esp_now_send(peerMac, (uint8_t*)data_send.c_str(), data_send.length());
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void Task2code(void * pvParameters) {
    for (;;) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// =====================================================
// LOOP
// =====================================================
void loop() {
    if (configMode) {
        server.handleClient(); // เลื่อน Web Server มารันตรงนี้ในโหมด Config
    } else {
        // เงื่อนไขที่ 3: ระหว่างรันปกติ หากผู้ใช้กดปุ่ม GPIO 35 ให้เข้าสู่โหมดเว็บเซิร์ฟเวอร์
        if (digitalRead(BUTTON_PIN) == LOW) {
            Serial.println("Button Pressed! Restarting to Config Mode...");
            // วิธีที่เคลียร์ที่สุดคือ ลบ flag ชั่วคราว หรือสั่งบังคับเปิด AP แล้วรีสตาร์ทบอร์ดเข้าโหมด config
            prefs.begin("config", false);
            prefs.putString("peer", ""); // ล้างค่าชั่วคราวเพื่อบังคับให้เข้าเว็บเซิร์ฟเวอร์ตอนรีบูต
            prefs.end();
            delay(500);
            ESP.restart(); 
        }
    }
    delay(100);
}
