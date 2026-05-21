// ============================================
// Ambil data dari Webhook (ESP32/LoRa)
// ============================================
const webhookData = $('Webhook').first().json.body;

const gateway        = webhookData.gateway;
const soilMoisture   = webhookData.soil_sensor.soil_moisture;
const nitrogen       = webhookData.environment_sensor.nitrogen;
const phosphorus     = webhookData.environment_sensor.phosphorus;
const potassium      = webhookData.environment_sensor.potassium;
const temperature    = webhookData.environment_sensor.temperature;
const humidity       = webhookData.environment_sensor.humidity;

// ============================================
// Ambil data dari HTTP Request (OpenWeather)
// ============================================
const weatherData     = $input.first().json;

const cityName        = weatherData.name;
const weatherDesc     = weatherData.weather[0].description;
const weatherTemp     = weatherData.main.temp;
const weatherHumidity = weatherData.main.humidity;
const windSpeed       = weatherData.wind.speed;

// ============================================
// Gabungkan & buat output rapi
// ============================================
const output = {
  timestamp: new Date().toISOString(),
  gateway: gateway,

  sensor_data: {
    soil_moisture : soilMoisture,
    nitrogen      : nitrogen,
    phosphorus    : phosphorus,
    potassium     : potassium,
    temperature   : temperature,
    humidity      : humidity,
  },

  weather_data: {
    city        : cityName,
    description : weatherDesc,
    temperature : weatherTemp,
    humidity    : weatherHumidity,
    wind_speed  : windSpeed,
  },

  ai_prompt: `
Kamu adalah asisten pertanian cerdas. Berikut data dari lapangan:

📡 Gateway: ${gateway}

🌱 Sensor Tanah & Lingkungan:
- Kelembaban Tanah : ${soilMoisture}%
- Nitrogen (N)     : ${nitrogen} mg/kg
- Phosphorus (P)   : ${phosphorus} mg/kg
- Potassium (K)    : ${potassium} mg/kg
- Suhu             : ${temperature}°C
- Kelembaban Udara : ${humidity}%

🌤️ Kondisi Cuaca di ${cityName}:
- Cuaca     : ${weatherDesc}
- Suhu      : ${weatherTemp}°C
- Kelembaban: ${weatherHumidity}%
- Angin     : ${windSpeed} m/s

Berikan analisis kondisi lahan dan rekomendasi tindakan pertanian yang perlu dilakukan hari ini.
  `.trim()
};

return output;