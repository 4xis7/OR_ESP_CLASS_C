#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <esp_now.h>
#include <SPIFFS.h>

// =======================================================
// CLIENT C NODE (ระบบเดิม + เพิ่มฟังก์ชันปุ่มและ Web Config)
// =======================================================

#define RXD1 25
#define TXD1 26
<<<<<<< HEAD
#define BUTTON_PIN 35  // ปุ่มกดสำหรับเข้าโหมด Web Server (ต่อแบบ Active LOW)

// ================== LED ==================
const int LED_wifi  = 18; // ไฟสีเหลือง (ESP-NOW Status)
const int LED_RS    = 17; // ไฟสีเขียว (RS232 Data Traffic)
const int LED_power = 19; // ไฟสีแดง (Power)
=======
#define BUTTON_PIN 35  // <-- เพิ่มพินปุ่มกดตามที่ต้องการ

// ================== LED ==================
const int LED_wifi  = 18; // ไฟสีเหลือง (ESP-NOW Status)
const int LED_RS    = 17; // ไฟสีเขียว (RS232 Status)
const int LED_power = 19; // ไฟสีแดง (Power Status)
>>>>>>> 7406e2d7fb118b6ad2f578315ff196f7e202386d

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
<<<<<<< HEAD
bool    configMode = false;

int buttonState = LOW;
int lastButtonState = LOW;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
=======
bool    configMode = false; // ตัวแปรเช็คว่าอยู่ในโหมดตั้งค่าหรือไม่
>>>>>>> 7406e2d7fb118b6ad2f578315ff196f7e202386d

esp_now_peer_info_t peerInfo;

TaskHandle_t Task1;
TaskHandle_t Task2;

void Task1code(void * pvParameters);
void Task2code(void * pvParameters);

<<<<<<< HEAD
// =====================================================
// ตรวจสอบ MAC Format (XX:XX:XX:XX:XX:XX)
// =====================================================
bool isValidMac(String mac)
{
    if (mac.length() != 17)
        return false;

    for (int i = 0; i < 17; i++)
    {
        if (i == 2 || i == 5 || i == 8 || i == 11 || i == 14)
        {
            if (mac[i] != ':')
                return false;
        }
        else
        {
            if (!isxdigit(mac[i]))
                return false;
=======
// (คงฟังก์ชัน isValidMac และ macStringToBytes ไว้เหมือนเดิมของคุณ)
bool isValidMac(String mac) {
    if (mac.length() != 17) return false;
    for (int i = 0; i < 17; i++) {
        if (i == 2 || i == 5 || i == 8 || i == 11 || i == 14) {
            if (mac[i] != ':') return false;
        } else {
            if (!isxdigit(mac[i])) return false;
>>>>>>> 7406e2d7fb118b6ad2f578315ff196f7e202386d
        }
    }
    return true;
}

<<<<<<< HEAD
// =====================================================
// MAC STRING -> BYTE
// =====================================================
bool macStringToBytes(String macStr, uint8_t *mac)
{
    if (macStr.length() != 17)
        return false;

=======
bool macStringToBytes(String macStr, uint8_t *mac) {
    if (macStr.length() != 17) return false;
>>>>>>> 7406e2d7fb118b6ad2f578315ff196f7e202386d
    int values[6];
    if (sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x", &values[0], &values[1], &values[2], &values[3], &values[4], &values[5]) != 6) {
        return false;
    }
    for (int i = 0; i < 6; ++i) mac[i] = (uint8_t)values[i];
    return true;
}

<<<<<<< HEAD
// =====================================================
// LOAD PEER MAC จาก NVS
// =====================================================
void loadPeer()
{
=======
// ปรับปรุงฟังก์ชันโหลด PEER เพื่อคืนค่าสถานะว่ามี MAC ไหม
bool loadPeer() {
>>>>>>> 7406e2d7fb118b6ad2f578315ff196f7e202386d
    prefs.begin("config", true);
    String macStr = prefs.getString("peer", "");
    prefs.end();

<<<<<<< HEAD
    if (macStr == "")
    {
        Serial.println("NO PEER SAVED");
        return;
=======
    if (macStr == "" || !isValidMac(macStr)) {
        Serial.println("NO PEER OR INVALID MAC");
        return false; 
>>>>>>> 7406e2d7fb118b6ad2f578315ff196f7e202386d
    }

    Serial.print("PEER = ");
    Serial.println(macStr);

    if (!macStringToBytes(macStr, peerMac)) {
        return false;
    }

    memcpy(peerInfo.peer_addr, peerMac, 6);  
    peerInfo.channel = 0; // ตามระบบเดิมของคุณ
    peerInfo.encrypt = false;
    esp_now_del_peer(peerMac); 

<<<<<<< HEAD
    esp_err_t result = esp_now_add_peer(&peerInfo);

    Serial.print("ADD PEER RESULT: ");
    Serial.println(esp_err_to_name(result));

    if (result == ESP_OK)
    {
=======
    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
>>>>>>> 7406e2d7fb118b6ad2f578315ff196f7e202386d
        Serial.println("PEER ADDED");
        peerReady = true;
        return true;
    } else {
        Serial.println("ADD PEER FAIL");
<<<<<<< HEAD
        peerReady = false;
    }
}

// =====================================================
// WEB HANDLERS
// =====================================================
void handleRoot()
{
    if (!SPIFFS.exists("/index.html"))
    {
        server.send(404, "text/plain", "index.html not found");
        return;
    }

=======
        return false;
    }
}

// Web Handlers คงเดิม...
void handleRoot() {
    if (!SPIFFS.exists("/index.html")) { server.send(404, "text/plain", "index.html not found"); return; }
>>>>>>> 7406e2d7fb118b6ad2f578315ff196f7e202386d
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
<<<<<<< HEAD
    mac.trim();
    mac.toUpperCase();

    if (!isValidMac(mac))
    {
        String errorHtml = "<html><meta charset='UTF-8'><body style='font-family:sans-serif; text-align:center; padding-top:50px;'>";
        errorHtml += "<h2 style='color:red;'>ERROR: รูปแบบ MAC Address ไม่ถูกต้อง!</h2>";
        errorHtml += "<p>กรุณากรอกในรูปแบบ XX:XX:XX:XX:XX:XX</p>";
        errorHtml += "<br><a href='/' style='padding:10px 20px; background:#ccc; text-decoration:none; color:black; border-radius:5px;'>กลับไปแก้ไข</a>";
        errorHtml += "</body></html>";
        server.send(200, "text/html", errorHtml);
        return;
    }

=======
    mac.trim(); mac.toUpperCase();
    if (!isValidMac(mac)) { server.send(200, "text/plain", "ERROR: Invalid MAC Format"); return; }
>>>>>>> 7406e2d7fb118b6ad2f578315ff196f7e202386d
    prefs.begin("config", false);
    prefs.putString("peer", mac);      
    prefs.end();
    Serial.println("MAC SAVED: " + mac);
<<<<<<< HEAD

    String successHtml = "<html><meta charset='UTF-8'><body style='font-family:sans-serif; text-align:center; padding-top:50px;'>";
    successHtml += "<div style='display:inline-block; border:2px solid #4CAF50; padding:30px; border-radius:10px; background-color:#f9f9f9;'>";
    successHtml += "<h2 style='color:#4CAF50;'>✓ บันทึกข้อมูลเรียบร้อยแล้ว!</h2>";
    successHtml += "<p style='font-size:18px;'>บันทึกค่าใหม่เป็น: <b>" + mac + "</b> สำเร็จ</p>";
    successHtml += "<p style='color:#666;'>กำลังเตรียมรีสตาร์ทบอร์ดเพื่อเข้าโหมดทำงานปกติ...</p>";
    successHtml += "</div>";
    successHtml += "</body></html>";

    server.send(200, "text/html", successHtml);

    delay(3000);   
    ESP.restart();
}

// =====================================================
// ฟังก์ชันเริ่มรันระบบ WEB SERVER CONFIG (โหมดตั้งค่า)
// =====================================================
void startConfigMode()
{
    configMode = true;
    Serial.println(">>> START CONFIG MODE (WEB SERVER Active) <<<");

    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    delay(500);

    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    server.on("/",        HTTP_GET,  handleRoot);
    server.on("/save",    HTTP_POST, handleSave);
    server.begin();

    while (configMode)
    {
        server.handleClient();
        delay(1);
    }
}

// =====================================================
// ESP-NOW SEND CALLBACK
// =====================================================
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if (status == ESP_NOW_SEND_SUCCESS)
    {
        Serial.println("OK");
        digitalWrite(LED_wifi, HIGH); 
    }
    else
    {
        Serial.println("FAIL");
        digitalWrite(LED_wifi, LOW);  
    }
}

// =====================================================
// ESP-NOW RECEIVE CALLBACK
// =====================================================
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    String dataIn = "";
    for (int i = 0; i < len; i++)
        dataIn += (char)incomingData[i];

=======
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
>>>>>>> 7406e2d7fb118b6ad2f578315ff196f7e202386d
    Serial.print(dataIn);
    Serial1.print(dataIn);
    Serial.flush();
}

// =====================================================
// SETUP
// =====================================================
<<<<<<< HEAD
void setup()
{
=======
void setup() {
>>>>>>> 7406e2d7fb118b6ad2f578315ff196f7e202386d
    Serial.begin(9600);
    Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1);

    pinMode(LED_power, OUTPUT);
    pinMode(LED_wifi,  OUTPUT);
    pinMode(LED_RS,    OUTPUT);
<<<<<<< HEAD
    pinMode(BUTTON_PIN, INPUT); 
=======
    pinMode(BUTTON_PIN, INPUT_PULLUP); // กำหนดปุ่มกด GPIO 35 แบบใช้ internal pullup 
>>>>>>> 7406e2d7fb118b6ad2f578315ff196f7e202386d

    digitalWrite(LED_power, HIGH); // เงื่อนไขที่ 1: เปิดเครื่องไฟสีแดงติดค้าง
    digitalWrite(LED_wifi,  LOW);
    digitalWrite(LED_RS,    LOW);

    if (!SPIFFS.begin(true)) Serial.println("SPIFFS ERROR");

<<<<<<< HEAD
    // ตรวจสอบข้อมูลจาก NVS Flash Memory
    prefs.begin("config", true);
    String macStr = prefs.getString("peer", "");
    prefs.end();

    // เงื่อนไข: ถ้ายังไม่มีการตั้งค่า MAC เลย ให้เปิดเข้าโหมด Config ทันที
    if (macStr == "")
    {
        Serial.println("NO MASTER MAC");
        startConfigMode();
    }

    // หากมีข้อมูลแล้ว จะรันในโหมดส่งข้อมูลปกติ
    WiFi.mode(WIFI_STA);
    Serial.println("NORMAL GATEWAY MODE ACTIVE");
    Serial.print("MY MAC: ");
    Serial.println(WiFi.macAddress());

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
=======
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
>>>>>>> 7406e2d7fb118b6ad2f578315ff196f7e202386d
        return;
    }

    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);

<<<<<<< HEAD
    loadPeer();

    // สร้าง Task1 รันบน Core 0 (จัดการปุ่มกด และส่ง Serial)
    xTaskCreatePinnedToCore(
        Task1code, "Task1",
        10000, NULL, 0, &Task1, 0); 
    delay(500);

    // สร้าง Task2 รันบน Core 1 (เปิดไว้สำหรับพัฒนาต่อตามระบบเดิม)
    xTaskCreatePinnedToCore(
        Task2code, "Task2",
        10000, NULL, 0, &Task2, 1);
=======
    xTaskCreatePinnedToCore(Task1code, "Task1", 10000, NULL, 0, &Task1, 0);
    delay(500);
    xTaskCreatePinnedToCore(Task2code, "Task2", 10000, NULL, 0, &Task2, 1);
>>>>>>> 7406e2d7fb118b6ad2f578315ff196f7e202386d
    delay(500);
}

// =====================================================
// TASK1 - Serial Relay -> ESP-NOW (Core 0)
// =====================================================
<<<<<<< HEAD
void Task1code(void * pvParameters)
{ 
    Serial.print("Task1 running on core ");
    Serial.println(xPortGetCoreID());

    for (;;)
    {
        // ---- ระบบตรวจจับปุ่มกดเพื่อเข้าโหมดตั้งค่า ----
        int reading = digitalRead(BUTTON_PIN);
        if (reading != lastButtonState)
        {
            lastDebounceTime = millis();
        }

        if ((millis() - lastDebounceTime) > debounceDelay)
        {
            if (reading != buttonState)
            {
                buttonState = reading;
                if (buttonState == HIGH) 
                {
                    Serial.println("ENTER CONFIG MODE");
                    startConfigMode();
                }
            }
        }
        lastButtonState = reading;

        // ---- ระบบอ่านค่าจาก Serial1 ดั้งเดิมของคุณ ----
        if (Serial1.available() > 0)
        {
            digitalWrite(LED_RS, HIGH);   

            // ใช้การตัดคำแบบเครื่องหมาย \r ตามระบบเดิม
            String msg = Serial1.readStringUntil('\r');
            
            digitalWrite(LED_RS, LOW);    

            String data_send = "MB3L:" + msg + "\r\n";
            
            if (peerReady)
            {
                esp_now_send(peerMac, (uint8_t*)data_send.c_str(), data_send.length());
            }
            delay(100);
        }
        
        vTaskDelay(1 / portTICK_PERIOD_MS); // ป้องกัน Task แย่งทรัพยากรระบบ CPU
    }
}

// =====================================================
// TASK2 - สแตนด์บายบน Core 1 (ตามโค้ดเดิม)
// =====================================================
void Task2code(void * pvParameters)
{
    Serial.print("Task2 running on core ");
    Serial.println(xPortGetCoreID());

    for (;;)
    {
=======
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
>>>>>>> 7406e2d7fb118b6ad2f578315ff196f7e202386d
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// =====================================================
// LOOP 
// =====================================================
<<<<<<< HEAD
void loop()
{
    delay(100); 
} 
=======
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
>>>>>>> 7406e2d7fb118b6ad2f578315ff196f7e202386d
