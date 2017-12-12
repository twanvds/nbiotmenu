# nbiotmenu

A menu-driven Arduino app for Sodaq NB-IoT and Adafruit RGBLCD shields.

In its current being, the project is still rough, not only at the edges.

It assumes an Arduino project stacked with:

* A [16x2 Alphanumeric LCD display and 5 pushbuttons shield from Adafruit](https://www.adafruit.com/product/716)
* The [Sodaq NB-IoT shield](https://shop.sodaq.com/nl/nb-iot-shield-deluxe-single-band-5-of-band-28.html)
* The [Arduino M0 Pro](https://store.arduino.cc/arduino-m0-pro)

## Arduino Leonardo v.s. M0 Pro
Previously I used an [Arduino Leonardo](https://store.arduino.cc/arduino-leonardo-with-headers), but I found them too finicky to deem them reliable.  When a sketch becomes larger then 50% of the available flash and/or more than 70% of the dynamic memory is used, uploading the sketches becomes problematic. You need a trick to get the board accepting uploads again. 

When `avrdude` complains it can't find the board or port, you retry uploading during a reset of the board while the IDE is looking for a port associated with the board, when the port is "lost". This happens a lot, too much to my taste. After `avrdude` you need to reset the board yourself and even then it is not guaranteed the board boots with the appropriate code.

I don't think it was a good idea to integrate the USB into the microcontroller, not for the type of usage a typical Arduino board has and that's experimenting with these devices without the "fear" of "damaging" it.  

The application assumes T-Mobile as the NB-IoT provider.

# User manual

The menu is built as follows:

1. USB 
    1. Speed
       1. 115200
       2. 57600
       3. 9600
       4. 2400
    2. Test
       1. Recv
       2. Send
2. NBIoT
    1. Radio
    2. Recv
    3. Reboot
    4. NCONFIG?
    5. NEUSTATS?
    6. TEST

The menu is navigated by the 4 "cursor" buttons. The 5th button at the rightside of the "cursorgroup" is the SELECT button.
The menu item is followed by characters hinting which actions are allowed: left, up, down, right and a little black diamond indicating that hitting the select button is to activate the underlying function. A little _open_ diamond signals a disabled (or NYI) action. The display will flash red-white when you attempt an illegal action.

The convention is during a white background is:

* left - exit the (sub)menu
* right - enter a submenu
* up - previous item in the current menu
* down - next item in the current menu
* select - activate function.

The convention during a yellow background is, i.e. with the response buffer viewer active:

* left, right - scroll the display either left or right
* up, down - scroll the display over the response buffer up and down
* select - leave the viewer and return to the last menu item.

The application has two modal states (i.e. not allowing user input):

* green background - the application awaits/processes output from the NB-IoT module or the USB interface 
* blue background - the application is sending data to the NB-IoT module or USB interface

When the application has been receiving data, the next mode is the viewer mode. With the USB receive test as an exception, in this case the received characters are shown immediately. Most of the time you should be able to exit such a modal state by hitting on the select button.

## debug output

This version spills its lots of debug information to `Serial` with a default speed of 115200 baud.





















