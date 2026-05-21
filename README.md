# Smart Farming Automation - Nursia

Project ini adalah sistem Smart Farming berbasis ESP32, LoRa, n8n, Google Sheets, Gemini AI, OpenWeather, dan Telegram.

Sistem ini digunakan untuk membaca data sensor pertanian, mengirim data dari node LoRa ke gateway ESP32, lalu meneruskan data ke workflow n8n untuk dianalisis dan dikirim sebagai notifikasi.

## Fitur Project

- Membaca data sensor pertanian dari node LoRa
- Mengirim data sensor ke gateway ESP32
- Mengirim data dari ESP32 Gateway ke Webhook n8n
- Mengambil data cuaca dari OpenWeather
- Memproses data sensor menggunakan JavaScript
- Menganalisis data NPK menggunakan Gemini AI
- Menyimpan data ke Google Sheets
- Mengirim hasil analisis ke Telegram

## File Project

- `ESP32Gateway.ino`  
  Program ESP32 sebagai gateway untuk menerima data dari node LoRa dan mengirimkannya ke Webhook n8n.

- `LoRa32-PERPASIF_NODE_1_TX.ino`  
  Program node LoRa 1 untuk mengirim data sensor ke gateway.

- `LoRa32-PERPASIF_NODE_2_TX.ino`  
  Program node LoRa 2 untuk mengirim data sensor ke gateway.

- `UAS-PERPASIF-NURSIA.json`  
  File workflow n8n yang berisi alur otomatisasi mulai dari Webhook, HTTP Request OpenWeather, Gemini AI, Google Sheets, hingga Telegram.

- `code.js`  
  Kode JavaScript pada node n8n untuk mengambil data dari Webhook dan OpenWeather, lalu menggabungkannya menjadi data sensor, data cuaca, dan prompt AI.

## Alur Kerja Sistem

1. Node LoRa membaca data sensor pertanian.
2. Data dikirim ke ESP32 Gateway.
3. ESP32 Gateway mengirim data ke Webhook n8n.
4. n8n mengambil data cuaca dari OpenWeather.
5. Data sensor dan cuaca diproses menggunakan JavaScript.
6. Gemini AI memberikan analisis kondisi lahan dan rekomendasi pertanian.
7. Hasil data disimpan ke Google Sheets.
8. Rekomendasi dikirim ke Telegram.

## Tools yang Digunakan

- ESP32
- LoRa32
- Arduino IDE
- n8n
- OpenWeather API
- Google Gemini AI
- Google Sheets
- Telegram Bot
- GitHub

## Cara Menggunakan

1. Upload program Arduino ke masing-masing board ESP32 atau LoRa32.
2. Import file workflow n8n ke aplikasi n8n.
3. Atur kembali credentials untuk Google Sheets, Gemini AI, dan Telegram.
4. Ganti API key OpenWeather sesuai akun yang digunakan.
5. Jalankan workflow n8n.
6. Kirim data sensor dari ESP32 Gateway ke Webhook n8n.
7. Cek hasil data pada Google Sheets dan Telegram.
