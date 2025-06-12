#define BLYNK_TEMPLATE_ID "TMPL3-jz9sy2t"
#define BLYNK_TEMPLATE_NAME "Temperature Alert"
#define BLYNK_AUTH_TOKEN "Q6NeHIGgNLJacn2KWBt9NYIfpBmLLnqU"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// WiFi & Blynk credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Temp";
char pass[] = "12345678";

// DHT sensor setup
#define DHTPIN D5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Relay setup
#define RELAY_PIN D4
#define ALERT_TEMP 35

// Timers
unsigned long lastSensorRead = 0;
unsigned long lastWiFiCheck = 0;
const unsigned long sensorInterval = 2000;  // 2 seconds
const unsigned long wifiInterval = 5000;    // 5 seconds

bool wifiConnected = false;

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Booting...");
  delay(1500);
  lcd.clear();

  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  WiFi.begin(ssid, pass);
}

void loop() {
  // === Handle WiFi every 5 seconds ===
  if (millis() - lastWiFiCheck >= wifiInterval) {
    lastWiFiCheck = millis();

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected, retrying...");
      WiFi.begin(ssid, pass);
      wifiConnected = false;
    } else if (!wifiConnected) {
      Serial.println("WiFi connected.");
      Blynk.config(auth);
      Blynk.connect(1000);
      wifiConnected = true;
    }
  }

  if (wifiConnected) {
    Blynk.run();
  }

  // === Sensor + LCD update every 2 seconds ===
  if (millis() - lastSensorRead >= sensorInterval) {
    lastSensorRead = millis();

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      lcd.setCursor(0, 0);
      lcd.print("Sensor Error    ");
      return;
    }

    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature, 1);
    lcd.print("C   ");

    if (temperature > ALERT_TEMP) {
      digitalWrite(RELAY_PIN, HIGH);
      lcd.setCursor(0, 1);
      lcd.print("Cooling ON ");
      if (wifiConnected && Blynk.connected()) {
        Blynk.logEvent("temp_alert", "Warning! Temp > 35°C");
      }
    } else {
      digitalWrite(RELAY_PIN, LOW);
      lcd.setCursor(0, 1);
      lcd.print("Cooling OFF");
    }

    if (wifiConnected && Blynk.connected()) {
      Blynk.virtualWrite(V5, temperature);
      Blynk.virtualWrite(V6, humidity);
    }
  }
}
