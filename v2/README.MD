# 🔴 Big Red ~~Mute~~ Button

![](https://www.google-analytics.com/collect?v=1&t=event&tid=UA-100869248-2&cid=555&ec=github&ea=pageview&el=big-red-mute-button&ev=1)

## Mute that mic with Javascript or lock that computer with custom hardware!

[insert image here]

This is a Bluetooth-controlled device that emulates a mouse, allowing it to appear as a standard input device to Windows and macOS. This enables it to be paired with the OS without installing any drivers or additional software.

The device uses BLE (Bluetooth Low Energy) to communicate wirelessly and emulates a Human Interface Device (HID). This makes it possible to trigger system-level actions like muting the microphone or locking the screen by simulating mouse movement or clicks.

A large physical button sends input events wirelessly. These can be picked up by the host OS as normal mouse activity or used with automation tools to trigger specific actions.

## Mouse Emulation

This device uses Bluetooth Low Energy (BLE) to emulate a mouse via the HID (Human Interface Device) profile. By creating a fake mouse, it appears to the host (Windows or macOS) as a standard mouse. And with BLE, we can create our own functionalities (custom characteristics) and attach it to the fake mouse device, and have some python code to write on those

## BLE Services and Characteristics

In Bluetooth Low Energy (BLE), communication is structured around services and characteristics. A service is a collection of related functionality—like a HID service for mouse input or a battery service for reporting battery level. Each service contains one or more characteristics, which represent individual pieces of data that can be read, written, or notified.

For example, this device implements:

- A HID service, which includes characteristics to send mouse reports (movement or button presses).
- A Battery service, with a battery level characteristic which exposes the current battery level to the connected device.
- A Custom Button service, with a single characteristic which notifies the host when the physical button is pressed.

## Mute the mic globally (only compatible with OSX & Windows) (WIP)

With the help of some python code we can make the mic mute globally, aplication-wide. To do so, just follow the instructions below.

I'm still working on a GUI to make this a little bit easier to do, and is still a little big hardcoded

### Setup - TBD

# Parts

- Xiao ESP32C3
- [Big Dome Pushbutton](https://www.sparkfun.com/products/9181)
- Buck Converter
- Lithium Battery

# TODO:

- GUI for python
