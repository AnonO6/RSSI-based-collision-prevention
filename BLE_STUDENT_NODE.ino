
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include "ThingSpeak.h"
#include <BLEAdvertisedDevice.h>
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#include "WiFi.h"
#define trigPin 13
#define echoPin 12
#define buzzer 15
#define SOUND_SPEED 0.034
int deviceConnect = 0;
long duration;
int scanTime = 4;     // In seconds
BLEScan *pBLEScan;
int c1;  //COUNTER for collision
int c2;  //Keeps track of movement in-out
//int cbuffer=0; //This helps debug classroom in-out errors
int prev = 1; //To store previous position of student
int classdetect = -1;
WiFiClient  client;
unsigned long myChannelNumber = 1;                        //Thingspeak channel no.
const char * myWriteAPIKey = "5AXVK8IQ6OLNKMLQ";          //Thingspeak WRITE API KEY

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
      classdetect = -1;
     // deviceConnect = 0; //This keeps check on no. of devices
      int ble_rssi;
      if (advertisedDevice.getName() == "student1")
      {
      //  deviceConnect++;
        ble_rssi = advertisedDevice.getRSSI();
        Serial.println("RSSI: " + String(ble_rssi));
        Serial.println("Student1 detected beware of collision");

        if (ble_rssi >= -60)
        {
          digitalWrite(trigPin, LOW);
          delayMicroseconds(2);
          // Sets the trigPin on HIGH state for 10 microseconds
          digitalWrite(trigPin, HIGH);
          delayMicroseconds(10);
          digitalWrite(trigPin, LOW);
          duration = pulseIn(echoPin, HIGH);
          int dist = duration * SOUND_SPEED / 2;
          if (dist < 100)
          {
            c1++;
            Serial.println("COLLISION!!");
            for (int i = 0; i < 150; i++)
            {
              digitalWrite(buzzer, HIGH);
              delay(5);
              digitalWrite(buzzer, LOW);
              delay(5);
            }
            ThingSpeak.setField(1, c1);
            int x1 = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
            if (x1 == 200) {
              Serial.println("Channel update successful.");
            }
            else {
              Serial.println("Problem updating channel. HTTP error code " + String(x1));
            }
            for (int i = 0; i < 120; i++)
            {
              digitalWrite(buzzer, HIGH);
              delay(5);
              digitalWrite(buzzer, LOW);
              delay(5);
            }
            delay(500);
          }
        }
        //Otherwise do nothing
      }
      if (advertisedDevice.getName() == "BSW004")
      {
       // deviceConnect++;
        ble_rssi = advertisedDevice.getRSSI();
        Serial.println("RSSI: " + String(ble_rssi));
        Serial.println("CLASSROOM DETECTED");
        classdetect = 1;
        if (ble_rssi > -85 && prev == 1)
        {
          c2++;
          Serial.println("Classroom entered");
          ThingSpeak.setField(2, c2);
          ThingSpeak.setField(5, 1);
          int x2 = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
          if (x2 == 200) {
            Serial.println("Channel update successful.");
           // cbuffer=1;
          }
          else {
            Serial.println("Problem updating channel. HTTP error code " + String(x2));
          }

          for (int i = 0; i < 20; i++)
          {
            digitalWrite(buzzer, HIGH);
            delay(40);
            digitalWrite(buzzer, LOW);
            delay(40);
          }
          prev = 0;
          delay(500);
        }
        if (ble_rssi < -85 && prev == 0)
        {
          c2++;
          Serial.println("In playground");
          ThingSpeak.setField(2, 0);
          ThingSpeak.setField(5, 0);
          int x2 = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
          if (x2 == 200) {
            Serial.println("Channel update successful.");
          }
          else {
            Serial.println("Problem updating channel. HTTP error code " + String(x2));
          }
          prev = 1;
          delay(2000);
        }
      }
      /*
      if (classdetect != 1 && prev!=1)
      {
        c2++;
        Serial.println("In playground");
        ThingSpeak.setField(2, 0);
        ThingSpeak.setField(5, 0);
        int x2 = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
        if (x2 == 200) {
          Serial.println("Channel update successful.");
        }
        else {
          Serial.println("Problem updating channel. HTTP error code " + String(x2));
        }
        prev = 1;
        delay(2000);
      }
      */
    }
};


static void my_gap_event_handler(esp_gap_ble_cb_event_t  event, esp_ble_gap_cb_param_t* param) {
  ESP_LOGW(LOG_TAG, "custom gap event handler, event: %d", (uint8_t)event);
}

static void my_gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t* param) {
  ESP_LOGW(LOG_TAG, "custom gattc event handler, event: %d", (uint8_t)event);
}

static void my_gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gatts_cb_param_t* param) {
  ESP_LOGW(LOG_TAG, "custom gatts event handler, event: %d", (uint8_t)event);
}

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer, OUTPUT);
  c1 = 0;
  c2 = 0;
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  Serial.println("Starting BLE work!");
  WiFi.begin("NagendraC", "csec104dhua");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  BLEDevice::setCustomGapHandler(my_gap_event_handler);
  BLEDevice::setCustomGattsHandler(my_gatts_event_handler);
  BLEDevice::setCustomGattcHandler(my_gattc_event_handler);

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(10);
  pBLEScan->setWindow(9); // less or equal setInterval value

}

void loop() {
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);

  //Serial.println("No. of connected devices: " + String(deviceConnect));
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
  delay(1000);
}
