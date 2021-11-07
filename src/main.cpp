#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MotorController {
  public:
    MotorController() {
      pinMode(r1, OUTPUT);
      pinMode(r2, OUTPUT);
      pinMode(l1, OUTPUT);
      pinMode(l2, OUTPUT);
    };

    void checkLoop() {
      if (timerL1 != 0 && timerL1 + onDuration <= millis()) {
        Serial.println("disable L1");
        timerL1 = 0;
        digitalWrite(l1, LOW);
      }
      if (timerL2 != 0 && timerL2 + onDuration <= millis()) {
        Serial.println("disable L2");
        timerL2 = 0;
        digitalWrite(l2, LOW);
      }

      if (timerR1 != 0 && timerR1 + onDuration <= millis()) {
        Serial.println("disable R1");
        timerR1 = 0;
        digitalWrite(r1, LOW);
      }
      if (timerR2 != 0 && timerR2 + onDuration <= millis()) {
        Serial.println("disable R2");
        timerR2 = 0;
        digitalWrite(r2, LOW);
      }
    }

    void enableMotor(byte payload) { // payload: MSB->LSB x L(1)/R(0) x x Strengh(H) Strength(L) Actuator(H) Actuator(L)
      byte intensity = payload >> 2 & 0x03;
      bool left = (payload >> 6) & 0x01;
      byte actuator = payload & 0x03;
      Serial.println("left:" + String(left));
      Serial.println("act: " + String(actuator));

      if (left) {
        switch (actuator) {
          case 0:
            Serial.println("enable L1");
            digitalWrite(l1, HIGH);
            timerL1 = millis();
            break;
          case 1:
            Serial.println("enable L2");
            digitalWrite(l2, HIGH);
            timerL2 = millis();
            break;
        }
      } else {
        switch (actuator) {
          case 0:
            Serial.println("enable R1");
            digitalWrite(r1, HIGH);
            timerR1 = millis();
            break;
          case 1:
            Serial.println("enable R2");
            digitalWrite(r2, HIGH);
            timerR2 = millis();
            break;
        }
      }
    }

  private:
    const int r1 = 12;
    const int r2 = 14;
    const int l1 = 27;
    const int l2 = 26;

    int timerR1 = 0;
    int timerR2 = 0;
    int timerL1 = 0;
    int timerL2 = 0;

    const int onDuration = 500; // ms
};


MotorController *motorController = nullptr;

class MyCallbacks: public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    digitalWrite(2, !digitalRead(2));
    std::string value = pCharacteristic->getValue();
    motorController->enableMotor(value[0]);
    if (value.length() > 0)
    {
      Serial.println("*********");
      Serial.print("New value: ");
      for (int i = 0; i < value.length(); i++)
      {
        Serial.print(value[i]);
      }

      Serial.println();
      Serial.println("*********");
    }
  }

  void onRead(BLECharacteristic *characteristics) {
    digitalWrite(2, !digitalRead(2));
  }
};

void setup()
{
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2, 1);

  motorController = new MotorController();

  BLEDevice::init("ESP32-BLE-Server");
  BLEServer *pServer = BLEDevice::createServer();

  MyCallbacks *bleCallbacks = new MyCallbacks();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE_NR
                                       );

  pCharacteristic->setCallbacks(bleCallbacks);
  pCharacteristic->setWriteProperty(true);

  pCharacteristic->setValue("Hello World");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop()
{
  delay(20);
  motorController->checkLoop();
}