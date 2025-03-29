#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// Thông tin WiFi
const char* ssid = "Phuong Anh";
const char* password = "30032020";
const char* serverUrl = "http://192.168.0.11:5000/update";  

// Cấu hình cảm biến
#define DHTPIN 27
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Chân cảm biến
#define LIGHT_SENSOR 34  
#define WATER_SENSOR 35  

void setup() {
    Serial.begin(115200);
    Serial2.begin(9600, SERIAL_8N1, 17, 16); // RX = D17, TX = D16
    dht.begin();  

    // Kết nối WiFi
    WiFi.begin(ssid, password);
    Serial.print("🔄 Đang kết nối WiFi...");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) { 
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi đã kết nối!");
    } else {
        Serial.println("\n❌ Kết nối WiFi thất bại! Kiểm tra SSID & Password.");
    }
}

void loop() {
    int soilMoisture = -1;

    // Nhận dữ liệu độ ẩm đất từ Arduino (qua UART)
    if (Serial2.available()) {
        int value = Serial2.parseInt();
        if (value > 0) {  // Đảm bảo giá trị hợp lệ
            soilMoisture = value;
        }
    }

    // Đọc các cảm biến
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int lightValue = analogRead(LIGHT_SENSOR);
    int waterLevel = analogRead(WATER_SENSOR);

    // Kiểm tra lỗi DHT11
    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("❌ Lỗi đọc cảm biến DHT11! Không gửi dữ liệu.");
        return;
    }

    // In ra Serial để debug
    Serial.println("\n📡 Dữ liệu cảm biến:");
    Serial.print("🌡️ Nhiệt độ: "); Serial.println(temperature);
    Serial.print("💧 Độ ẩm không khí: "); Serial.println(humidity);
    Serial.print("🌱 Độ ẩm đất: "); Serial.println(soilMoisture);
    Serial.print("☀️ Ánh sáng: "); Serial.println(lightValue);
    Serial.print("🚰 Mực nước: "); Serial.println(waterLevel);

    // Gửi dữ liệu nếu WiFi vẫn kết nối
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");

        // Tạo JSON dữ liệu
        String jsonData = "{";
        jsonData += "\"temperature\": " + String(temperature, 2) + ",";  // Làm tròn 2 chữ số
        jsonData += "\"humidity\": " + String(humidity, 2) + ",";
        jsonData += "\"soil_moisture\": " + String(soilMoisture) + ",";
        jsonData += "\"light\": " + String(lightValue) + ",";
        jsonData += "\"water_level\": " + String(waterLevel);
        jsonData += "}";

        Serial.println("📤 Gửi dữ liệu: " + jsonData);
        int httpResponseCode = http.POST(jsonData);
        // Kiểm tra phản hồi từ server
        Serial.print("📡 Mã phản hồi HTTP: ");
        Serial.println(httpResponseCode);

        if (httpResponseCode > 0) {
            Serial.println("✅ Dữ liệu đã gửi thành công!");
        } else {
            Serial.println("❌ Gửi dữ liệu thất bại!");
        }

        http.end();
    } else {
        Serial.println("⚠️ Mất kết nối WiFi! Đang chờ kết nối lại...");
    }

    delay(5000);  // Gửi dữ liệu mỗi 5 giây
}