// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <ICMultiMeter.h>
#include "image.h"
#include "functions.h"
#include "command.h"
#include "tasks.h"

// Setup
void setup()
{
  uint8_t loop = 0;

  // Debug
  Serial.begin(115200);

  // Init screensaver timer
  screensaver = millis();

  // Init M5
  M5.begin(true, true, false, false);

  // Init Led
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

  // Init Power
  power();

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
    delay(250);
    loop += 1;
  }

  Serial.println(WiFi.localIP());

  // Scroll
  pos = M5.Lcd.width();
  Sprite.setTextSize(1);  // Font size scaling is x1
  Sprite.setTextFont(2);  // Font 2 selected
  Sprite.createSprite(M5.Lcd.width(), 12);

  message = String(NAME) + " V" + String(VERSION) + " by " + String(AUTHOR);
 
  // Start server (for Web site Screen Capture)
  httpServer.begin();

  // Let's go
  #if BOARD == CORE2
    M5.Axp.SetLed(0);
  #endif

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

    if(screensaverMode == 0)
    {
      if(charge != chargeOld) {
        chargeOld = charge;
        M5.Lcd.setFreeFont(0);
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

  if(DEBUG) {
    Serial.println("----------");
    Serial.print("Alternance ");
    Serial.println(alternance);
  }

  // View battery
  viewBattery();

  // View baseline
  viewBaseline();

  // Manage Screen Saver
  wakeAndSleep();

  // Manage Web Server if enable
  if (WiFi.status() == WL_CONNECTED)
  {
    getScreenshot();
  }
  else {
    wifiConnected = false;
  }
}