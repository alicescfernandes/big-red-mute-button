# 🔴 Big Red Button (BRB for short)

## A wireless bluetooth button that does anything

This is a Bluetooth-controlled device that emulates a Bluetooth mouse, allowing it to appear as a standard input device to Windows and macOS. This enables it to be paired with the OS without installing any drivers or additional software. This is the V3 version of the button, where i experimented a lot with parts and printed circuit boards!

The device uses BLE (Bluetooth Low Energy) to communicate wirelessly via Bluetooth characteristics (think of it like wireless variables) that can be changed by a device that is authenticated with the device.

## WHy & how did i build this?

What led me to redo the BRB (🔴) _again_ was the desire to make the perfect button, and that meant that v1 and v2 were not perfect. For one, the v2 project kept restarting on battery, and i just had way to many wires. The button itself was big and noisy, and used a 12V LED that i could barely see and needed a boost converter to take 5V into 12V.

For this revision, i wanted all on 5V, which meant rebuilding the button itself. Since i had a few spare keyboard Cherry MX switches and a Neopixel that i ended up storing away, i've settled on using those parts for this revision. For the dome part, i decided to take apart the red button of V1, and repurposed the battery of V2.

I experimented _a lot_ on this revision. Went with magnetic connectors for the bottom and top part, custom made PCBs (i designed them myself, and even milled one on a CNC device), a big (i mean _huge_) keycap for the Cherry MX with 12 powerful 5V LEDS! The "net new" parts ended up being a IP5603 module to manage the charging of the battery and a few accessory parts for the enclosure.

### Why not TP4056?

I researched a lot about battery management IC's, and ended up with 3 options: TP4056, IP5603  and the BQ25895. They all have advantages:
- TP4046 is the simplest of them all, but doesn't transform the output of the battery to 5V, and i really need those 5V. You can definitely add a boost circuit for the output, but i didn't want to bother with that
- IP5603 does transform the power to 5V, but it only provides visual indicators for battery percentage. It's also relatively simple to build a circuit around this one
- BQ25895 can provide a 5V power and battery percentage information over I2C protocol, which is awesome, but the reference circuit is a bit too scary for me to try, and it's a bit outta my league.

The good news is that, since i've incorporated IP5306 into my own circuit, i can just change the battery management into any other of those options without rebuilding the whole schematic! I'll try this later just for fun, and because i wanna try incorporating BQ2585 because of those I2C features!

### Software used
- EasyEDA for schematic and PCB Design
- Flatcam for board milling
- Arduino IDE to code the ESP32's

## Inner workings

### Mouse Emulation

This device uses Bluetooth Low Energy (BLE) to emulate a mouse via the HID (Human Interface Device) profile. By creating a fake mouse, it appears to the host (Windows or macOS) as a standard mouse. And with BLE, we can create our own functionalities (custom characteristics) and attach it to the fake mouse device, and have some python code to write on those.

The fake mouse is needed for Windows and MacOS, that refuse to accept input of devices that are don't provide HID functionality, but i assure you that the mouse is controlled by the device, it just emulates the device characteristics of a mouse.

### BLE Services and Characteristics

In Bluetooth Low Energy (BLE), communication is structured around services and characteristics. A service is a collection of related functionality, like a HID service for mouse input or a battery service for reporting battery level. Each service contains one or more characteristics, which represent individual pieces of data that can be read, written, or notified.

For example, this device implements:

- A HID service, which includes characteristics to send mouse reports (movement or button presses).
- A Battery service, with a battery level characteristic which exposes the current battery level to the connected device.
- A Custom Button service, with a single characteristic which notifies the host when the physical button is pressed.

### You said it could do anything?

Yes, with the help of some python code we can make the mic mute globally, trigger a lock screen or just rick-roll somebody. The host application was built using Rumps and Python to build a menu bar GUI. I'm not sure if this is compatible with Linux and Windows but at least works on Mac (:joy:)



## Assembling one yourself

### Bill of Materials (if you wanna build it yourself)

- [Waveshare ESP32-Tiny](https://www.waveshare.com/esp32-s3-tiny.htm) or a [Seed XIAO ESP32-S3](https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html)
- [Big Dome Pushbutton](https://www.sparkfun.com/products/9181), you will need just the "dome" part
- IP5306 lithium battery module
- USB power input via:
    - A USB Panel mount to 2 wires (something like this https://www.adafruit.com/product/6072, that has a red and a black wire)
    - A USB-C 5V PD decoy chip (pay attention to the 5V, it needs to be 5V)
- A Lithium Battery (mind the Amps)
- A 12 RGB LED Neopixel (https://www.adafruit.com/product/1643)
- A Cherry MX Switch (any type)
- And a Neokey breakout board (https://www.adafruit.com/product/4978, or you can just use a cherry mx like a simple button)

## Easy circuit to follow 
Assemble your parts like the image below

>[!IMPORTANT]
> The ground (the black wire) is a common ground. This means that all grounds need to be connected between each other

![alt text](image-1.png)

Try to solder into a protoboard for less wires. The enclosure here won't work so you will have to adjust the enclosure to match the footprint