
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

/* Use the following link to install OLED library:
  https://github.com/vlast3k/Arduino-libraries/tree/master/SSD1306 */

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define buzzer 15
int beaconInRange = -1;
int scanTime = 2;     // In seconds

BLEScan *pBLEScan;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
      if (advertisedDevice.getName() == "BSW004")
      {
        int ble_rssi = advertisedDevice.getRSSI();
        Serial.println("RSSI: " + String(ble_rssi));
        Serial.println(advertisedDevice.getAddress().toString().c_str());

        if (ble_rssi > -85 && ble_rssi < -60)
        {
          beaconInRange = 0;
        }
        else if (ble_rssi > -60 && ble_rssi < -40)
        {
          beaconInRange = 1;
        }
        else if (ble_rssi > -40)
        {
          beaconInRange = 2;
        }
      }
    }
};
void setup()
{
  Serial.begin(115200);

  Serial.println("Starting BLE work!");

  BLEDevice::init("student1");

  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setValue("student1");
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();

  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();

  Serial.println("Characteristic defined!");

  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(10);
  pBLEScan->setWindow(9); // less or equal setInterval value
  pinMode(buzzer, OUTPUT); // set pin to output mode

}
void loop()
{
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);

  Serial.println("Flag: " + String(beaconInRange));
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
  if (beaconInRange == 1)
  {
    for (int i = 0; i < 20; i++)
    {
      digitalWrite(buzzer, HIGH);
      delay(40);
      digitalWrite(buzzer, LOW);
      delay(40);
    }
  }
  beaconInRange = -1;

}
