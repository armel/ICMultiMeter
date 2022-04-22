# ICMultiMeter
![basic](https://img.shields.io/badge/M5Stack-BASIC-blue)
![grey](https://img.shields.io/badge/M5Stack-GREY-blue)
![fire](https://img.shields.io/badge/M5Stack-FIRE-orange)
![core2](https://img.shields.io/badge/M5Stack-CORE2-green)
![aws](https://img.shields.io/badge/M5Stack-AWS-orange)

![licence](https://img.shields.io/github/license/armel/ICMultiMeter)
![language](https://img.shields.io/github/languages/top/armel/ICMultiMeter)
![size](https://img.shields.io/github/repo-size/armel/ICMultiMeter)
![version](https://img.shields.io/github/v/release/armel/ICMultiMeter)
![activity](https://img.shields.io/github/commit-activity/y/armel/ICMultiMeter)

**An excellent [video](https://www.youtube.com/watch?v=SCPEO7Eiy1E&ab_channel=HAMRADIODUDE) was released by [@HamRadioDude](https://twitter.com/HamRadioDude) about the installation of the IC705SMeter project. It can help you ! This is the same approach here. 
For Germans, take a look at this other excellent [video](https://www.youtube.com/watch?v=DrcMoVNwG_8) released by [@ManuelLausmann](https://twitter.com/ManuelLausmann).
Many thanks to them 🙏🏻** 

The ICMultiMeter project allows you to display the equivalent of the Meter screen of the IC-705, IC-7300 and IC-9700, directly on the M5Stack screen. This allows you to dedicate the IC screen to the Waterfall while having all the measurements on the M5Stack screen.

![ICMultiMeter FM](https://github.com/armel/ICMultiMeter/blob/main/img/FM.png)
![ICMultiMeter SSB](https://github.com/armel/ICMultiMeter/blob/main/img/SSB.png)

# Technical architecture

## Quick overview

[M5Stack](https://m5stack.com/) is based on an ESP-32, dual core, which can be clocked up to 240 MHz. M5Stack has 16 MB of flash memory. Like all ESPs, Wi-Fi is of course integrated. The 2 inch IPS color display, based on the ILI9342C chipset, has a comfortable resolution of 320 x 240 pixels. It is very bright. The integrated battery is 110 mAh. It is possible to add an additional battery (700 or 800mAh) if needed.

In terms of size and weight, it is very compact: 54 x 54 x 18mm for 47.2g. Can be carried in the pocket without any problem ;)

## Detailed technical specs

Here are the detailed technical specs, for the curious:

| Resources |	Description |
| --------- | ------------ |
|ESP32| 240MHz dual core, 600 DMIPS, 520KB SRAM, Wi-Fi, dual mode Bluetooth
Flash| Memory	16MB|
|Power| Input	5V @ 500mA|
|Port|	TypeC x 1, GROVE(I2C+I/0+UART) x 1|
|Core|Bottom Port	PIN (G1，G2，G3，G16, G17, G18, G19, G21, G22, G23, G25, G26, G35, G36)|
|IPS Screen|	2 inch, 320x240 Colorful TFT LCD, ILI9342C, max brightness 853nit|
|Button|	Custom button x 3|
|Speaker|	1W-0928|
|Battery|	110mAh @ 3.7V|
|Antenna|	2.4G 3D Antenna|
|Operating Temperature|	32°F to 104°F ( 0°C to 40°C )|
|Net weight|	47.2g|
|Gross weight|	93g|
|Product Size|	54 x 54 x 18mm|
|Package Size	|95 x 65 x 25mm|
|Case Material|	Plastic ( PC )|

## In addition

About the QSJ, count around 45€. You then have a complete development platform, totally autonomous, programmable in C and C++, MicroPython and UIFlow, from Linux, Windows or MacOS, all in a compact and ultra ergonomic box.


# Installation

## Prepare the stack

The easiest way is to install [PlateformIO for VSCode](https://platformio.org/install/ide?install=vscode) on your PC (Linux or Windows) or on your Mac (Intel or M1). It is a cross-platform and multilanguage development environment that is powerful and pleasant to use.

Then, still on your PC or Mac, clone the ICMultiMeter project via the command :

`git clone https://github.com/armel/ICMultiMeter.git`

You can also download a [zip archive](https://github.com/armel/ICMultiMeter/releases) of the project, if you prefer, and unzip it.

As I said, an excellent [video](https://www.youtube.com/watch?v=SCPEO7Eiy1E&ab_channel=HAMRADIODUDE) was released by @HamRadioDude about the installation of the IC705SMeter project. It can help you ! This is the same approach here.

## Configuration

Open the ICMultiMeter project with PlateformIO for VSCode.

### File `src/settings.h`

#### IC Transceiver Model

Line 5, the constant `IC_MODEL` set your Transceiver model.
Choose between : 705, 7300 or 9700.

> I don't own the IC-9700, so I haven't tested it.

#### IC Connect method

If you're a using an IC-7300 or IC-9700, choose __USB__ !

Line 8, the constant `IC_CONNECT` set your connect method.
Choose between : BT or USB

> Note that BT only works with IC-705.

#### CI-V Address of your Transceiver

Line 11, the constant `CI_V_ADDRESS` set the CI-V Address of your Transceiver. I have indicated the default value. Refer to the documentation, if needed.

> Note that, if you read 94h as `CI_V_ADDRESS` on your Transceiver, you must indicate 0x94. If it's A4h, you must indicate 0xA4. And so on. 

#### Wifi Configuration 

If you're a using an IC-7300 or IC-9700, __it's necessary__ !

Line 14 and 15, the constants `WIFI_SSID` and `WIFI_PASSWORD` set your Wifi configuration.

In complement, you can view your ICMultiMeter from a simple browser. It is even possible to control it by this way, as the buttons are clickable. In order to display your ICMultiMeter in your browser, just go to `http://ip_address_of_your_ICMultiMeter/`. As a reminder, the IP address that your ICMultiMeter retrieves is sometimes displayed on the screen.

> Beware: it's slow! And there is no automatic refresh. You have to click on the background of the screen image to make a new capture. And otherwise, as said, the buttons are functional.

#### Proxy Configuration

If you're a using an IC-7300 or IC-9700, __it's necessary__ ! 

You will need to take a look at my other project, [ICUSBProxy](https://github.com/armel/ICUSBProxy). This project allows to set up an HTTP _proxy_ allowing to talk wirelessly, from an M5Stack (or other equipment), with an Icom transceiver like IC-7300 or IC-9700 and sending CI-V commands.

Please, take the time to read the [README.md](https://github.com/armel/ICUSBProxy/blob/main/README.md) and install [ICUSBProxy](https://github.com/armel/ICUSBProxy).

It's done ? Nice, so we can move forward.

Line 18 and 19, the constants `SERIAL_DEVICE` and `BAUDE_RATE` set the CI-V COM port settings. So COM port number (COM1, /dev/ttyUSB0, etc.) and Baude rate (115200, 19200, 9600, etc.).
Line 20 and 21, the constants `PROXY_URL` and `PROXY_PORT` set the URL and port of the Proxy.

> About Proxy, the idea is to use a PC or a nano computer (like a Raspberry Pi) to connect the IC-7300 or IC-9700 transceiver via the USB cable (USB type A to USB type B). The M5Stack will talk to this PC by Wifi and the PC will talk to the transceiver by the USB cable. By this way, the M5Stack keeps the big advantage of being wireless.

#### TFT image retention

To avoid TFT image retention (also called _Burn In_), there is a kind of screen saver. If there is no action on the buttons or PTT, after a delay of 60 minutes, the screen will show the Icom logo. 

You can change the constant `TIMEOUT_SCREENSAVER` to set the delay, line 24. 

If the screen saver is active and you press a button or the PTT to transmit, the ICMultiMeter screen will come back.

Note that if the connection between your M5Stack and the Transceiver has been established and you turn off your Transceiver (or disconnect the Bluetooth link, with IC-705), the screen will go completely into _sleep mode_. The screen will come back as soon as the connection is established again after you turn on your Transceiver. 

#### Examples of settings

First of all, here is a part of my transceiver settings, on IC-7300 :

```
Connectors > CI-V > CI-V USB Port       Unlink from [REMOTE]
Connectors > CI-V > CI-V USB Baud Rate  115200
Connectors > CI-V > CI-V USB Echo Back  OFF
```

Next, in `settings.h` ... 

##### IC-705, BT

| settings        | value           | 
| ------------- |:-------------:| 
| IC_MODEL      | 705 | 
| IC_CONNECT      | BT      | 
| CI_V_ADDRESS | 0xA4      |

##### IC-705, USB

| settings        | value           | 
| ------------- |:-------------:| 
| IC_MODEL      | 705 | 
| IC_CONNECT      | USB      | 
| CI_V_ADDRESS | 0xA4      |
| WIFI_SSID   |  _My WiFi SSID_ |
| WIFI_PASSWORD | _My WiFi Password_ |
| SERIAL_DEVICE | "/dev/ttyACM0" |
| BAUDE_RATE | 115200 | 
| PROXY_URL | "http://192.168.1.32" |
| PROXY_PORT | 1234 |    

##### IC-7300, USB

| settings        | value           | 
| ------------- |:-------------:| 
| IC_MODEL      | 7300 | 
| IC_CONNECT      | USB      | 
| CI_V_ADDRESS | 0x94      |
| WIFI_SSID   |  _My WiFi SSID_ |
| WIFI_PASSWORD | _My WiFi Password_ |
| SERIAL_DEVICE | "/dev/ttyUSB0" |
| BAUDE_RATE | 115200 | 
| PROXY_URL | "http://192.168.1.32" |
| PROXY_PORT | 1234 |    

## Compiling and flashing the M5Stack

Compile and upload the project to your M5Stack. You are done !

# Usage

Once launched, if you're using Bluetooth with an IC-705, you must connect your IC-705 to your M5Stack via the menu (Set / Bluetooth Set) of your transceiver. Refer to the documentation, if needed.

If you're using USB with [ICUSBProxy](https://github.com/armel/ICUSBProxy), plug your Transceiver via the USB cable (USB type A to USB type B) to your PC. It's time to start the Python3 script :

`./ICUSBProxy.py`

In addition :

- a long press on the left button allows to decrease the brightness, 
- a long press on the central button allows to switch off the M5Stack,
- a long press on the right button allows to increase the brightness.

> The value of the brightness is preserved at the next restart.

# Using the Bin Loader (power user only...)

It's possible to store several applications on the SPI Flash File Storage of your M5Stack or on SD Card Storage. At startup, a procedure is provided to load a particular application.

## Preparation

I will detail here the procedure to deploy ICSMeter and ICMultiMeter applications on the same M5Stack.

### Step 1 - Compile

Start by compiling your applications, as you used to do. Nothing changes here. For example, start by compiling the ICSMeter application. Then do the same with the ICMultiMeter application. 

### Step 2 - Collecting the binary files

That's it, you have compiled the ICSMeter and ICMultiMeter application? It's perfect.

Each compilation has produced a binary. It is this binary that is sent / flashed to your M5Stack, via the USB connection.

Go to the root of the ICSMeter folder, which contains the whole project. And go to the directory `.pio/build/m5stack`.

You will find a `firmware.bin` file there. Now, there are 2 solutions...

### Step 2.1 - SD Card Storage (simple)

Format an SD Card as FAT32.

Copy the `firmware.bin` at the root of the SD Card. And take the opportunity to rename it, for example, `ICSMeter.bin`.

Do the same with the ICMultiMeter application. Of course rename it with a different name, for example, `ICMultiMeter.bin`.

At this point, you should have 2 clearly identified binaries in the root of your SD Card : `ICSMeter.bin` and `ICMultiMeter.bin`.

### Step 2.2 - SPI Flash File Storage (more difficult)

Copy `firmware.bin` in the `data` directory at the root of the ICSMeter folder. And take the opportunity to rename it, for example, `ICSMeter.bin`.

> If the `data` folder does not exist, create it.

Do the same with the ICMultiMeter application. Go to the root of the ICMultiMeter folder, which contains the whole project. And go to the directory `.pio/build/m5stack`.

You will also find a `firmware.bin` file. Copy it, too, in the `data` directory at the __root of the ICSMeter folder__. And take the opportunity to rename it to, for example, `ICMultiMeter.bin`.

> Important, the idea is to copy these 2 binaries in the same directory `data`** (located at the root of the ICSMeter folder).

At this point, you should have 2 clearly identified binaries: `ICSMeter.bin` and `ICMultiMeter.bin` in the `data` directory at the root of the ICSMeter folder.

So let's move on to what is probably the most complicated step. Open the ICSMeter project from Visual Studio Code, as you would compile it. 

![Capture](https://github.com/armel/RRFRemote/blob/main/img/flash_1.png)

Step 1, click on the Platformio icon (the icon with an ant's head...). Step 2, unroll the section `m5stack`.

![Capture](https://github.com/armel/RRFRemote/blob/main/img/flash_2.png)

Finally, step 3, go to the `Platform` sub-section. And click on `Upload Filesystem Image`.

Wait ! It's coffee time (or beer) :) The contents of the `data` directory will be written to the SPI Flash File Storage of your M5Stack. Are you done? You're there !!!!

## Usage

Start your M5Stack. You should see a black screen, followed by 1, 2 and 3 small dots at the top of the screen. This is the famous Bin Loader ;)

As soon as the first little dot appears, you can :

- either press the left or right button, to launch the default application.
- or press the central button. In this case, the Bin Loader menu appears and offers you the list of available binaries in SPI Flash File Storage or SD Card. 

If you have followed the procedure perfectly, you should have a choice between `ICSMeter.bin` and `ICMultiMeter.bin`.

The left and right buttons allow you to switch from one binary to another. And the middle button validates the selected binary. In this case, the selected application will be loaded ;)

> The loading takes about 20 seconds...

## Limitation

I think it is possible to have 3 or 4 applications cohabit in SPI Flash File Storage. If need more, use SD Card Storage.

# Credits
 
Many thanks to...

| Project             | Author                                                |  Link                                        |
|:------------------- | :---------------------------------------------------- | :------------------------------------------- |
| M5Stack             | [M5Stack](https://twitter.com/M5Stack)                | https://github.com/m5stack/M5Stack           |
| M5Stack-SD-Updater  | [Tobozo](https://twitter.com/TobozoTagada)            | https://github.com/tobozo/M5Stack-SD-Updater |
| FastLED             | FastLED                                               | https://github.com/FastLED/FastLED           |


# Donations

Special thanks to Rolf Schroeder, Brian Garber, Matt B-Wilkinson, Robert Agnew, Meinhard Frank Günther, Johan Hansson, Tadeusz Pater, Frederic Ulmer, Joshua Murray, Mark Hammond and Angel Mateu Muzzio for their donations. That’s so kind of them. Thanks so much 🙏🏻

If you find this project fun and useful then [offer me a beer](https://www.paypal.me/F4HWN) :) 

By the way, you can follow me on [Twitter](https://twitter.com/F4HWN) and post pictures of your installation with your M5Stack. It always makes me happy ;) 