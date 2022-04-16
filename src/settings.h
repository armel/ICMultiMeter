// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// M5Stack Board
#define BOARD BASIC                             // Your M5Stack Board

// IC Transceiver Model
#define IC_MODEL 705                            // 705, 7300 (and may be 9700)

// IC Connect
#define IC_CONNECT  BT                          // BT (Bluetooth) or USB (Universal Serial Bus)

// CI_V Address 
#define CI_V_ADDRESS 0xA4                       // CI-V default address 0xA4

// Wifi Config
#define WIFI_SSID       "YOUR WIFI SSID"        // Your WiFi SSID
#define WIFI_PASSWORD   "YOUR WIFI PASSWORD"    // Your WiFi Password

// Proxy Config
#define SERIAL_DEVICE "/dev/ttyUSB0"            // Proxy Serial Device
#define BAUDE_RATE 115900                       // Proxy Serial Device Baude Rate
#define PROXY_URL "http://192.168.1.32"         // Proxy HTTP URL
#define PROXY_PORT 1234                         // Proxy HTTP Port

// Screen Saver Time Out
#define TIMEOUT_SCREENSAVER 60 * 60 * 1000      // 60 min default timeout (in milli seconds)