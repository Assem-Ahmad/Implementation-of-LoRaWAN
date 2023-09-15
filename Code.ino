/*
  LoRaWAN IoT Network Implementation Project
  Authors: Esmaeil Alkhazmi, Assem Aldarrat, Mohammed Abbas
  University of Benghazi - Faculty of Engineering - Electrical and Electronic Engineering Department

  This code is part of the Performance Analysis and Implementation of The LoRaWAN IoT Network project
  conducted at the University of Benghazi. It implements a LoRa communication system with sender and
  receiver functionality, displaying relevant information on an OLED screen.

  Note: The arduino-LoRa Library (https://github.com/sandeepmistry/arduino-LoRa) was modified for this project.
  In the "arduino-LoRa/src/LoRa.h" path, a private function "int getSpreadingFactor();" was moved to public classes.
  This modification allows us to access and utilize the spreading factor information for our performance analysis.

  Code last modified: [14-8-2023]

*/#include <SPI.h>
#include <LoRa.h>
#include "board_def.h"
#include <WiFi.h>
#include "WiFi.h"

#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

OLED_CLASS_OBJ display(OLED_ADDRESS, OLED_SDA, OLED_SCL);



#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"


//display size for better string placement
int width;
int height;

int _moisture,sensor_analog;
const int sensor_pin = 12;



 
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
 
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(500);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}
 
void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["Moisture"] = _moisture;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
 
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}

void setup()
{
  Serial.begin(115200);

  connectAWS();
  // dht.begin();

  while (!Serial);

  if (OLED_RST > 0) {
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, HIGH);
    delay(100);
    digitalWrite(OLED_RST, LOW);
    delay(100);
    digitalWrite(OLED_RST, HIGH);
  }

  display.init();
  width = display.getWidth() / 2;
  height = display.getHeight() / 2;
  display.flipScreenVertically();
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(width - 50, height, LORA_SENDER ? "LoRa++ Sender" : "LoRa++ Receiver");
  display.display();
  delay(2000);


  // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // if (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //     display.clear();
  //     Serial.println("WiFi Connect Fail");
  //     display.drawString(display.getWidth() / 2, display.getHeight() / 2, "WiFi Connect Fail");
  //     display.display();
  //     delay(2000);
  //     esp_restart();
  // }
  // Serial.print("Connected : ");
  // Serial.println(WiFi.SSID());
  // Serial.print("IP:");
  // Serial.println(WiFi.localIP().toString());
  // display.clear();
  // display.drawString(display.getWidth() / 2, display.getHeight() / 2, "IP:" + WiFi.localIP().toString());
  // display.display();
  // delay(2000);

  SPI.begin(CONFIG_CLK, CONFIG_MISO, CONFIG_MOSI, CONFIG_NSS);
  LoRa.setPins(CONFIG_NSS, CONFIG_RST, CONFIG_DIO0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  if (!LORA_SENDER) {
    display.clear();
    display.drawString(width - 50, height, "LoraRecv++ Ready");
    display.display();
  }
  LoRa.setSpreadingFactor(9);//select Spreading Factor 
}

int count = 0;
void loop()
{

#if LORA_SENDER

  count++;
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(width - 50, height, String(count) + ". Packet: " + String(count));
  display.display();
  LoRa.beginPacket();
  LoRa.print(count);
  LoRa.print(". Packet= ");
  LoRa.print(count);
  LoRa.endPacket();
  delay(5000);

#else

  sensor_analog = analogRead(sensor_pin);
  _moisture = ( 100 - ( (sensor_analog/4095.00) * 100 ) );
  Serial.print("Moisture = ");
  Serial.print(_moisture);  
  Serial.println("%"); 
  publishMessage();
  client.loop();
  delay(10000);
  
  
  if (LoRa.parsePacket()) {
    String recv = "";
    while (LoRa.available()) {
      recv += (char)LoRa.read();
    }
    count++;
    display.clear();
    display.drawString(width - 50, height + 8, recv);
    String info =  "RSSI: " + String(LoRa.packetRssi());
    String snr =  "SNR: " + String(LoRa.packetSnr());
    String offset =  "OFFSET: " + String(float(LoRa.packetFrequencyError() / 1000)) + "kHz";
    String sf =  "SF: " + String(LoRa.getSpreadingFactor());
    String BW =  "BW: " + String(LoRa.getSignalBandwidth() / 1000) + "kHz";
    
    display.drawString(width - 50, height - 16, info);
    display.drawString(width - 50, height - 24, snr);
    display.drawString(width - 50, height - 32, offset);
    display.drawString(width - 50, height - 0, sf);
    display.drawString(width - 50, height - 8, BW);

    display.display();

    Serial.println(String(count));
    Serial.println(info);
    Serial.println(snr);
    Serial.println(offset);
    Serial.println(sf);
    Serial.println(BW);

  }
#endif
}

