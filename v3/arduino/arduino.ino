#include <NimBLEDevice.h>
#include <Adafruit_NeoPixel.h>

#include "secrets.h"
#include "constants.h"

unsigned long last_debounce = 0;     // the last time the output pin was toggled
static unsigned long debounce_delay = 260;  // the debounce time; increase if the output flickers

bool animate_button_state = false;
bool device_connected_state = false;
bool button_state = false; // state triggered phisically via button
bool standby_state = false; // sleep state only activated via software
bool write_to_characteristic_state = false;

int blink_cycles_done = 0;  // number of completed on-off blinks
bool blink_led_state = false;  // current state within a cycle
unsigned long blink_last_changed = 0;



int read_value(NimBLECharacteristic *c) {
  time_t timestamp;
  NimBLEAttValue stuff = btn_characteristic->getValue();  // timestamp optional
  const uint8_t *value = stuff.getValue(&timestamp);
  return (int)value[0];
}


void write_btn_characterstic_value(int value) {
  btn_characteristic->setValue(byte(value));
  btn_characteristic->notify();
}

// Call every on loop with 1 of the 3 animations.
// Returns true while still animating, false once finished
// IN_OUT never finishes on its own
bool breatheStep(BreatheMode mode, unsigned long interval = 5) {
  static unsigned long lastUpdate = 0;
  static BreatheMode lastMode = BREATHE_IN;

  // Reset state whenever the mode changes (fresh start each time it's selected)
  if (mode != lastMode) {
    lastMode = mode;
    switch (mode) {
      case BREATHE_IN:
        brightness = 0;
        step = 1;
        break;
      case BREATHE_OUT:
        brightness = 255;
        step = -1;
        break;
      case BREATHE_IN_OUT:
        brightness = 0;
        step = 1;
        break;
    }
  }

  unsigned long now = millis();
  if (now - lastUpdate >= interval) {
    lastUpdate = now;

    brightness += step;

    // Reverse direction only in IN_OUT mode
    if (mode == BREATHE_IN_OUT) {
      if (brightness >= 255) {
        brightness = 255;
        step = -1;
      } else if (brightness <= 0) {
        brightness = 0;
        step = 1;
      }
    } else {
      brightness = constrain(brightness, 0, 255);
    }

    pixels.setBrightness(brightness);
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(255, 255, 255));
    }
    pixels.show();
  }

  if (mode == BREATHE_IN) return brightness != 255;
  if (mode == BREATHE_OUT) return brightness != 0;

  return true;
}

void resetState(){
  animate_button_state = false;
  blink_cycles_done = 0;
  blink_led_state = false;
  button_state = false;
  write_to_characteristic_state = false;
  standby_state = false;

  // resets the bluetooth state
  write_btn_characterstic_value(0);
}

class ServerCallbacks : public NimBLEServerCallbacks {
public:
  void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) {
    device_connected_state = true;

    Serial.println("connected");
    Serial.print("Client address: ");
    Serial.println(connInfo.getAddress().toString().c_str());
  }

  void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) {
    device_connected_state = false;

    Serial.println("disconnected");

    // Restart advertising so the device can connect again
    NimBLEDevice::startAdvertising();
  }
};


class BoxWriteCallbacks : public NimBLECharacteristicCallbacks {
public:
  void onWrite(
    NimBLECharacteristic *pCharacteristic,
    NimBLEConnInfo &connInfo) {
    NimBLEAttValue value = pCharacteristic->getValue();

    if (pCharacteristic->getUUID().equals(NimBLEUUID(BTN_CHARACTERISTIC)) && standby_state == false) {
      // handle button write
        if (value.size() > 0) {
          button_state = value[0] != 0;
          animate_button_state = true;
        }
    } 
    
    if (pCharacteristic->getUUID().equals(NimBLEUUID(STANDBY_CHARACTERISTIC))) {
      // handle standby write
      if (value.size() > 0) {
          standby_state = value[0] != 0;
        }
    }
  }
};



void create_hid_service() {
  NimBLEService *hidService = ble_server->createService("1812");  // HID service

  // HID Info
  const uint8_t hidInfo[] = { 0x01, 0x01, 0x00, 0x02 };
  hidService->createCharacteristic("2A4A", NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN)->setValue(hidInfo, sizeof(hidInfo));

  // HID Report Map
  hidService->createCharacteristic("2A4B", NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN)->setValue(hidReportDescriptor, sizeof(hidReportDescriptor));

  // HID Control Point
  const uint8_t ctrl[] = { 0x00 };
  hidService->createCharacteristic("2A4C", NIMBLE_PROPERTY::WRITE_NR)->setValue(ctrl, sizeof(ctrl));

  // Protocol Mode
  const uint8_t proto[] = { 0x01 };
  hidService->createCharacteristic("2A4E", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE_NR)->setValue(proto, sizeof(proto));

  // Input Report characteristic (required by macOS)
  NimBLECharacteristic *inputReport = hidService->createCharacteristic("2A4D", NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ_AUTHEN);
  const uint8_t dummyInput[] = { 0x00, 0x00, 0x00 };  // dummy mouse report
  inputReport->setValue(dummyInput, sizeof(dummyInput));
  inputReport->createDescriptor("2908", NIMBLE_PROPERTY::READ)->setValue(std::vector<uint8_t>{ 0x01, 0x01 });  // Report ID 1, input

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

  // Add the 0x2901 User Description descriptor
  NimBLEDescriptor *btn_description = btn_characteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ_AUTHEN, 100);
  btn_description->setValue("Button  status (pressed / not pressed)");

  standby_characteristic = service->createCharacteristic(STANDBY_CHARACTERISTIC, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::READ_AUTHEN | NIMBLE_PROPERTY::WRITE_AUTHEN | NIMBLE_PROPERTY::NOTIFY);
  standby_characteristic->setValue(byte(0));  // Clients can subscribe to this, and update as they wish

  // Add the 0x2901 User Description descriptor
  NimBLEDescriptor *standby_description = standby_characteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ_AUTHEN, 100);
  standby_description->setValue("Standby Mode");

  service->start();
}


void on_button_click() {
  unsigned long current_time = millis();
  if (current_time - last_debounce > debounce_delay && standby_state == false) {
    button_state = !button_state;
    write_to_characteristic_state = true;  // writing a characterstic inside of interrupt breaks XIAO
    last_debounce = current_time;
  }
}

void setupNeoPixel() {
  pixels.begin();
  pixels.setBrightness(0);
  pixels.show();
}

// Call this every loop() until it returns false (animation done)
// Call every loop().
// To start: blink_cycles_done = 0; blink_led_state = false;
// Returns true while blinking, false when idle/done.
void showAll() {
  pixels.setBrightness(100);
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 255, 255));
  }
  pixels.show();
}

void hideAll() {
  pixels.setBrightness(0);
  pixels.clear();
  pixels.show();
}

bool blinkTwice() {
  if (blink_cycles_done >= 2) {
    return false;  // done
  }

  unsigned long now = millis();
  if (now - blink_last_changed >= blinkInterval) {
    blink_last_changed = now;
    blink_led_state = !blink_led_state;

    if (blink_led_state) {
      showAll();
    } else {
      hideAll();
      blink_cycles_done++;
    }
  }

  return true;  // still blinking
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
  advertising->addServiceUUID("1812");               // HID Service
  advertising->addServiceUUID(BUTTON_SERVICE_UUID);  // your custom service
  advertising->setName(NAME);                        // device name

  // Optional: set scan response data instead of setScanResponse(true)
  NimBLEAdvertisementData scanData;
  scanData.setName(NAME);
  advertising->setScanResponseData(scanData);

  // Optional: set preferred connection intervals (if your core supports it)
  // advertising->setMinInterval(0x06);  // 7.5 ms
  // advertising->setMaxInterval(0x12);  // 22.5 ms

  advertising->start();

  attachInterrupt(digitalPinToInterrupt(RED_BTN), on_button_click, RISING);

  setupNeoPixel();
}



void loop() {

  /* #region Bluetooth Button */
  unsigned long current_millis = millis();
  if (device_connected_state) {
    if (write_to_characteristic_state) {
      write_to_characteristic_state = false;
      animate_button_state = true;
      Serial.println("wrote to char");
      Serial.println(button_state ? HIGH : LOW);
      if (button_state) {
        write_btn_characterstic_value(1);
      } else {
        write_btn_characterstic_value(0);
      }
    }

    if (animate_button_state) {
      animate_button_state = breatheStep(button_state ? BREATHE_IN : BREATHE_OUT, 1);
    }
  }
  /* #endregion */

  /* #region Button Led*/
  if (!device_connected_state) {
    breatheStep(BREATHE_IN_OUT);
    resetState();
  }

  // Clear the Neopixel if the device is connected
  if (device_connected_state) {
    Serial.println("blink_cycles_done:" + String(blink_cycles_done));
    blinkTwice();
  }
  /* #endregion */
}