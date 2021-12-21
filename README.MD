# 🔴 Big Red Mute Button
![](https://www.google-analytics.com/collect?v=1&t=event&tid=UA-100869248-2&cid=555&ec=github&ea=pageview&el=big-red-mute-button&ev=1)
## Mute that mic with Javascript and custom hardware!
[insert image here]

With the release of Chrome 80, a new web API was shipped with the browser. This API allows the browser to communicate via serial with other devices, however is behind a flag. This project makes use of that API to communicate with an Arduino Micro that opens a serial port over USB. Both are set to the 6900 baud rate, so that they can communicate with each other. 

This makes use Tampermonkey to inject a script that will allow you to connect to any device over serial, and when a connection is established, the javascript will hijack the mute button (of the DOM) and will read the status of the button, and will send a first command to the connected device. Then, with the MutationObserver API, whenever the button is clicked, it will read again the status of the button, and send over the command to the arduino. 

The arduino accepts (over serial) two string values: a '0' and a '1',when is 0 the light is turned off, and when is '1' the light is turned on. It's the browser that tells if the LED is on or off. When you press the "physical" button, the arduino will detect the press and send it over to the browser (by writing a '0' on the console), and the browser will check the current status of the mute, and toggle it.


### Setup - Tampermonkey
- Go to ``chrome://flags`` in your browser and enable the ``#enable-experimental-web-platform-features`` flag
- Restart the browser
- Install Tampermonkey extension
- Install the script of this project  (https://github.com/alicescfernandes/big-red-mute-button/raw/master/tampermonkey/mutebutton.user.js)
- Reload the browser again
- Plug in your arduino
- Upload the arduino code of this project

## Mute the mic globally (only compatible with OSX) (WIP)
With the help of some python code we can make the mic mute globally, aplication-wide. To do so, just follow the instructions below.

I'm still working on a GUI to make this a little bit easier to do, ad
### Setup - Python (only compatible with OSX)
- `cd python/`
- `pip3 install -r requirements.txt`
- Press the button and you should be muted.


### Attention Mac Users!
If your system is High Sierra or below you may need to install the CH34X drivers on your mac. [This page](https://github.com/adrianmihalko/ch340g-ch34g-ch34x-mac-os-x-driver) explain how you can install the driver


# Parts
- ~~A Relay~~ A IRFZ4N Transistor (i had a relay on the first draft, but it makes a clicking noise when switching, so i've replaced it with a 2N2222 transistor)
- Arduino (i'm using a chinese version of nano)
- [Big Dome Pushbutton](https://www.sparkfun.com/products/9181)
- two 10k Ohm resistor
- A 12V battery pack or
- A 9V Battery and a 9V-12v step-up boost converter ([i'm using this one](https://www.pololu.com/product/2117))

Some parts i've used here are somewhat expensive, so do replace them with cheaper ones. Just keep in mind that the arduino should have serial ports and also HID capabilities.

# TODO:
- Fritzing & Schematics
- Moving JS code into an extension
- Resistor values
# References
- [Web Serial API](https://web.dev/serial/)
- [Web Serial Codelab](https://codelabs.developers.google.com/codelabs/web-serial/#0)

