#include <NimBLEDevice.h>

#include "secrets.h"

#define RED_LED D2
#define RED_BTN D10

NimBLEServer *ble_server;
NimBLECharacteristic *btn_characteristic;
NimBLECharacteristic *bas_characteristic;

bool deviceConnected = false;
bool prev_reading;
bool reading;
unsigned long last_debounce = 0;     // the last time the output pin was toggled
unsigned long debounce_delay = 150;  // the debounce time; increase if the output flickers
bool button_state = false;
bool write_to_characteristic = false;
int previous_battery = 0;

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer) {
    deviceConnected = true;
    Serial.println("connected");
  };

  void onDisconnect(NimBLEServer *pServer) {
    deviceConnected = false;
    Serial.println("disconnected");
  }
};

int read_value(NimBLECharacteristic *c) {
  time_t timestamp;
  NimBLEAttValue stuff = btn_characteristic->getValue();  // timestamp optional
  const uint8_t *value = stuff.getValue(&timestamp);
  return (int)value[0];
}

class BoxWriteCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic) {
    Serial.println("someone wrote here");
    int value = read_value(pCharacteristic);

    button_state = value ? true : false;
    digitalWrite(RED_LED, button_state ? HIGH : LOW);
  }
};

// https://www.bluetooth.com/specifications/dis-1-2/
void create_dis_service() {
  // Create device info service (DIS)
  NimBLEService *service = ble_server->createService("180A");

  // Create Manufacturer Name String characteristic, read only
  NimBLECharacteristic *characteristic = service->createCharacteristic("2A29", NIMBLE_PROPERTY::READ);
  characteristic->setValue("Alice Fernandes");

  service->start();
}

// https://www.bluetooth.com/specifications/specs/battery-service/
void create_bas_service() {
  NimBLEService *service = ble_server->createService("180F");

  bas_characteristic = service->createCharacteristic("2A19", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  bas_characteristic->setValue(100);  // Clients can subscribe to this, and update as they wish

  service->start();
}

void create_box_service() {
  NimBLEService *service = ble_server->createService(BUTTON_SERVICE_UUID);
  btn_characteristic = service->createCharacteristic(BTN_CHAR, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::READ_AUTHEN | NIMBLE_PROPERTY::WRITE_AUTHEN | NIMBLE_PROPERTY::NOTIFY);

  // btn_characteristic = service->createCharacteristic(BTN_CHAR, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  btn_characteristic->setValue(byte(0));  // Clients can subscribe to this, and update as they wish
  btn_characteristic->setCallbacks(new BoxWriteCallbacks());
  service->start();
}

void write_value(NimBLECharacteristic *c, byte value) {
  c->setValue(value);
  c->notify(true);
}

int get_battery_percentage() {
  uint32_t Vbatt = 0;
  for (int i = 0; i < 16; i++) {
    Vbatt = Vbatt + analogReadMilliVolts(A0);  // ADC with correction
  }
  float Vbattf = 2 * Vbatt / 16 / 1000.0;  // attenuation ratio 1/2, mV --> V
  int vbat = Vbattf * 100;
  long val = map(vbat, 250, 390, 0, 100);
  return round5(val);
}

void on_button_click() {
  unsigned long current_time = millis();
  if (current_time - last_debounce > debounce_delay) {
    button_state = !button_state;
    write_to_characteristic = true;  // writing a characterstic inside of interrupt breaks XIAO
    last_debounce = current_time;
  }
}

void setup() {
  Serial.begin(9600);
  NimBLEDevice::init("Alice");

  NimBLEDevice::setSecurityAuth(true, true, true);
  NimBLEDevice::setSecurityPasskey(PASSKEY);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

  ble_server = NimBLEDevice::createServer();
  ble_server->setCallbacks(new ServerCallbacks());

  create_dis_service();
  create_bas_service();
  create_box_service();

  NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID(BUTTON_SERVICE_UUID);
  advertising->start();

  pinMode(RED_BTN, INPUT_PULLUP);
  pinMode(RED_LED, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(RED_BTN), on_button_click, FALLING);

  int battery = get_battery_percentage();
  previous_battery = battery;
  write_value(batteryService, previous_battery);
}

void loop() {
  Serial.println(PASSKEY);
  if (write_to_characteristic) {
    write_to_characteristic = false;
    Serial.println(button_state);
    Serial.println(button_state ? HIGH : LOW);
    digitalWrite(RED_LED, button_state ? HIGH : LOW);
    write_value(btn_characteristic, button_state ? 1 : 0);
  }

  int battery = get_battery_percentage();
  if (battery != previous_battery) {
    previous_battery = battery;
    write_value(bas_characteristic, previous_battery);
  }
}