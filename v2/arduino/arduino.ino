#include <NimBLEDevice.h>

#include "secrets.h"

#define RED_LED D2
#define RED_BTN D10

NimBLEServer *bleServer;
NimBLECharacteristic *btnCharacteristic;

bool deviceConnected = false;
bool prev_reading;
bool reading;
unsigned long last_debounce = 0;     // the last time the output pin was toggled
unsigned long debounce_delay = 150;  // the debounce time; increase if the output flickers
bool button_state = false;
bool write_to_characteristic = false;

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer) {
    deviceConnected = true;
    Serial.println("connected");
  };

  void onDisconnect(NimBLEServer *pServer) {
    deviceConnected = false;
    Serial.println("connected");
  }
};

// https://www.bluetooth.com/specifications/dis-1-2/
void create_dis_service() {
  // Create device info service (DIS)
  NimBLEService *service = bleServer->createService("180A");

  // Create Manufacturer Name String characteristic, read only
  NimBLECharacteristic *characteristic = service->createCharacteristic("2A29", NIMBLE_PROPERTY::READ);
  characteristic->setValue("Alice Fernandes");

  service->start();
}

// https://www.bluetooth.com/specifications/specs/battery-service/
void create_bas_service() {
  NimBLEService *service = bleServer->createService("180F");

  NimBLECharacteristic *characteristic = service->createCharacteristic("2A19", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  characteristic->setValue(100);  // Clients can subscribe to this, and update as they wish

  service->start();
}

void create_box_service() {
  NimBLEService *service = bleServer->createService(BUTTON_SERVICE_UUID);

  btnCharacteristic = service->createCharacteristic(BTN_CHAR, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN | NIMBLE_PROPERTY::NOTIFY);
  btnCharacteristic->setValue(0);  // Clients can subscribe to this, and update as they wish

  service->start();
}

void write_value(NimBLECharacteristic *c, byte value) {
  c->setValue(value);
  c->notify(true);
}

void readValue(NimBLECharacteristic *c) {
  time_t timestamp;
  NimBLEAttValue stuff = btnCharacteristic->getValue();  // timestamp optional
  const uint8_t *value = stuff.getValue(&timestamp);
  Serial.println(value[0]);
}

void setup() {
  Serial.begin(9600);
  NimBLEDevice::init("NimBLE");

  NimBLEDevice::setSecurityAuth(true, true, true);
  NimBLEDevice::setSecurityPasskey(PASSKEY);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

  bleServer = NimBLEDevice::createServer();
  bleServer->setCallbacks(new ServerCallbacks());

  create_dis_service();
  create_bas_service();
  create_box_service();

  NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID(BUTTON_SERVICE_UUID);
  advertising->start();

  pinMode(RED_BTN, INPUT_PULLUP);
  pinMode(RED_LED, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(RED_BTN), on_button_click, FALLING);
}

void on_button_click() {
  unsigned long current_time = millis();
  if (current_time - last_debounce > debounce_delay) {
    button_state = !button_state;
    write_to_characteristic = true;  // writing a characterstic inside of interrupt breaks XIAO
    last_debounce = current_time;
  }
}
void loop() {
  if (write_to_characteristic) {
    write_to_characteristic = false;
    Serial.println(button_state);
    Serial.println(button_state ? HIGH : LOW);
    digitalWrite(RED_LED, button_state ? HIGH : LOW);
    write_value(btnCharacteristic, button_state ? 1 : 0);
  }
}