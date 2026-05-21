#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================
// WIFI
// =====================================================
const char* ssid = "NOTE 30";
const char* password = "12345678";

// =====================================================
// WEBHOOK N8N
// =====================================================
String webhookURL =
"https://n8n-rjk7kfxekmlh.jkt1.sumopod.my.id/webhook-test/ed161526-b53a-4958-ac8c-70c4294ade9c";

// =====================================================
// OLED
// =====================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// =====================================================
// PIN TTGO T3 V1.6.1
// =====================================================
#define SCK     5
#define MISO    19
#define MOSI    27
#define SS      18
#define RST     14
#define DIO0    26

// =====================================================
// FREKUENSI
// =====================================================
#define BAND 433E6

String incoming = "";

// =====================================================
// STORAGE NODE DATA
// =====================================================
bool node1Ready = false;
bool node2Ready = false;

String soilValue = "";

int nitrogenVal = 0;
int phosphorusVal = 0;
int potassiumVal = 0;

float temperatureVal = 0;
float humidityVal = 0;

// =====================================================
// SETUP
// =====================================================
void setup() {

  Serial.begin(115200);
  delay(1000);

  // =====================================================
  // OLED START
  // =====================================================
  Wire.begin(21, 22);

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C)) {

    Serial.println("OLED gagal start!");

    while (1);
  }

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();

  // =====================================================
  // WIFI CONNECT
  // =====================================================
  WiFi.begin(ssid, password);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connecting WiFi");
  display.display();

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");
  Serial.print("IP : ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Connected");
  display.println(WiFi.localIP());
  display.display();

  delay(2000);

  // =====================================================
  // SPI
  // =====================================================
  SPI.begin(SCK, MISO, MOSI, SS);

  // =====================================================
  // LORA
  // =====================================================
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {

    Serial.println("LoRa gagal!");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("LoRa FAILED!");
    display.display();

    while (1);
  }

  LoRa.setSyncWord(0xF3);

  Serial.println("LoRa berhasil!");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("LoRa READY");
  display.println("Waiting data...");
  display.display();
}

// =====================================================
// LOOP
// =====================================================
void loop() {

  int packetSize = LoRa.parsePacket();

  if (packetSize) {

    incoming = "";

    while (LoRa.available()) {

      incoming += (char)LoRa.read();
    }

    Serial.println("=================================");
    Serial.println("RAW DATA:");
    Serial.println(incoming);
    Serial.println("=================================");

    processData(incoming);
  }
}

// =====================================================
// PROCESS DATA
// =====================================================
void processData(String data) {

  // =====================================================
  // NODE1
  // FORMAT:
  // NODE1|soil=65
  // =====================================================
  if (data.startsWith("NODE1")) {

    int soilIndex = data.indexOf("soil=");

    if (soilIndex != -1) {

      String soil =
        data.substring(soilIndex + 5);

      int rssi = LoRa.packetRssi();
      float snr = LoRa.packetSnr();

      // =====================================================
      // SERIAL
      // =====================================================
      Serial.println("NODE1 diterima");

      // =====================================================
      // OLED
      // =====================================================
      display.clearDisplay();

      display.setTextSize(1);

      display.setCursor(0, 0);
      display.println("NODE1");

      display.setCursor(0, 15);
      display.print("Soil : ");
      display.println(soil);

      display.setCursor(0, 30);
      display.print("RSSI : ");
      display.println(rssi);

      display.display();

      // =====================================================
      // SAVE DATA
      // =====================================================
      soilValue = soil;

      node1Ready = true;

      // =====================================================
      // CHECK SEND
      // =====================================================
      checkAndSendCombined();
    }
  }

  // =====================================================
  // NODE2
  // FORMAT:
  // NODE2|n=120,p=40,k=80,temp=29.4,hum=78
  // =====================================================
  else if (data.startsWith("NODE2")) {

    String payload =
      data.substring(data.indexOf("|") + 1);

    int nVal    = getIntValue(payload, "n=");
    int pVal    = getIntValue(payload, "p=");
    int kVal    = getIntValue(payload, "k=");

    float tVal  = getFloatValue(payload, "temp=");
    float hVal  = getFloatValue(payload, "hum=");

    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();

    // =====================================================
    // SERIAL
    // =====================================================
    Serial.println("NODE2 diterima");

    // =====================================================
    // OLED
    // =====================================================
    display.clearDisplay();

    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("NODE2");

    display.setCursor(0, 10);
    display.print("N:");
    display.print(nVal);

    display.print(" P:");
    display.println(pVal);

    display.setCursor(0, 20);
    display.print("K:");
    display.print(kVal);

    display.setCursor(0, 30);
    display.print("T:");
    display.print(tVal);

    display.print(" H:");
    display.println(hVal);

    display.setCursor(0, 45);
    display.print("RSSI:");
    display.println(rssi);

    display.display();

    // =====================================================
    // SAVE DATA
    // =====================================================
    nitrogenVal = nVal;
    phosphorusVal = pVal;
    potassiumVal = kVal;

    temperatureVal = tVal;
    humidityVal = hVal;

    node2Ready = true;

    // =====================================================
    // CHECK SEND
    // =====================================================
    checkAndSendCombined();
  }

  // =====================================================
  // UNKNOWN FORMAT
  // =====================================================
  else {

    Serial.println("Format data tidak dikenali!");
  }
}

// =====================================================
// CHECK & SEND COMBINED DATA
// =====================================================
void checkAndSendCombined() {

  // =====================================================
  // KIRIM JIKA SEMUA NODE SUDAH ADA
  // =====================================================
  if (node1Ready && node2Ready) {

    String finalJson = "{";

    finalJson += "\"gateway\":\"LORA_GATEWAY\",";

    // =====================================================
    // NODE1
    // =====================================================
    finalJson += "\"soil_sensor\":{";

    finalJson += "\"soil_moisture\":";
    finalJson += soilValue;

    finalJson += "},";

    // =====================================================
    // NODE2
    // =====================================================
    finalJson += "\"environment_sensor\":{";

    finalJson += "\"nitrogen\":";
    finalJson += String(nitrogenVal);
    finalJson += ",";

    finalJson += "\"phosphorus\":";
    finalJson += String(phosphorusVal);
    finalJson += ",";

    finalJson += "\"potassium\":";
    finalJson += String(potassiumVal);
    finalJson += ",";

    finalJson += "\"temperature\":";
    finalJson += String(temperatureVal);
    finalJson += ",";

    finalJson += "\"humidity\":";
    finalJson += String(humidityVal);

    finalJson += "}";

    finalJson += "}";

    // =====================================================
    // SERIAL
    // =====================================================
    Serial.println("=================================");
    Serial.println("FINAL JSON");
    Serial.println(finalJson);
    Serial.println("=================================");

    // =====================================================
    // SEND TO WEBHOOK
    // =====================================================
    sendToWebhook(finalJson);

    // =====================================================
    // RESET FLAG
    // =====================================================
    node1Ready = false;
    node2Ready = false;
  }
}

// =====================================================
// SEND TO WEBHOOK
// =====================================================
void sendToWebhook(String jsonData) {

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    http.begin(webhookURL);

    http.addHeader(
      "Content-Type",
      "application/json"
    );

    Serial.println("Mengirim ke n8n...");
    Serial.println(jsonData);

    int httpResponseCode =
      http.POST(jsonData);

    Serial.print("HTTP Response : ");
    Serial.println(httpResponseCode);

    String response =
      http.getString();

    Serial.println("Response:");
    Serial.println(response);

    if (httpResponseCode > 0) {

      Serial.println("Data berhasil dikirim!");

    } else {

      Serial.println("Gagal kirim data!");
    }

    http.end();
  }

  else {

    Serial.println("WiFi Disconnect!");
  }
}

// =====================================================
// GET INTEGER
// =====================================================
int getIntValue(String data, String key) {

  int start = data.indexOf(key);

  if (start == -1) return 0;

  start += key.length();

  int end = data.indexOf(",", start);

  if (end == -1)
    end = data.length();

  return data.substring(start, end).toInt();
}

// =====================================================
// GET FLOAT
// =====================================================
float getFloatValue(String data, String key) {

  int start = data.indexOf(key);

  if (start == -1) return 0;

  start += key.length();

  int end = data.indexOf(",", start);

  if (end == -1)
    end = data.length();

  return data.substring(start, end).toFloat();
}