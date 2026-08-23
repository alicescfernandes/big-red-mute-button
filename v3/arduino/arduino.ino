#include <NimBLEDevice.h>

#include "secrets.h"

#define RED_LED D4
#define RED_BTN D9
#define STATUS_LED A0
#define BATTERY_PERCENTAGE A1

NimBLEServer *ble_server;
NimBLECharacteristic *btn_characteristic;
NimBLECharacteristic *bas_characteristic;

bool device_connected = false;
bool prev_reading;
bool reading;
unsigned long last_debounce = 0;     // the last time the output pin was toggled
unsigned long debounce_delay = 150;  // the debounce time; increase if the output flickers
bool button_state = false;
bool write_to_characteristic = false;
int previous_battery = 0;

const int ledPin = 13;

int led_state = LOW;
unsigned long previous_millis = 0;
const long interval = 1000;

const uint8_t hidReportDescriptor[] = {
    0x05, 0x01,  // Usage Page (Generic Desktop)
    0x09, 0x02,  // Usage (Mouse)
    0xA1, 0x01,  // Collection (Application)
    0x09, 0x01,  //   Usage (Pointer)
    0xA1, 0x00,  //   Collection (Physical)
    0x05, 0x09,  //     Usage Page (Buttons)
    0x19, 0x01,  //     Usage Minimum (1)
    0x29, 0x03,  //     Usage Maximum (3)
    0x15, 0x00,  //     Logical Minimum (0)
    0x25, 0x01,  //     Logical Maximum (1)
    0x95, 0x03,  //     Report Count (3)
    0x75, 0x01,  //     Report Size (1)
    0x81, 0x02,  //     Input (Data, Variable, Absolute)
    0x95, 0x01,  //     Report Count (1)
    0x75, 0x05,  //     Report Size (5)
    0x81, 0x01,  //     Input (Constant)
    0x05, 0x01,  //     Usage Page (Generic Desktop)
    0x09, 0x30,  //     Usage (X)
    0x09, 0x31,  //     Usage (Y)
    0x15, 0x81,  //     Logical Minimum (-127)
    0x25, 0x7F,  //     Logical Maximum (127)
    0x75, 0x08,  //     Report Size (8)
    0x95, 0x02,  //     Report Count (2)
    0x81, 0x06,  //     Input (Data, Variable, Relative)
    0xC0,        //   End Collection
    0xC0         // End Collection
};

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) {
    device_connected = true;
    digitalWrite(STATUS_LED, HIGH);
    Serial.println("connected");

    NimBLEAddress addr(desc->peer_ota_addr);
    Serial.println("Connected!");
    Serial.print("Client address: ");
    Serial.println(addr.toString().c_str());
    Serial.println("connected");
  };

  void onDisconnect(NimBLEServer *pServer) {
    device_connected = false;
    Serial.println("disconnected");
    digitalWrite(STATUS_LED, LOW);
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

void create_hid_service() {
  NimBLEService *hidService = ble_server->createService("1812");  // HID service

  // HID Info
  const uint8_t hidInfo[] = {0x01, 0x01, 0x00, 0x02};
  hidService->createCharacteristic("2A4A", NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN)->setValue(hidInfo, sizeof(hidInfo));

  // HID Report Map
  hidService->createCharacteristic("2A4B", NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN)->setValue(hidReportDescriptor, sizeof(hidReportDescriptor));

  // HID Control Point
  const uint8_t ctrl[] = {0x00};
  hidService->createCharacteristic("2A4C", NIMBLE_PROPERTY::WRITE_NR)->setValue(ctrl, sizeof(ctrl));

  // Protocol Mode
  const uint8_t proto[] = {0x01};
  hidService->createCharacteristic("2A4E", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE_NR)->setValue(proto, sizeof(proto));

  // Input Report characteristic (required by macOS)
  NimBLECharacteristic *inputReport = hidService->createCharacteristic("2A4D", NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ_AUTHEN);
  const uint8_t dummyInput[] = {0x00, 0x00, 0x00};  // dummy mouse report
  inputReport->setValue(dummyInput, sizeof(dummyInput));
  inputReport->createDescriptor("2908", NIMBLE_PROPERTY::READ)->setValue(std::vector<uint8_t>{0x01, 0x01});  // Report ID 1, input

  hidService->start();
}
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
  btn_characteristic = service->createCharacteristic(BTN_CHARACTERISTIC, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::READ_AUTHEN | NIMBLE_PROPERTY::WRITE_AUTHEN | NIMBLE_PROPERTY::NOTIFY);

  // btn_characteristic = service->createCharacteristic(BTN_CHARACTERISTIC, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  btn_characteristic->setValue(byte(0));  // Clients can subscribe to this, and update as they wish
  btn_characteristic->setCallbacks(new BoxWriteCallbacks());
  service->start();
}

void write_value(NimBLECharacteristic *c, byte value) {
  c->setValue(value);
  c->notify(true);
}

int round5(int n) { return (n / 5 + (n % 5 > 2)) * 5; }

int get_battery_percentage() {
  uint32_t Vbatt = 0;
  for (int i = 0; i < 16; i++) {
    Vbatt = Vbatt + analogReadMilliVolts(BATTERY_PERCENTAGE);  // ADC with correction
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
  Serial.begin(11520);
  NimBLEDevice::init(NAME);
  // NimBLEDevice::deleteAllBonds();  // Dev Only

  NimBLEDevice::setSecurityAuth(true, true, true);
  NimBLEDevice::setSecurityPasskey(PASSKEY);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

  ble_server = NimBLEDevice::createServer();
  ble_server->setCallbacks(new ServerCallbacks());

  create_hid_service();
  create_dis_service();
  create_bas_service();
  create_box_service();

  NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID("1812");
  advertising->addServiceUUID(BUTTON_SERVICE_UUID);
  advertising->setName(NAME);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);
  advertising->setMaxPreferred(0x12);
  advertising->start();

  pinMode(RED_LED, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);

  pinMode(BATTERY_PERCENTAGE, INPUT);
  pinMode(RED_BTN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(RED_BTN), on_button_click, FALLING);

  int battery = get_battery_percentage();
  previous_battery = battery;
  write_value(bas_characteristic, previous_battery);
}

void loop() {
  unsigned long current_millis = millis();
  if (write_to_characteristic) {
    Serial.println("wrote to char");
    write_to_characteristic = false;
    Serial.println(button_state ? HIGH : LOW);
    digitalWrite(RED_LED, button_state ? HIGH : LOW);
    write_value(btn_characteristic, button_state ? 1 : 0);
  }

  int battery = get_battery_percentage();
  if (battery != previous_battery) {
    previous_battery = battery;
    write_value(bas_characteristic, previous_battery);
  }

  if (!device_connected && current_millis - previous_millis >= interval) {
    previous_millis = current_millis;

    if (led_state == LOW) {
      led_state = HIGH;
    } else {
      led_state = LOW;
    }

    digitalWrite(STATUS_LED, led_state);
  }
}