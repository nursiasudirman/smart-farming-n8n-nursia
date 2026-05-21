#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

// --- KONFIGURASI PIN & PARAMETER SENSOR ---
#define LED_PIN 25
#define BAT_PIN 35

// Pin & Variabel Soil Moisture
#define SOIL_MOISTURE_PIN 34
const int AirValue = 3500;
const int WaterValue = 1500;
int soilMoisturePercent = 0;

// Variabel Baterai
float batVoltage = 0.0;
int batPercent = 0;

// --- KONFIGURASI OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 21
#define OLED_SCL 22

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

// --- VARIABEL GLOBAL ---
int currentRssi = 0; 
unsigned long previousMillis = 0;
const long interval = 5000; // Interval pengiriman 5 detik

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi LED Bawaan
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  analogReadResolution(12);
  
  Serial.println("Inisialisasi Sistem Sensor Node 1...");
  
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
  display.println("Node 1 Starting...");
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
    readSoilMoisture();
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

void readSoilMoisture() {
  int rawValue = analogRead(SOIL_MOISTURE_PIN);
  soilMoisturePercent = map(rawValue, AirValue, WaterValue, 0, 100);
  soilMoisturePercent = constrain(soilMoisturePercent, 0, 100);
  Serial.printf("Soil Moisture: %d%%\n", soilMoisturePercent);
}

// --- FUNGSI PENGIRIMAN DATA LORA ---

void sendDataLoRa() {
  digitalWrite(LED_PIN, HIGH); 
  
  // Format Payload KHUSUS NODE 1 HANYA: SOIL MOISTURE
  // Contoh: NODE1|soil=67
  String payload = "NODE1|soil=" + String(soilMoisturePercent);

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
  
  // Menampilkan Kelembapan Tanah (Tengah/Besar)
  display.setCursor(0, 0);
  display.println("Node 1 - Soil Sensor");
  
  display.drawLine(0, 12, 128, 12, WHITE);
  
  display.setTextSize(2);
  display.setCursor(0, 25); 
  display.print("Soil:"); 
  display.print(soilMoisturePercent); 
  display.println("%");
  
  // Mengembalikan ukuran teks ke normal untuk baris bawah
  display.setTextSize(1);
  
  // Posisi Rata Kiri & Kanan di bagian bawah
  int16_t x1, y1; 
  uint16_t w, h;

  // 1. Indikator BATERAI (Kiri Bawah)
  display.setCursor(0, 54); 
  display.print("BAT:" + String(batPercent) + "%");
  
  // 2. Indikator RSSI (Kanan Bawah)
  String rssiTxt = "RSSI:" + String(currentRssi);
  display.getTextBounds(rssiTxt, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(128 - w, 54); 
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