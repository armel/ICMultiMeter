// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

void viewMenu(uint8_t x, uint8_t y, uint16_t w, uint8_t h)
{
  display.fillRoundRect(x, y, w, h, 8, TFT_MENU_BACK);
  display.drawRoundRect(x, y, w, h, 8, TFT_MENU_BORDER);

  display.setTextDatum(CC_DATUM);
  display.setFont(&YELLOWCRE8pt7b);
  display.setTextPadding(w - 2);
  display.setTextColor(TFT_MENU_SELECT);
  display.drawString("SETTINGS", 160 + offsetX, 14 + y);

  display.setTextDatum(CC_DATUM);
  display.setFont(0);
  display.setTextPadding(w - 2);
  display.setTextColor(TFT_MENU_SELECT);
  display.drawString(String(NAME) + " V" + String(VERSION) + " by " + String(AUTHOR), 160 + offsetX, 28 + y);

  display.drawFastHLine(x + 1, 36 + y, w - 2, TFT_MENU_SELECT);
  display.drawFastHLine(x + 1, (y + h) - 24, w - 2, TFT_MENU_SELECT);
}

void viewOption(int8_t settingsChoice, boolean settingsSelect, uint8_t x, uint8_t y, uint16_t w)
{
  uint8_t start = 0;
  uint8_t i, j;

  display.setTextDatum(CC_DATUM);
  display.setFont(&tahoma8pt7b);
  display.setTextPadding(w - 2);
  display.setTextColor(TFT_MENU_SELECT, TFT_MENU_BACK);

  size_t stop = sizeof(settings) / sizeof(settings[0]);

  if (settingsChoice > 6)
  {
    start = settingsChoice - 6;
  }
  else
  {
    start = 0;
  }

  stop = start + 7;

  j = 0;
  for (i = start; i < stop; i++)
  {
    if (settingsSelect == false)
    {
      if (settingsChoice == i)
      {
        display.setTextColor(TFT_BLACK, TFT_MENU_SELECT);
        display.drawString(settings[i], 160 + offsetX, 45 + y + (j * 18));
      }
      else
      {
        display.setTextColor(TFT_MENU_SELECT, TFT_MENU_BACK);
        display.drawString(settings[i], 160 + offsetX, 45 + y + (j * 18));
      }
    }
    else
    {
      if (settingsChoice == i)
      {
        display.setTextColor(TFT_BLACK, TFT_MENU_SELECT);
        display.drawString(settings[i], 160 + offsetX, 45 + y + (j * 18));
      }
      else
      {
        display.setTextColor(TFT_SILVER, TFT_MENU_BACK);
        display.drawString(settings[i], 160 + offsetX, 45 + y + (j * 18));
      }
    }
    j++;
    if (j > 7)
    {
      j = 7;
    }
  }
}