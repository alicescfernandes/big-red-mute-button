#include <NimBLEDevice.h>
#include <Adafruit_NeoPixel.h>

#include "secrets.h"
#include "constants.h"

bool device_connected = false;
bool prev_reading;
bool reading;
unsigned long last_debounce = 0;     // the last time the output pin was toggled
unsigned long debounce_delay = 150;  // the debounce time; increase if the output flickers
bool button_state = false;
bool write_to_characteristic = false;


unsigned long previous_millis = 0;
const long interval = 1000;

int blinkCyclesDone = 0;       // number of completed on-off blinks
bool blinkLedOn = false;       // current state within a cycle
unsigned long blinkLastChange = 0;
const unsigned long blinkInterval = 150; // ms per half-cycle (on or off)


class ServerCallbacks : public NimBLEServerCallbacks {
public:
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
    device_connected = true;

    Serial.println("connected");
    Serial.print("Client address: ");
    Serial.println(connInfo.getAddress().toString().c_str());
  }

  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
    device_connected = false;
    
    Serial.println("disconnected");

    // Restart advertising so the device can connect again
    NimBLEDevice::startAdvertising();
  }
};

int read_value(NimBLECharacteristic *c) {
  time_t timestamp;
  NimBLEAttValue stuff = btn_characteristic->getValue();  // timestamp optional
  const uint8_t *value = stuff.getValue(&timestamp);
  return (int)value[0];
}

class BoxWriteCallbacks : public NimBLECharacteristicCallbacks {
public:
  void onWrite(
    NimBLECharacteristic* pCharacteristic,
    NimBLEConnInfo& connInfo
  ) {
    Serial.println("someone wrote here");

    NimBLEAttValue value = pCharacteristic->getValue();

    if (value.size() > 0) {
      button_state = value[0] != 0;

      if(button_state){
        showAll();
      }else{
        hideAll();
      }
    }
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

void on_button_click() {
  unsigned long current_time = millis();
  if (current_time - last_debounce > debounce_delay) {
    button_state = !button_state;
    write_to_characteristic = true;  // writing a characterstic inside of interrupt breaks XIAO
    last_debounce = current_time;
  }
}


// Call this once in setup() after pixels.begin()
void breatheInit() {
  pixels.begin();
  pixels.setBrightness(0);
  pixels.show();
}

bool breatheStep() {
  static int brightness = 0;
  static int step = 1;
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 10; // ms between steps (tune speed here)

  unsigned long now = millis();
  if (now - lastUpdate >= interval) {
    lastUpdate = now;

    brightness += step;

    if (brightness >= 255) {
      brightness = 255;
      step = -1;
    } else if (brightness <= 0) {
      brightness = 0;
      step = 1;
    }

    pixels.setBrightness(brightness);
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(255, 255, 255));
    }
    pixels.show();
  }

  return true;
}


// Call this every loop() until it returns false (animation done)
// Call every loop().
// To start: blinkCyclesDone = 0; blinkLedOn = false;
// Returns true while blinking, false when idle/done.

void showAll(){
  pixels.setBrightness(255);
  for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(255, 255, 255));
    }    
  pixels.show();
}

void hideAll(){
  pixels.setBrightness(0);
  pixels.clear();
  pixels.show();
}

bool blinkTwice() {
  if (blinkCyclesDone >= 2) {
      return false; // done
    }
    
  unsigned long now = millis();
  if (now - blinkLastChange >= blinkInterval) {
    blinkLastChange = now;

    blinkLedOn = !blinkLedOn;

    if (blinkLedOn) {
      showAll();
    } else {
      hideAll();
      blinkCyclesDone++;
    }
  }

  return true; // still blinking
}

void setup() {
  Serial.begin(11520);
  NimBLEDevice::init(NAME);

  pinMode(RED_LED, OUTPUT);

  pinMode(RED_BTN, INPUT_PULLUP);
  // NimBLEDevice::deleteAllBonds();  // Dev Only

  NimBLEDevice::setSecurityAuth(true, true, true);
  NimBLEDevice::setSecurityPasskey(PASSKEY);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

  ble_server = NimBLEDevice::createServer();
  ble_server->setCallbacks(new ServerCallbacks());

  create_hid_service();
  create_dis_service();
  create_box_service();

  NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID("1812");                 // HID Service
  advertising->addServiceUUID(BUTTON_SERVICE_UUID);    // your custom service
  advertising->setName(NAME);                          // device name

  // Optional: set scan response data instead of setScanResponse(true)
  NimBLEAdvertisementData scanData;
  scanData.setName(NAME);
  advertising->setScanResponseData(scanData);

  // Optional: set preferred connection intervals (if your core supports it)
  // advertising->setMinInterval(0x06);  // 7.5 ms
  // advertising->setMaxInterval(0x12);  // 22.5 ms

  advertising->start();

  attachInterrupt(digitalPinToInterrupt(RED_BTN), on_button_click, FALLING);

  breatheInit();
}

void loop() {
  
  /* #region Bluetooth Button */
  unsigned long current_millis = millis();
  if (write_to_characteristic) {
    Serial.println("wrote to char");
    write_to_characteristic = false;
    Serial.println(button_state ? HIGH : LOW);
    digitalWrite(RED_LED, button_state ? HIGH : LOW);
    write_value(btn_characteristic, button_state ? 1 : 0);
  }
  /* #endregion */

  /* #region Button Led*/
  if (!device_connected) {
    breatheStep();
  }

  // Clear the Neopixel if the device is connected
  if(device_connected){
    Serial.println("blinkCyclesDone:" + String(blinkCyclesDone));
   blinkTwice();
  }
  /* #endregion */
}