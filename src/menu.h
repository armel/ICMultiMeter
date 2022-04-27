// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

void viewMenu(uint8_t x, uint8_t y, uint16_t w, uint8_t h) {
    M5.Lcd.fillRoundRect(x, y, w, h, 8, TFT_MENU_BACK);
    M5.Lcd.drawRoundRect(x, y, w, h, 8, TFT_MENU_BORDER);

    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setFont(&YELLOWCRE8pt7b);
    M5.Lcd.setTextPadding(w - 2);
    M5.Lcd.setTextColor(TFT_MENU_SELECT);
    M5.Lcd.drawString("SETTINGS", 160, 14 + y);

    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setFont(0);
    M5.Lcd.setTextPadding(w - 2);
    M5.Lcd.setTextColor(TFT_MENU_SELECT);
    M5.Lcd.drawString(String(NAME) + " V" + String(VERSION) + " by " + String(AUTHOR), 160, 28 + y);

    M5.Lcd.drawFastHLine(x + 1, 36 + y, w - 2, TFT_MENU_SELECT);
    M5.Lcd.drawFastHLine(x + 1, (y + h) - 24, w - 2, TFT_MENU_SELECT);
}

void viewOption(int8_t settingsChoice, boolean settingsSelect, uint8_t x, uint8_t y, uint16_t w) {
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setFont(&tahoma8pt7b);
    M5.Lcd.setTextPadding(w - 2);
    M5.Lcd.setTextColor(TFT_MENU_SELECT, TFT_MENU_BACK);

    size_t stop = sizeof(settings) / sizeof(settings[0]);
    for(uint8_t i = 0; i < stop; i++) {
        if(settingsChoice == i && settingsSelect == false) {
            M5.Lcd.setTextColor(TFT_BLACK, TFT_MENU_SELECT);
            M5.Lcd.drawString(settings[i], 160, 45 + y + (i * 18));
        }
        else if(settingsChoice == i && settingsSelect == true) {
            M5.Lcd.setTextColor(TFT_BLACK, TFT_MENU_SELECT);
            M5.Lcd.drawString(settings[i], 160, 45 + y + (i * 18));
        }
        else {
            M5.Lcd.setTextColor(TFT_MENU_SELECT, TFT_MENU_BACK);
            M5.Lcd.drawString(settings[i], 160, 45 + y + (i * 18));
        }
    }
}