#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

// --- KONFIGURASI PIN & PARAMETER SENSOR ---
#define LED_PIN 25
#define BAT_PIN 35

float batVoltage = 0.0;
int batPercent = 0;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 21
#define OLED_SCL 22

#define DHTPIN 13
#define DHTTYPE DHT22

#define RX_PIN 4 
#define TX_PIN 2
#define MODBUS_BAUDRATE 4800

// --- KONFIGURASI PIN LORA (T3 LoRa32 V1.6.1) ---
#define LORA_SCK  5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_CS   18
#define LORA_RST  23
#define LORA_IRQ  26
#define LORA_FREQ 433E6 // Sesuaikan dengan modul: 433E6, 868E6, atau 915E6

// --- INISIALISASI OBJEK ---
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);

// --- VARIABEL GLOBAL ---
const byte npk_query[] = {0x01, 0x03, 0x00, 0x1E, 0x00, 0x03, 0x65, 0xCD};
byte npk_response[11];

float temp = 0.0, hum = 0.0;
int val_N = 0, val_P = 0, val_K = 0;

int currentRssi = 0; 
unsigned long previousMillis = 0;
const long interval = 5000; // Interval pengiriman 5 detik

void setup() {
  Serial.begin(115200);
  Serial2.begin(MODBUS_BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
  
  // Inisialisasi LED Bawaan
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  analogReadResolution(12);
  
  Serial.println("Inisialisasi Sistem Sensor Node 2...");
  dht.begin();
  
  // Inisialisasi OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(15, 25);
  display.println("Node 2 Starting...");
  display.display();
  
  // Inisialisasi LoRa
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);
  
  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("LoRa initialization failed!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("LoRa Failed!");
    display.display();
    while (1);
  }
  
  // Samakan SyncWord dengan Receiver
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initialized OK!");
  
  delay(2000);
  LoRa.receive(); 
}

void loop() {
  // --- 1. JADWAL PENGIRIMAN DATA SENSOR ---
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    readDHT();
    readNPK();
    readBattery();
    updateOLED();
    sendDataLoRa();
  }

  // --- 2. MENERIMA PESAN (RSSI / Balasan) ---
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    currentRssi = LoRa.packetRssi();
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }
    Serial.println("Menerima pesan: " + incoming + " dengan RSSI: " + String(currentRssi));
    updateOLED();
  }
}

// --- FUNGSI PEMBACAAN SENSOR ---

void readDHT() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    hum = h;
    temp = t;
  }
}

void readNPK() {
  while (Serial2.available()) Serial2.read();
  Serial2.write(npk_query, sizeof(npk_query));

  unsigned long startTime = millis();
  int bytesReceived = 0;

  while (millis() - startTime < 1000) {
    if (Serial2.available()) {
      npk_response[bytesReceived++] = Serial2.read();
      if (bytesReceived >= 11) break;
    }
  }

  if (bytesReceived == 11 && npk_response[0] == 0x01 && npk_response[1] == 0x03) {
    val_N = (npk_response[3] << 8) | npk_response[4];
    val_P = (npk_response[5] << 8) | npk_response[6];
    val_K = (npk_response[7] << 8) | npk_response[8];
    Serial.printf("NPK Berhasil: N=%d, P=%d, K=%d\n", val_N, val_P, val_K);
  } else {
    Serial.println("Peringatan: Gagal membaca NPK (Timeout / Invalid).");
  }
}

// --- FUNGSI PENGIRIMAN DATA LORA ---

void sendDataLoRa() {
  digitalWrite(LED_PIN, HIGH); 
  
  // Format Payload KHUSUS NODE 2 HANYA: N, P, K, TEMP, HUM
  // Contoh: NODE2|n=120,p=40,k=80,temp=29.4,hum=78.0
  String payload = "NODE2|n=" + String(val_N) + 
                   ",p=" + String(val_P) + 
                   ",k=" + String(val_K) + 
                   ",temp=" + String(temp, 1) + 
                   ",hum=" + String(hum, 1);

  // Mulai transmisi
  LoRa.beginPacket();
  LoRa.print(payload);
  int status = LoRa.endPacket();

  if(status == 1) {
    Serial.println("Data LoRa Terkirim: " + payload);
  } else {
    Serial.println("Gagal Mengirim LoRa!");
  }
  
  delay(500); // Nyalakan LED selama 500ms
  digitalWrite(LED_PIN, LOW); 
  
  LoRa.receive(); 
}

// --- FUNGSI TAMPILAN OLED ---

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  // Baris 1: Suhu & Kelembapan
  display.setCursor(0, 0);
  display.print("T:"); display.print(temp, 1); display.print("C  ");
  display.print("H:"); display.print(hum, 1); display.println("%");
  
  // Garis Pemisah horizontal
  display.drawLine(0, 15, 128, 15, WHITE);
  
  // Sensor NPK di sisi kiri
  display.setCursor(0, 22); display.print("N: "); display.print(val_N); display.println(" mg");
  display.setCursor(0, 34); display.print("P: "); display.print(val_P); display.println(" mg");
  display.setCursor(0, 46); display.print("K: "); display.print(val_K); display.println(" mg");
  
  // Posisi Rata Kanan (Right-Aligned)
  int16_t x1, y1; 
  uint16_t w, h;

  // 1. Indikator BATERAI
  String batTxt = "BAT:" + String(batPercent) + "%";
  display.getTextBounds(batTxt, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(128 - w, 22); 
  display.print(batTxt);
  
  // 2. Indikator RSSI
  String rssiTxt = "RSSI:" + String(currentRssi);
  display.getTextBounds(rssiTxt, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(128 - w, 46); 
  display.print(rssiTxt);
  
  display.display();
}

// --- FUNGSI PEMBACAAN BATERAI (DENGAN OVERSAMPLING) ---
void readBattery() {
  uint32_t sumAdc = 0;
  const int numSamples = 64; 
  
  for (int i = 0; i < numSamples; i++) {
    sumAdc += analogRead(BAT_PIN);
    delay(2); 
  }
  
  float avgAdc = (float)sumAdc / numSamples;
  batVoltage = (avgAdc / 4095.0) * 3.3 * 2.0;
  batPercent = ((batVoltage - 3.2) / (4.2 - 3.2)) * 100;
  batPercent = constrain(batPercent, 0, 100);
}