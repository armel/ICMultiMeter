// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Get Button
void button(void *pvParameters)
{
  for (;;)
  {

    getButton();

    if(btnA || btnB || btnC) {
      screensaver = millis();
    }

    if (btnA == 1 || buttonLeftPressed == 1)
    {
      option = 0;
      buttonLeftPressed = 0;
      preferences.putUInt("option", option);
    }
    else if (btnB == 1 || buttonCenterPressed == 1)
    {
      option = 1;
      buttonCenterPressed = 0;
      preferences.putUInt("option", option);
    }
    else if (btnC == 1 || buttonRightPressed == 1)
    {
      option = 2;
      buttonRightPressed = 0;
      preferences.putUInt("option", option);
    }
    else if (btnL == 1) {
      brightness -= 1;
      if(brightness < 1) {
        brightness = 1;
      }
      setBrightness(brightness);
      Serial.println(brightness);
      preferences.putUInt("brightness", brightness);
      vTaskDelay(pdMS_TO_TICKS(50));
    }
    else if (btnM == 1) {
      shutdown();
    }
    else if (btnR == 1) {
      brightness += 1;
      if(brightness > 254) {
        brightness = 254;
      }
      setBrightness(brightness);
      Serial.println(brightness);
      preferences.putUInt("brightness", brightness);
      vTaskDelay(pdMS_TO_TICKS(50));
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// Manage scroll

// Scroll
void scroll(void *pvParameters)
{
  uint16_t h = 12;
  uint16_t w;
  uint32_t timer = 0, wait = 0, limit = 40; // Retry all 100ms

  for(;;) {
    timer = millis();

    w = Sprite.textWidth(message) + 40;
    if (w < 320)
    {
      w = 320;
    }
    // We could just use fillSprite(color) but lets be a bit more creative...
    while (h--)
      Sprite.drawFastHLine(0, h, w, TFT_BLACK);

    // Now print text on top of the graphics
    Sprite.setFreeFont(0);
    Sprite.setTextColor(TFT_DARKGREY); // White text, no background colour
    Sprite.setTextWrap(false);      // Turn of wrap so we can print past end of sprite

    // Need to print twice so text appears to wrap around at left and right edges
    Sprite.drawString(message, pos, 2);
    Sprite.drawString(message, pos - w, 2);

    Sprite.pushSprite(0, 95);

    pos -= 1;
    if (pos == 0)
    {
      pos = w;
    }

    wait = millis() - timer;
    if (wait < limit)
    {
      vTaskDelay(pdMS_TO_TICKS(limit - wait));
      // Serial.println(pause - wait);
    }
  }
}