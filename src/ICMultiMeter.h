// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define VERSION "1.0.1"
#define AUTHOR "F4HWN"
#define NAME "ICMultiMeter"

#define DEBUG 0
#define ATOM atom
#define WIDTH displayWidth
#define HEIGHT displayHeight

#define TIMEOUT_BIN_LOADER  3                 // 3 sec
#define STEP 2

#define BT 1
#define USB 2

#define M5ATOMDISPLAY_LOGICAL_WIDTH  WIDTH    // width
#define M5ATOMDISPLAY_LOGICAL_HEIGHT  HEIGHT  // height
#define M5ATOMDISPLAY_REFRESH_RATE 60         // refresh rate

#include <Preferences.h>
#include <FastLED.h>
#include <HTTPClient.h>
#include <SD.h>
#include <FS.h>
#include <SPIFFS.h>

#if ATOM == 1
  #include <M5AtomDisplay.h>
#endif

#include <M5Unified.h>
#include <BluetoothSerial.h>
#include <M5StackUpdater.h>

// Color
#define TFT_MODE_BORDER display.color565(115, 135, 159)
#define TFT_MODE_BACK display.color565(24, 57, 92)

#define TFT_FIL_BORDER display.color565(99, 99, 99)
#define TFT_FIL_BACK display.color565(66, 66, 66)

#define TFT_GAUGE_1 display.color565(215, 96, 32)
#define TFT_GAUGE_2 display.color565(255, 128, 32)
#define TFT_GAUGE_3 display.color565(200, 64, 16)

#define TFT_MENU_BORDER display.color565(115, 135, 159)
#define TFT_MENU_BACK display.color565(24, 57, 92)
#define TFT_MENU_SELECT display.color565(255, 255, 255)

// Web site Screen Capture stuff
#define GET_unknown 0
#define GET_index_page  1
#define GET_screenshot  2

// Display
uint16_t offsetX = 0;
uint16_t offsetY = 0;

#if ATOM == 0
  M5GFX &display(M5.Lcd);
#else
  M5AtomDisplay display(WIDTH, HEIGHT);
#endif

// Sprite
LGFX_Sprite gaugeSprite(&display);
LGFX_Sprite levelSprite(&display);
LGFX_Sprite logoSprite(&display);

// LED
#define NUM_LEDS 10
CRGB leds[NUM_LEDS];

// Bluetooth connector
BluetoothSerial serialBT;

// Preferences
Preferences preferences;

// Config
long icModel;
long icConnect;
long icConnectOld;
String icSerialDevice;
char icCIVAddress = 0xA4;
uint8_t icAddress[6];

// Global Variables
WiFiServer httpServer(80);
WiFiClient httpClient, civClient;

int8_t config = 0;
int8_t beep = 0;
int8_t transverter = 0;
int8_t voice = 0;
int8_t voiceMode = 0;
int8_t voiceTimeout = 0;
int8_t screensaver = 0;
int8_t configOld = 255;

uint8_t voiceCounter = 0;
uint8_t htmlGetRequest;
uint8_t option = 2;
uint8_t brightness = 64;
uint8_t sOld = 255;
uint8_t SWROld = 255;
uint8_t powerOld = 255;
uint8_t IdOld = 255;
uint8_t COMPLOld = 255;
uint8_t ALCOld = 127;
uint8_t TXOld = 127;
uint8_t agcOld = 127;
uint8_t ANOld = 127;
uint8_t NBOld = 127;
uint8_t NROld = 127;
uint8_t AMPOld = 127;
uint8_t toneOld = 127;
uint8_t AFOld = 127;
uint8_t MICOld = 127;
uint8_t SQLOld = 127;
uint8_t COMPOld = 127;
uint8_t NRLevelOld = 127;
uint8_t NBLevelOld = 127;
uint8_t VdOld = 0;
uint8_t IPOld = 0;
uint8_t batteryLevelOld = 0;

uint32_t screensaverTimer;
uint32_t transmit;

boolean screensaverMode = false;
boolean screenshot = false;
boolean settingsMode = false;
boolean settingLock = true;
boolean btConnected = false;
boolean btClient = false;
boolean wifiConnected = false;
boolean proxyConnected = false;
boolean txConnected = true;
boolean needClear = true;
boolean startup = true;
boolean wakeup = true;
boolean batteryCharginglOld = true;
boolean voiceRefresh = true;
boolean bandeLow;

String frequencyOld = "";
String filterOld = "";
String modeOld = "";
String RITOld = "";

char valStringOld[32];

// Flags for button presses via Web site Screen Capture
bool buttonLeftPressed = false;
bool buttonCenterPressed = false;
bool buttonRightPressed = false;

// Bin loader
File root;
String binFilename[128];
uint8_t binIndex = 0;

// Menu
const char *settings[] = {"Config", "Voice TX", "Transverter Mode", "Brightness", "Beep", "Screensaver", "IP Address", "Shutdown", "Exit"};
const char *choiceVoice[] = {"OFF", "T1", "T2", "T3", "T4", "T5", "T6", "T7", "T8"};
const char *choiceBrightness[] = {"BRIGHTNESS"};
const char *choiceBeep[] = {"BEEP LEVEL"};
const char *choiceScreensaver[] = {"TIMEOUT"};
const double choiceTransverter[] = {
    0,
    TRANSVERTER_LO_1,
    TRANSVERTER_LO_2,
    TRANSVERTER_LO_3,
    TRANSVERTER_LO_4,
    TRANSVERTER_LO_5
};