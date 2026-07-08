#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>

// ================= WIFI =================

const char* ssid = "Gauravs iPhone";
const char* password = "g12345678t";

// ================= MQTT =================

const char* mqtt_server =
"969aecc363e84e80be51f1e55de3c709.s1.eu.hivemq.cloud";

const char* mqtt_user = "Gaurav";
const char* mqtt_password = "Vaiga@2007";

#include <WiFiClientSecure.h>

WiFiClientSecure espClient;
PubSubClient client(espClient);

// ================= LCD =================

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= BMP180 =================

Adafruit_BMP085 bmp;

// ================= DHT22 =================

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// ================= MQ SENSOR =================

#define MQ_PIN 34

// ================= RAIN SENSOR =================

#define RAIN_PIN 35

// =================================================
// MQTT RECONNECT
// =================================================

void reconnectMQTT()
{
  while (!client.connected())
  {
    Serial.print("Connecting MQTT...");

    if (client.connect("ESP32_WeatherStation", mqtt_user,mqtt_password))
    {
      Serial.println(" Connected!");
    }
    else
    {
      Serial.print(" Failed. State=");
      Serial.println(client.state());

      delay(2000);
    }
  }
}

// =================================================
// WIFI RECONNECT
// =================================================

void reconnectWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
    return;

  Serial.println("Reconnecting WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Reconnected!");
}

// =================================================
// SETUP
// =================================================

void setup()
{
  Serial.begin(115200);

  // LCD
  lcd.init();
  lcd.backlight();

  // Startup Screen
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SMART WEATHER");

  lcd.setCursor(0,1);
  lcd.print("STATION V1.0");

  delay(2000);

  lcd.clear();

  // DHT
  dht.begin();

  // BMP180
  if (!bmp.begin())
  {
    Serial.println("BMP180 not found!");
    while(1);
  }

  // WiFi
  Serial.println("Connecting WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");

  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // MQTT
  espClient.setInsecure();
  client.setServer(mqtt_server, 8883);

  reconnectMQTT();

  Serial.println("System Ready!");
}

// =================================================
// LOOP
// =================================================

void loop()
{
  reconnectWiFi();

  if (!client.connected())
  {
    reconnectMQTT();
  }

  client.loop();

  // ================= DHT22 =================

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
 String dhtStatus;

if (isnan(temperature) || isnan(humidity))
{
  dhtStatus = "FAULT";

  temperature = 0;
  humidity = 0;
}
else
{
  dhtStatus = "OK";
}

  // ================= BMP180 =================

  float pressure = bmp.readPressure() / 100.0;
  float altitude = bmp.readAltitude();

  // ================= MQ =================

  int gasValue = analogRead(MQ_PIN);
  String mqStatus;

  if(gasValue <= 5 || gasValue >= 4090)
  {
    mqStatus = "FAULT";
  }
  else
  {
    mqStatus = "OK";
  }

  // ================= RAIN =================

  int total = 0;

  for(int i=0;i<10;i++)
  {
    total += analogRead(RAIN_PIN);
    delay(10);
  }

  int rainValue = total / 10;
  String rainSensorStatus;

  if(rainValue <= 5)
  {
    rainSensorStatus = "FAULT";
  }
  else
  {
    rainSensorStatus = "OK";
  }

  // ================= AIR QUALITY =================

  String airStatus;

  if(gasValue < 1500)
    airStatus = "GOOD";
  else if(gasValue < 2500)
    airStatus = "MODERATE";
  else
    airStatus = "POOR";

  // ================= RAIN STATUS =================

  String rainStatus;

  if(rainValue > 3200)
    rainStatus = "NO RAIN";
  else if(rainValue > 2000)
    rainStatus = "LIGHT";
  else
    rainStatus = "HEAVY";

  // ================= PRESSURE STATUS =================

  String pressureStatus;

  if(pressure < 1000)
    pressureStatus = "LOW";
  else if(pressure > 1020)
    pressureStatus = "HIGH";
  else
    pressureStatus = "NORMAL";

  // ================= System status=================

  String systemStatus;
  if(
      dhtStatus == "OK" &&
      mqStatus == "OK" &&
      rainSensorStatus == "OK"
  )
  {
      systemStatus = "HEALTHY";
  }
  else
  {
      systemStatus = "CHECK SYSTEM";
  }

  // ================= ALERTS =================

  String alertMessage = "NORMAL";

  if(temperature > 35)
  {
      alertMessage = "HIGH TEMPERATURE";
  }
  else if(airStatus == "POOR")
  {
      alertMessage = "POOR AIR QUALITY";
  }
  else if(rainStatus == "HEAVY")
  {
      alertMessage = "HEAVY RAIN DETECTED";
  }
  else if(systemStatus != "HEALTHY")
  {
      alertMessage = "SYSTEM FAULT";
  }


  // ================= SERIAL MONITOR =================

  Serial.println();
  Serial.println("========== WEATHER DATA ==========");

  Serial.print("Temperature : ");
  Serial.println(temperature);

  Serial.print("Humidity    : ");
  Serial.println(humidity);

  Serial.print("Pressure    : ");
  Serial.println(pressure);

  Serial.print("Pressure Status : ");
  Serial.println(pressureStatus);

  Serial.print("Altitude    : ");
  Serial.println(altitude);

  Serial.print("Gas Value   : ");
  Serial.println(gasValue);

  Serial.print("Air Quality : ");
  Serial.println(airStatus);

  Serial.print("Rain Value  : ");
  Serial.println(rainValue);

  Serial.print("Rain Status : ");
  Serial.println(rainStatus);

  Serial.print("DHT Status : ");
  Serial.println(dhtStatus);

  Serial.print("MQ Status : ");
  Serial.println(mqStatus);

  Serial.print("Rain Sensor Status : ");
  Serial.println(rainSensorStatus);

  Serial.print("System Status : ");
  Serial.println(systemStatus);

  Serial.print("Alert : ");
  Serial.println(alertMessage);



  

  // ================= MQTT =================

  #define NODE_ID "MASTER"

  
  String tempStr = String(temperature,1);
  String humStr = String(humidity,1);
  String pressureStr = String(pressure,1);
  String altitudeStr = String(altitude,1);
  String gasStr = String(gasValue);
  String rainValueStr = String(rainValue);

  client.publish("weather/temperature", tempStr.c_str());
  client.publish("weather/humidity", humStr.c_str());
  client.publish("weather/pressure", pressureStr.c_str());
  client.publish("weather/pressurestatus",pressureStatus.c_str());
  client.publish("weather/altitude", altitudeStr.c_str());
  client.publish("weather/gas", gasStr.c_str());

  client.publish("weather/airquality", airStatus.c_str());

  client.publish("weather/rainvalue", rainValueStr.c_str());

  client.publish("weather/rainstatus", rainStatus.c_str());

  client.publish("weather/dhtstatus",dhtStatus.c_str());

  client.publish("weather/mqstatus",mqStatus.c_str());

  client.publish( "weather/rainsensorstatus",rainSensorStatus.c_str());

  client.publish("weather/systemstatus",systemStatus.c_str() );

  client.publish("weather/alert", alertMessage.c_str());

  

  Serial.println("MQTT Data Published");

  // ================= LCD SCREEN 1 =================

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.print(temperature,1);

  lcd.print(" H:");
  lcd.print((int)humidity);
  lcd.print("%");

  lcd.setCursor(0,1);
  lcd.print("Air:");
  lcd.print(airStatus);

  delay(2000);

  // ================= LCD SCREEN 2 =================

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("P:");
  lcd.print(pressure,1);
  lcd.print(" hPa");

  lcd.setCursor(0,1);
  lcd.print("PS:");
  lcd.print(pressureStatus);
  lcd.print("      ");   // clears leftovers

  delay(2000);

  // ================= LCD SCREEN 3 =================

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Rain:");
  lcd.print(rainStatus);

  lcd.setCursor(0,1);
  lcd.print("Val:");
  lcd.print(rainValue);

  delay(2000);


// ================= LCD SCREEN 4 =================

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Altitude:");

  lcd.setCursor(0,1);
  lcd.print((int)altitude);
  lcd.print(" m");

  delay(2000);

  // ================= LCD SCREEN 5 =================

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("ALERT:");

  lcd.setCursor(0,1);
  lcd.print(alertMessage);

  delay(2000);

}

