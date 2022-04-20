// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "settings.h"
#include "ICMultiMeter.h"
#include "font.h"
#include "image.h"
#include "tools.h"
#include "webIndex.h"
#include "functions.h"
#include "command.h"
#include "tasks.h"

// Setup
void setup()
{
  uint8_t loop = 0;

  // Init M5
  auto cfg = M5.config();
  M5.begin(cfg);

  // Init Led
  if(M5.getBoard() == m5::board_t::board_M5Stack) {
    FastLED.addLeds<NEOPIXEL, 15>(leds, NUM_LEDS);  // GRB ordering is assumed
  }
  else if(M5.getBoard() == m5::board_t::board_M5StackCore2) {
    FastLED.addLeds<NEOPIXEL, 25>(leds, NUM_LEDS);  // GRB ordering is assumed
  }

  // Preferences
  preferences.begin(NAME);
  option = preferences.getUInt("option", 2);
  brightness = preferences.getUInt("brightness", 64);

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

  setBrightness(brightness);
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(TFT_BLACK);

  viewGUI();

  if(IC_MODEL == 705 && IC_CONNECT == BT)
  {
    CAT.register_callback(callbackBT);

    if (!CAT.begin(NAME))
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
    if(tx != 0) screensaver = millis();   // If transmit, refresh tempo

    if (screensaverMode == false && screenshot == false)
    {
      if(charge != chargeOld) {
        chargeOld = charge;
        M5.Lcd.setFont(0);
        M5.Lcd.setTextPadding(35);
        M5.Lcd.setTextDatum(CR_DATUM);
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

        if( IC_MODEL == 705 && charge == 0) 
        {
          M5.Lcd.drawString("(10W)", 194, 138);
        }
        else if(IC_MODEL == 705 && charge == 1)
        {
          M5.Lcd.drawString("(5W)", 194, 138);
        }
      }  

      getFrequency();
      
      if(tx == 0) {
        if(needClear == false) {
          getALCLevel();
          clearGUI();
          for(uint8_t i = 0; i <= 9; i++){
            leds[i] = CRGB::Black;
          }
          FastLED.setBrightness(16);
          FastLED.show();
          needClear = true;
        }
        getSmeterLevel();
      }
      else {
        screensaver = millis();   // If transmit, refresh tempo
        if(needClear) {
          for(uint8_t i = 0; i <= 9; i++){
            leds[i] = CRGB::Red;
          }
          FastLED.setBrightness(16);
          FastLED.show();
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
          getNR();
          getNB();
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
          break;
      }
    
      if(tx == 0) {
        getSmeterLevel();
      }
      else {
        getPowerLevel(charge);
      }
    }
  }
  
  alternance = (alternance++ < 2) ? alternance : 0;

  // View battery
  viewBattery();

  // View baseline
  viewBaseline();

  // Manage Screen Saver
  wakeAndSleep();

  //M5.Lcd.drawFastHLine(0, 57, 320, TFT_FIL_BORDER);
  //M5.Lcd.drawFastHLine(0, 86, 320, TFT_FIL_BORDER);
}