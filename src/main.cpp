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
      if (timerL1 != 0 && timerL1 + onDurationL1 <= millis()) {
        Serial.println("disable L1");
        timerL1 = 0;
        digitalWrite(l1, LOW);
      }
      if (timerL2 != 0 && timerL2 + onDurationL2 <= millis()) {
        Serial.println("disable L2");
        timerL2 = 0;
        digitalWrite(l2, LOW);
      }

      if (timerR1 != 0 && timerR1 + onDurationR1 <= millis()) {
        Serial.println("disable R1");
        timerR1 = 0;
        digitalWrite(r1, LOW);
      }
      if (timerR2 != 0 && timerR2 + onDurationR2 <= millis()) {
        Serial.println("disable R2");
        timerR2 = 0;
        digitalWrite(r2, LOW);
      }
    }

    void enableMotor(byte payload) { // payload: MSB->LSB x L(1)/R(0) x x Duration(H) Duration(L) Actuator(H) Actuator(L)
      int onDuration = (((payload >> 2) & 0x03) + 1) * 250;
      bool left = (payload >> 6) & 0x01;
      byte actuator = payload & 0x03;
      Serial.print("duration: " + String(onDuration));

      if (left) {
        switch (actuator) {
          case 0:
            Serial.println(" L1");
            digitalWrite(l1, HIGH);
            timerL1 = millis();
            onDurationL1 = onDuration;
            break;
          case 1:
            Serial.println(" L2");
            digitalWrite(l2, HIGH);
            timerL2 = millis();
            onDurationL2 = onDuration;
            break;
        }
      } else {
        switch (actuator) {
          case 0:
            Serial.println(" R1");
            digitalWrite(r1, HIGH);
            timerR1 = millis();
            onDurationR1 = onDuration;
            break;
          case 1:
            Serial.println(" R2");
            digitalWrite(r2, HIGH);
            timerR2 = millis();
            onDurationR2 = onDuration;
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

    int onDurationL1 = 500; // ms
    int onDurationL2 = 500; // ms
    int onDurationR1 = 500; // ms
    int onDurationR2 = 500; // ms
};


MotorController *motorController = nullptr;

class MyCallbacks: public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    digitalWrite(2, !digitalRead(2));
    std::string value = pCharacteristic->getValue();
    motorController->enableMotor(value[0]);
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