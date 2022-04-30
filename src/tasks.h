// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Get Button
void button(void *pvParameters)
{
  int8_t transverterOld = 0;
  int8_t voiceOld = 0;
  uint8_t brightnessOld = 64;
  static int8_t settingsChoice = 0;
  static boolean settingsSelect = false;

  uint8_t x = 44;
  uint8_t y = 4;
  uint16_t w = 320 - (x * 2);
  uint8_t h = 190;

  for (;;)
  {
    // Get screenshot
    getScreenshot();

    // Get button
    getButton();
            
    if(btnA || btnB || btnC) {
      screensaver = millis();
      brightnessOld = preferences.getUInt("brightness");
      transverterOld = preferences.getUInt("transverter");
      voiceOld = preferences.getUInt("voice");
    }

    if(settingsMode == false)
    {
      // Voice mode
      if(btnA || btnC) {
        if(voice != 0)
        {
          if(btnA) { // 1 time
            if(voiceCounter == 0)
            {
              voiceMode = 2;
              voiceCounter = 1;
            }
            vTaskDelay(pdMS_TO_TICKS(250));
          }
          else if(btnC) { // n time
            if(voiceCounter == 0)
            {
              voiceMode = 2;
              voiceCounter = 255;
            }
            else {
              voiceMode = 1;
              voiceCounter = 0;
            }
            vTaskDelay(pdMS_TO_TICKS(250));
          }
        }
      }
      // Enter settings
      else if(btnB) {
        settingsMode = true;
        while(settingLock == true) {
          vTaskDelay(10);
        }
        viewMenu(x, y, w, h);
        viewOption(settingsChoice, settingsSelect, x, y, w);
        vTaskDelay(500);
      }
    }
    // Select settings
    else if(settingsMode == true)
    {
      if(settingsSelect == false) {
        if(btnA || btnC) {
          if(btnA) {
            settingsChoice--;
          }
          else if(btnC) {
            settingsChoice++;
          }

          size_t stop = sizeof(settings) / sizeof(settings[0]);
          stop--;

          settingsChoice = (settingsChoice < 0) ? stop : settingsChoice;
          settingsChoice = (settingsChoice > stop) ? 0 : settingsChoice;

          viewOption(settingsChoice, settingsSelect, x, y, w);
        }
        else if(btnB) {
          voiceRefresh = true;
          settingsSelect = true;
          viewOption(settingsChoice, settingsSelect, x, y, w);

          String settingsString = String(settings[settingsChoice]);
          if(settingsString == "Shutdown") {
            shutdown();
          }
          else if(settingsString == "Exit") {
            clearData();
            viewGUI();
            settingsSelect = false;
            settingsMode = false;
            vTaskDelay(pdMS_TO_TICKS(150));
          }
        }
        vTaskDelay(pdMS_TO_TICKS(150));
      }
      // Manage settings
      else if(settingsSelect == true) {
        String settingsString = String(settings[settingsChoice]);

        M5.Lcd.setTextDatum(CC_DATUM);
        M5.Lcd.setFont(&YELLOWCRE8pt7b);
        M5.Lcd.setTextPadding(w - 2);
        M5.Lcd.setTextColor(TFT_MENU_SELECT, TFT_MENU_BACK);

        // Brightness
        if(settingsString == "Brightness")
        {
          M5.Lcd.drawString(String(choiceBrightness[0]) + " " + String(map(brightness, 1, 254, 1, 100)) + "%", 160, h - 6);

          if(btnA || btnC) {
            if(btnA == 1) {
              brightness -= 1;
              if(brightness < 1) {
                brightness = 1;
              }
            }
            else if(btnC == 1) {
              brightness += 1;
              if(brightness > 254) {
                brightness = 254;
              }
            }
          }
          else if(btnB == 1) {
            if(brightnessOld != brightness)
              preferences.putUInt("brightness", brightness);
            clearData();
            viewGUI();
            settingsSelect = false;
            settingsMode = false;
            vTaskDelay(pdMS_TO_TICKS(150));
          }
          setBrightness(brightness);
          vTaskDelay(pdMS_TO_TICKS(25));
        }

        // Transverter
        else if(settingsString == "Transverter Mode")
        {
          M5.Lcd.drawString(String(choiceTransverter[transverter]), 160, h - 6);

          if(btnA || btnC) {
            if(btnA == 1) {
              transverter -= 1;
              if(transverter < 0) {
                transverter = 1;
              }
            }
            else if(btnC == 1) {
              transverter += 1;
              if(transverter > 1) {
                transverter = 0;
              }
            }
          }
          else if(btnB == 1) {
            if(transverterOld != transverter)
              preferences.putUInt("transverter", transverter);
            clearData();
            viewGUI();
            settingsSelect = false;
            settingsMode = false;
            vTaskDelay(pdMS_TO_TICKS(150));
          }
          vTaskDelay(pdMS_TO_TICKS(150));
        }

        // Voice TX
        else if(settingsString == "Voice TX")
        {
          M5.Lcd.drawString(String(choiceVoice[voice]), 160, h - 6);

          if(btnA || btnC) {
            if(btnA == 1) {
              voice -= 1;
              if(voice < 0) {
                voice = 8;
              }
            }
            else if(btnC == 1) {
              voice += 1;
              if(voice > 8) {
                voice = 0;
              }
            }
          }
          else if(btnB == 1) {
            if(voiceOld != voice)
              preferences.putUInt("voice", voice);

            clearData();
            viewGUI();
            settingsSelect = false;
            settingsMode = false;
            vTaskDelay(pdMS_TO_TICKS(150));
          }
          vTaskDelay(pdMS_TO_TICKS(150));
        }

        // IP Address
        else if(settingsString == "IP Address")
        {
          M5.Lcd.drawString(String(WiFi.localIP().toString().c_str()), 160, h - 6);

          if(btnB == 1) {
            clearData();
            viewGUI();
            settingsSelect = false;
            settingsMode = false;
            vTaskDelay(pdMS_TO_TICKS(150));
          }
          vTaskDelay(pdMS_TO_TICKS(150));
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));  
  }    
}