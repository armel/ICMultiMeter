// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "settings.h"
#include "ICMultiMeter.h"
#include "font.h"
#include "image.h"
#include "tools.h"
#include "webIndex.h"
#include "command.h"
#include "functions.h"
#include "menu.h"
#include "tasks.h"

// Setup
void setup()
{
  uint8_t loop = 0;

  // Init M5
  auto cfg = M5.config();
  M5.begin(cfg);

  pinMode(32, INPUT_PULLUP);
  pinMode(26, INPUT_PULLUP);

  // Init Display
  display.begin();

  offsetX = (display.width() - 320) / 2; 
  offsetY = (display.height() - 240) / 2;

  // Preferences
  preferences.begin(NAME);
  led = preferences.getUInt("led", 0);
  brightness = preferences.getUInt("brightness", 64);
  transverter = preferences.getUInt("transverter", 0);
  voice = preferences.getUInt("voice", 0);
  beep = preferences.getUInt("beep", 0);
  screensaver = preferences.getUInt("screensaver", 60);
  config = preferences.getUInt("config", 0);

  // Init Setting
  size_t n = sizeof(choiceConfig) / sizeof(choiceConfig[0]);
  n = (n / 4) - 1;

  if(config > n)
  {
    config = n;
  }

  icModel = strtol(choiceConfig[(config * 4) + 0], 0, 10);
  icCIVAddress = strtol(String(choiceConfig[(config * 4) + 1]).substring(2, 4).c_str(), 0, 16);
  if(strcmp(choiceConfig[(config * 4) + 2], "USB") == 0)
  {
    icConnect = USB;
    icSerialDevice = choiceConfig[(config * 4) + 3];
  }
  else
  {
    icConnect = BT;
    uint8_t i = 0;
    while(i <= 15)
    {
      icAddress[i/3] = strtol(String(choiceConfig[(config * 4) + 3]).substring(i, i + 2).c_str(), 0, 16);
      Serial.println(icAddress[i/3]);
      i += 3;
    }
  }
  icConnectOld = icConnect;

  // Init Sprite
  if(icConnect == USB || ESP.getPsramSize() > 0) // Sprite mode
  {
  }

  gaugeSprite.setColorDepth(8);
  gaugeSprite.createSprite(180, 8);

  levelSprite.setColorDepth(8);
  levelSprite.createSprite(44, 10);

  logoSprite.setColorDepth(8);
  logoSprite.createSprite(44, 22);
  logoSprite.drawJpg(logo, sizeof(logo), 0, 0, 44, 22);

  // Init Led
  if(M5.getBoard() == m5::board_t::board_M5Stack) {
    FastLED.addLeds<NEOPIXEL, 15>(leds, NUM_LEDS);  // GRB ordering is assumed
  }
  else if(M5.getBoard() == m5::board_t::board_M5StackCore2) {
    FastLED.addLeds<NEOPIXEL, 25>(leds, NUM_LEDS);  // GRB ordering is assumed
  }

  // Bin Loader
  binLoader();

  // Wifi
  WiFi.onEvent(callbackWifiOn, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent(callbackWifiOff, SYSTEM_EVENT_STA_DISCONNECTED);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED && loop <= 10)
  {
    vTaskDelay(250);
    loop += 1;
  }

  Serial.println(WiFi.localIP());
 
  // Start server (for Web site Screen Capture)
  httpServer.begin();

  display.setBrightness(map(brightness, 1, 100, 1, 254));
  display.setRotation(1);
  display.fillScreen(TFT_BLACK);

  viewGUI();

  serialBT.register_callback(callbackBT);

  if(icModel == 705 && icConnect == BT)
  {
    uint8_t attempt = 0;
    char valString[24] = "Connexion";
    char dot[4] = "";

    //value(strcat(valString, dot));

    serialBT.begin(NAME, true);
    btClient = serialBT.connect(icAddress);

    while(!btClient && attempt < 3) 
    {
      Serial.printf("Attempt %d - Make sure IC-705 is available and in range.", attempt + 1);

      sprintf(dot, "%.*s", 1, ".....");
      value(strcat(valString, dot));

      btClient = serialBT.connect(icAddress);
      attempt++;
    }
  
    if(!btClient) 
    {
      if (!serialBT.begin(NAME))
      {
        Serial.println("An error occurred initializing Bluetooth");
      }
      else
      {
        Serial.println("Bluetooth initialized");
      }
    }
    else
    {
      snprintf(valString, 24, "%s", "");
      value(valString);
    }
  }
  else
  {
    if (WiFi.status() == WL_CONNECTED) wifiConnected = true;
  }

  // Multitasking task for retreive button
  xTaskCreatePinnedToCore(
      button,   // Function to implement the task
      "button", // Name of the task
      8192,     // Stack size in words
      NULL,     // Task input parameter
      4,        // Priority of the task
      NULL,     // Task handle
      1);       // Core where the task should run

  // Multitasking task for retreive scroll
  /*
  xTaskCreatePinnedToCore(
      scroll,   // Function to implement the task
      "scroll", // Name of the task
      8192,     // Stack size in words
      NULL,     // Task input parameter
      2,        // Priority of the task
      NULL,     // Task handle
      1);       // Core where the task should run
  */
}

// Main loop
void loop()
{
  static uint8_t alternance = 0;
  static uint8_t chargeOld = 128;
  static uint8_t tx = 0;
  static uint8_t mode = 0;
  static uint8_t charge = 0;
  static uint8_t comp = 0;
  
  if(checkConnection()) {
    tx = getTX();
    if(tx != 0) screensaverTimer = millis();   // If transmit, refresh tempo

    if (screensaverMode == false && screenshot == false && settingsMode == false)
    {
      settingLock = true;

      if(charge != chargeOld) {
        chargeOld = charge;
        display.setFont(0);
        display.setTextPadding(35);
        display.setTextDatum(CR_DATUM);
        display.setTextColor(TFT_WHITE, TFT_BLACK);

        if(icModel == 705 && charge == 0) 
        {
          display.drawString("(10W)", 194 + offsetX, 138 + offsetY);
        }
        else if(icModel == 705 && charge == 1)
        {
          display.drawString("(5W)", 194 + offsetX, 138 + offsetY);
        }
      }
      
      settingLock = false;
    }  

    if (screensaverMode == false && screenshot == false && settingsMode == false)
    {
      settingLock = true;

      getFrequency();
      
      if(tx == 0) {
        if(needClear == false) {
          getALCLevel();
          clearGUI();
          if(strcmp(choiceLed[led], "TX") == 0)
          {
            for(uint8_t i = 0; i <= 9; i++){
              leds[i] = CRGB::Black;
            }
            FastLED.setBrightness(16);
            FastLED.show();
          }
          needClear = true;
        }
        getSmeterLevel();
      }
      else {
        screensaverTimer = millis();   // If transmit, refresh tempo
        if(needClear) {
          if(strcmp(choiceLed[led], "TX") == 0)
          {
            for(uint8_t i = 0; i <= 9; i++){
              leds[i] = CRGB::Red;
            }
            FastLED.setBrightness(16);
            FastLED.show();
          }
          needClear = false;
        }

        getALCLevel();
        getSWRLevel();
        getIdLevel();
        if(mode <= 1 && comp == 1) {
          getCOMPLevel();
        }
        getPowerLevel(charge);
      }

      settingLock = false;
    }

    if (screensaverMode == false && screenshot == false && settingsMode == false)
    {
      settingLock = true;

      switch (alternance)
      {
        case 0:
          charge = getPowerType();
          if(mode == 5 || mode == 6) {
            getTone();
          }
          else {
            getTone(false);
          }
          getAGC();
          getAMP();
          getVdLevel();
          break;

        case 1:
          mode = getModeFilter();
          if(getNR())
          {
            getNRLevel();
          }
          else
          {
            cleanNRLevel();
          }
          if(getNB())
          {
            getNBLevel();
          }
          else
          {
            cleanNBLevel();
          }
          getAN();
          getRIT();
          break;

        case 2:
          if(mode <= 1) {
            comp = getCOMP();
          }
          else {
            comp = getCOMP(false);
          }
          getAF();
          getSQL();
          getMIC();
          getIP();
          break;
      }

      // Manage voice
      voiceManager(tx, alternance);

      if(tx == 0) {
        getSmeterLevel();
      }
      else {
        getPowerLevel(charge);
      }

      // View battery
      viewBattery();

      settingLock = false;
    }
  }
  
  alternance = (alternance++ < 2) ? alternance : 0;

  // Manage Screen Saver
  wakeAndSleep();

  if(DEBUG == 1)
  {
    Serial.printf("%d kb %d kb %d kb %d kb\n", 
      ESP.getHeapSize() / 1024,
      ESP.getFreeHeap() / 1024, 
      ESP.getPsramSize() / 1024, 
      ESP.getFreePsram() / 1024
    );

    Serial.printf("%d %ld %ld\n", screensaverMode, millis() - screensaverTimer, long(screensaver * 60 * 1000));
  }
}