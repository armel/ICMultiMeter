// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Bluetooth callback
void callbackBT(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  if (event == ESP_SPP_SRV_OPEN_EVT)
  {
    btClient = true;
    Serial.println("BT Client Connected");
  }
  if (event == ESP_SPP_CLOSE_EVT)
  {
    btClient = false;
    Serial.println("BT Client disconnected");
  }
}

// Wifi callback On
void callbackWifiOn(WiFiEvent_t event, WiFiEventInfo_t info)
{
  wifiConnected = true;
  Serial.println("Wifi Client Connected");
}

// Wifi callback Off
void callbackWifiOff(WiFiEvent_t event, WiFiEventInfo_t info)
{
  wifiConnected = false;
  Serial.println("Wifi Client disconnected");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

// List files on SPIFFS or SD
void getBinaryList(File dir, String type)
{
  while (true)
  {
    File entry = dir.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }

    if (strstr(entry.name(), "/.") == NULL && strstr(entry.name(), ".bin") != NULL)
    {
      // Serial.println(type + "_" + entry.name());
      binFilename[binIndex] = type + "_" + entry.name();
      binIndex++;
    }

    if (entry.isDirectory() && strstr(entry.name(), "/.") == NULL)
    {
      getBinaryList(entry, type);
    }

    entry.close();
  }
}

// Bin Loader
void binLoader()
{
  boolean click = 0;
  int8_t cursor = 0;
  int8_t start = 0;
  int8_t stop = 0;
  int8_t limit = 8;
  int8_t change = 255;
  String tmpName;

  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS Mount Failed");

    display.setTextFont(1);
    display.setTextSize(2);

    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(CC_DATUM);
    display.drawString("Flash File System", 160, 20);
    display.drawString("needs to be formated.", 160, 50);
    display.drawString("It takes around 4 minutes.", 160, 100);
    display.drawString("Please, wait until ", 160, 150);
    display.drawString("the application starts !", 160, 180);

    Serial.println("SPIFFS Formating...");

    SPIFFS.format(); // Format SPIFFS...

    display.setTextFont(0);
    display.setTextSize(0);

    return;
  }

  root = SPIFFS.open("/");
  getBinaryList(root, "SP");

  if (SD.begin(GPIO_NUM_4, SPI, 25000000))
  {
    root = SD.open("/");
    getBinaryList(root, "SD");
  }

  if (binIndex != 0)
  {
    // QRCode
    display.qrcode("https://github.com/armel/ICMultiMeter", 90, 80, 140, 6);

    display.setTextFont(1);
    display.setTextSize(1);

    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(CC_DATUM);

    for (uint8_t i = TIMEOUT_BIN_LOADER * 10; i > 0; i--)
    {
      getButton();

      if (i % 10 == 0)
      {
        tmpName += ".";
        display.drawString(tmpName, 160, 20);
      }

      if (btnA || btnC)
      {
        SD.end(); // If not Bluetooth doesn't work !!!
        return;
      }
      else if (btnB)
      {
        click = 1;
        display.fillRect(0, 0, 320, 240, TFT_BLACK);

        break;
      }

      vTaskDelay(100);
    }
  }

  while (click == 1)
  {
    while (btnB != 0)
    {
      getButton();
      vTaskDelay(100);
    }

    display.setTextFont(1);
    display.setTextSize(2);

    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(CC_DATUM);
    display.drawString("Bin Loader V0.2", 160, 20);

    getButton();

    if (btnA)
    {
      cursor--;
    }
    else if (btnC)
    {
      cursor++;
    }
    else if (btnB)
    {
      if (binFilename[cursor].substring(0, 4) == "SP_/")
      {
        updateFromFS(SPIFFS, binFilename[cursor].substring(3));
      }
      else
      {
        updateFromFS(SD, binFilename[cursor].substring(3));
      }
      ESP.restart();
    }

    cursor = (cursor < 0) ? binIndex - 1 : cursor;
    cursor = (cursor > binIndex - 1) ? 0 : cursor;

    start = cursor / limit;

    stop = (start * limit) + limit;

    /*
    Serial.print(cursor);
    Serial.print("-");
    Serial.print(start);
    Serial.print("-");
    Serial.print(stop);
    Serial.println("----------");
    */

    if (change != cursor)
    {
      change = cursor;
      display.setTextPadding(320);

      uint8_t i = 0;
      for (uint8_t j = (start * limit); j < stop; j++)
      {
        tmpName = binFilename[j].substring(4);

        if (cursor == j)
        {
          tmpName = ">> " + tmpName + " <<";

          if (binFilename[cursor].substring(0, 4) == "SP_/")
          {
            display.setTextSize(1);
            display.drawString("SPI Flash File Storage", 160, 50);
          }
          else
          {
            display.setTextSize(1);
            display.drawString("SD Card Storage", 160, 50);
          }
        }

        display.setTextSize(2);
        display.drawString(tmpName, 160, 80 + i * 20);
        i++;
      }
    }
    vTaskDelay(100);
  }
  SD.end(); // If not Bluetooth doesn't work !!!
}

// Print battery
void viewBattery()
{
  uint8_t batteryLevel;
  boolean batteryCharging;

  if (screensaverMode == false)
  {
    // On left, view battery level
    batteryLevel = map(getBatteryLevel(1), 0, 100, 0, 16);
    batteryCharging = isCharging();

    if (batteryLevel != batteryLevelOld || batteryCharging != batteryCharginglOld)
    {

      batteryLevelOld = batteryLevel;
      batteryCharginglOld = batteryCharging;

      display.drawRect(294 + offsetX, 4 + offsetY, 20, 12, TFT_WHITE);
      display.drawRect(313 + offsetX, 7 + offsetY, 4, 6, TFT_WHITE);
      display.fillRect(296 + offsetX, 6 + offsetY, 16, 8, TFT_BLACK);
      display.fillRect(296 + offsetX, 6 + offsetY, batteryLevel, 8, TFT_WHITE);

      if (batteryCharging)
      {
        display.setTextColor(TFT_WHITE, TFT_BLACK);
        display.setFont(0);
        display.setTextDatum(CR_DATUM);
        display.setTextPadding(28);
        display.drawString("+", 290 + offsetX, 11 + offsetY);
      }
      else
      {
        display.setTextColor(TFT_WHITE, TFT_BLACK);
        display.setFont(0);
        display.setTextDatum(CR_DATUM);
        display.setTextPadding(28);
        display.drawString(String(getBatteryLevel(1)) + "%", 290 + offsetX, 11 + offsetY);
      }
    }
  }
}

// View GUI
void viewGUI()
{
  // Clear
  display.fillRect(0 + offsetX, 0 + offsetY, 320, 240, TFT_BLACK);

  // IC Connect

  display.setFont(0);
  display.setTextDatum(CC_DATUM);

  display.fillRoundRect(261 + offsetX, 96 + offsetY, 56, 13, 2, TFT_MODE_BACK);
  display.drawRoundRect(261 + offsetX, 96 + offsetY, 56, 13, 2, TFT_MODE_BORDER);
  display.setTextColor(TFT_WHITE);

  if (icConnect == BT)
    display.drawString(String(icModel) + " BT", 289 + offsetX, 103 + offsetY);
  else
    display.drawString(String(icModel) + " USB", 289 + offsetX, 103 + offsetY);

  if (transverter > 0)
  {
    display.fillRoundRect(230 + offsetX, 96 + offsetY, 26, 13, 2, TFT_MODE_BACK);
    display.drawRoundRect(230 + offsetX, 96 + offsetY, 26, 13, 2, TFT_MODE_BORDER);
    display.setTextColor(TFT_WHITE);
    display.drawString("LO" + String(transverter), 244 + offsetX, 103 + offsetY);
  }

  /*
  display.setFont(0);
  display.setTextDatum(CC_DATUM);

  display.fillRoundRect(283, 90, 34, 20, 2, TFT_MODE_BACK);
  display.drawRoundRect(283, 90, 34, 20, 2, TFT_MODE_BORDER);
  display.setTextColor(TFT_WHITE);

  if (icConnect == BT)
    display.drawString("BT", 300, 96);
  else
    display.drawString("USB", 300, 96);

  display.drawString(String(icModel), 300, 105);

  // display.drawFastHLine(0, 57, 320, TFT_FIL_BORDER);
  // display.drawFastHLine(0, 86, 320, TFT_FIL_BORDER);

  // display.drawJpg(logo, sizeof(logo), 272, 0, 44, 22);
  // display.drawJpg(logo, sizeof(logo), 40, 0, 44, 22);
  // display.drawJpg(logo, sizeof(logo), 272, 94, 44, 22);
  */

  display.drawJpg(logo, sizeof(logo), 0 + offsetX, 49 + offsetY, 44, 22);

  display.setFont(0);
  display.setTextPadding(0);

  // Id
  display.setTextDatum(CR_DATUM);
  display.setTextColor(TFT_WHITE);

  display.drawFastHLine(30 + offsetX, 226 + offsetY, 180, TFT_FIL_BORDER);
  display.drawString("Id", 28 + offsetX, 223 + offsetY);

  display.setTextDatum(CC_DATUM);
  display.setTextColor(TFT_WHITE);

  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    display.drawFastVLine(30 + i + offsetX, 218 + offsetY, 8, TFT_FIL_BACK);
  }

  if(icModel == 705)
  {
    for (uint8_t i = 0; i <= 12; i++)
    {
      if (i == 12)
      {
        display.drawString(String(" 4 A"), 30 + (i * 14) + offsetX, 234 + offsetY);
        display.drawFastVLine(30 + (i * 14) + offsetX, 224 + offsetY, 2, TFT_FIL_BORDER);
      }
      else if (i % 3 == 0)
      {
        display.drawString(String(i / 3), 30 + (i * 14) + offsetX, 234 + offsetY);
        display.drawFastVLine(30 + (i * 14) + offsetX, 224 + offsetY, 2, TFT_FIL_BORDER);
      }
      else
      {
        display.drawString(String("."), 30 + (i * 14) + offsetX, 230 + offsetY);
      }
    }
  }
  else if(icModel == 7300)
  {
    for (uint8_t i = 0; i <= 15; i++)
    {
      if (i == 15)
      {
        display.drawString(String("25A"), 30 + (i * 11.3) + 4 + offsetX, 234 + offsetY);
        display.drawFastVLine(30 + (i * 11.3) + offsetX, 224 + offsetY, 2, TFT_FIL_BORDER);
      }
      else if (i % 3 == 0)
      {
        display.drawString(String((i / 3) * 5), 30 + (i * 11.3) + offsetX, 234 + offsetY);
        display.drawFastVLine(30 + (i * 11.3) + offsetX, 224 + offsetY, 2, TFT_FIL_BORDER);
      }
      else
      {
        display.drawString(String("."), 30 + (i * 11.3) + offsetX, 230 + offsetY);
      }
    }
  }
  else
  {
    for (uint8_t i = 0; i <= 12; i++)
    {
      if (i == 12)
      {
        display.drawString(String(" 20A"), 30 + (i * 14) + offsetX, 234 + offsetY);
        display.drawFastVLine(30 + (i * 14) + offsetX, 224 + offsetY, 2, TFT_FIL_BORDER);
      }
      else if (i % 3 == 0)
      {
        display.drawString(String((i / 3) * 5) + offsetX, 30 + (i * 14) + offsetY, 234);
        display.drawFastVLine(30 + (i * 14) + offsetX, 224 + offsetY, 2, TFT_FIL_BORDER);
      }
      else
      {
        display.drawString(String("."), 30 + (i * 14) + offsetX, 230 + offsetY);
      }
    }
  }

  display.drawFastVLine(30 + offsetX, 218 + offsetY, 8, TFT_FIL_BORDER);

  // SWR
  display.setTextDatum(CR_DATUM);
  display.setTextColor(TFT_WHITE);

  display.drawFastHLine(30 + offsetX, 202 + offsetY, 180, TFT_FIL_BORDER);
  display.drawString("SWR", 28 + offsetX, 200 + offsetY);

  display.setTextDatum(CC_DATUM);
  display.setTextColor(TFT_WHITE);

  const char *legendeSWR[] = {"1", "1.5", "2", "2.5", "3"};

  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    display.drawFastVLine(30 + i + offsetX, 194 + offsetY, 8, TFT_FIL_BACK);
  }

  for (uint8_t i = 0; i <= 4; i += 1)
  {
    display.drawString(legendeSWR[i], 30 + (i * 22) + offsetX, 210 + offsetY);
    display.drawFastVLine(30 + (i * 22) + offsetX, 200 + offsetY, 2, TFT_FIL_BORDER);
  }
  display.drawFastVLine(30 + offsetX, 194 + offsetY, 8, TFT_FIL_BORDER);
  display.drawString("o", 201 + offsetX, 208 + offsetY);
  display.drawString("o", 206 + offsetX, 208 + offsetY);

  // COMP
  display.setTextDatum(CR_DATUM);
  display.setTextColor(TFT_WHITE);

  display.drawString("COMP", 28 + offsetX, 174 + offsetY);

  display.setTextDatum(CC_DATUM);
  display.setTextColor(TFT_WHITE);

  const char *legendeCOMP[] = {"0", "5", "10", "15", "20"};

  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    display.drawFastVLine(30 + i + offsetX, 170 + offsetY, 8, TFT_FIL_BACK);
  }

  for (uint8_t i = 0; i <= 4; i += 1)
  {
    display.drawString(legendeCOMP[i], 34 + (i * 34) + offsetX, 174 + offsetY);
  }
  display.drawString("dB", 204 + offsetX, 174 + offsetY);

  // ALC
  display.setTextDatum(CR_DATUM);
  display.setTextColor(TFT_WHITE);

  display.drawFastHLine(30 + offsetX, 154 + offsetY, 90, TFT_RED);
  display.drawString("ALC", 28 + offsetX, 150 + offsetY);

  display.setTextDatum(CC_DATUM);
  display.setTextColor(TFT_WHITE);

  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    display.drawFastVLine(30 + i + offsetX, 146 + offsetY, 8, TFT_FIL_BACK);
  }

  // S and Po
  display.setTextDatum(CR_DATUM);
  display.setTextColor(TFT_WHITE);

  display.drawFastHLine(30 + offsetX, 120 + offsetY, 90, TFT_FIL_BORDER);
  display.drawFastHLine(120 + offsetX, 120 + offsetY, 90, TFT_RED);

  display.drawFastHLine(30 + offsetX, 130 + offsetY, 180, TFT_FIL_BORDER);
  display.drawString("S", 28 + offsetX, 114 + offsetY);
  display.drawString("Po", 28 + offsetX, 138 + offsetY);

  display.setTextDatum(CC_DATUM);
  display.setTextColor(TFT_WHITE);

  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    display.drawFastVLine(30 + i + offsetX, 122 + offsetY, 8, TFT_FIL_BACK);
  }

  for (uint8_t i = 1; i <= 9; i++)
  {
    if (i % 2 != 0)
    {
      display.drawString(String(i), 30 + (i * 10) + offsetX, 114 + offsetY);
    }
    display.drawFastVLine(30 + (i * 10) + offsetX, 119 + offsetY, 2, TFT_FIL_BORDER);
  }

  const char *legendePO[] = {" 0", "25", "50", "", "100%"};

  for (uint8_t i = 0; i <= 4; i++)
  {
    display.drawString(legendePO[i], 30 + (i * 45) + offsetX, 138 + offsetY);
    display.drawFastVLine(30 + (i * 45) + offsetX, 129 + offsetY, 2, TFT_FIL_BORDER);
  }

  display.setTextColor(TFT_RED);

  for (uint8_t i = 20; i <= 60; i += 20)
  {
    if (i % 2 == 0)
    {
      if (i != 60)
      {
        display.drawString("+" + String(i), 120 + (i * 1.5) + offsetX, 114 + offsetY);
      }
      else
      {
        display.drawString("+" + String(i) + "dB", 120 + (i * 1.5) + offsetX, 114 + offsetY);
      }
    }
    display.drawFastVLine(120 + (i * 1.5) + offsetX, 119 + offsetY, 2, TFT_RED);
  }

  display.drawFastVLine(30 + offsetX, 121 + offsetY, 8, TFT_FIL_BORDER);

  // Vd
  display.setTextDatum(CL_DATUM);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.drawRect(220 + offsetX, 208 + offsetY, 100, 32, TFT_FIL_BORDER);
  display.drawString("Vd", 220 + offsetX, 208 + offsetY);

  if (icModel == 705)
  {
    display.drawString("5", 230 + offsetX, 219 + offsetY);
  }
  else
  {
    display.drawString("10", 230 + offsetX, 219 + offsetY);
  }

  display.setTextDatum(CR_DATUM);
  display.drawString("16V", 312 + offsetX, 219 + offsetY);

  display.setTextDatum(CC_DATUM);
  display.setTextColor(TFT_WHITE);

  for (uint8_t i = 0; i <= 80; i += STEP)
  {
    display.drawFastVLine(230 + i + offsetX, 226 + offsetY, 8, TFT_FIL_BACK);
  }

  if (icModel == 705)
  {
    display.drawFastHLine(230 + offsetX, 234 + offsetY, 50, TFT_RED);
    display.drawFastHLine(280 + offsetX, 234 + offsetY, 30, TFT_FIL_BORDER);
  }
  else
  {
    display.drawFastHLine(230 + offsetX, 234 + offsetY, 25, TFT_RED);
    display.drawFastHLine(255 + offsetX, 234 + offsetY, 55, TFT_FIL_BORDER);
  }
}

// Clear GUI
void clearGUI()
{
  // Power
  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    display.drawFastVLine(30 + i + offsetX, 122 + offsetY, 8, TFT_FIL_BACK);
  }

  for (uint8_t i = 0; i <= 4; i++)
  {
    display.drawFastVLine(30 + (i * 45) + offsetX, 129 + offsetY, 2, TFT_FIL_BORDER);
  }

  // SWR
  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    display.drawFastVLine(30 + i + offsetX, 194 + offsetY, 8, TFT_FIL_BACK);
  }

  for (uint8_t i = 0; i <= 4; i += 1)
  {
    display.drawFastVLine(30 + (i * 22) + offsetX, 200 + offsetY, 2, TFT_FIL_BORDER);
  }

  // Id
  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    display.drawFastVLine(30 + i + offsetX, 218 + offsetY, 8, TFT_FIL_BACK);
  }

  if(icModel == 705 || icModel == 9700)
  {
    for (uint8_t i = 0; i <= 12; i++)
    {
      if (i % 3 == 0)
      {
        display.drawFastVLine(30 + (i * 14) + offsetX, 224 + offsetY, 2, TFT_FIL_BORDER);
      }
    }
  }
  else
  {
    for (uint8_t i = 0; i <= 15; i++)
    {
      if (i % 3 == 0)
      {
        display.drawFastVLine(30 + (i * 11.3) + offsetX, 224 + offsetY, 2, TFT_FIL_BORDER);
      }
    }
  }

  // COMP
  const char *legendeCOMP[] = {"0", "5", "10", "15", "20"};

  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    display.drawFastVLine(30 + i + offsetX, 170 + offsetY, 8, TFT_FIL_BACK);
  }

  display.setFont(0);
  display.setTextDatum(CC_DATUM);
  display.setTextColor(TFT_WHITE);

  for (uint8_t i = 0; i <= 4; i += 1)
  {
    display.drawString(legendeCOMP[i], 34 + (i * 34) + offsetX, 174 + offsetY);
  }
  display.drawString("dB", 204 + offsetX, 174 + offsetY);
}

// Clear Data
void clearData()
{
  sOld = 255;
  SWROld = 255;
  powerOld = 255;
  IdOld = 255;
  COMPLOld = 255;
  ALCOld = 127;
  TXOld = 127;
  agcOld = 127;
  ANOld = 127;
  NBOld = 127;
  NROld = 127;
  AMPOld = 127;
  toneOld = 127;
  AFOld = 127;
  MICOld = 127;
  SQLOld = 127;
  COMPOld = 127;
  NRLevelOld = 127;
  NBLevelOld = 127;
  VdOld = 0;
  IPOld = 0;
  batteryLevelOld = 0;

  frequencyOld = "";
  filterOld = "";
  modeOld = "";
  RITOld = "";

  batteryCharginglOld = true;
}

// Print value
void value(char* valString, uint8_t x = 160, uint8_t y = 76)
{
  if (strcmp(valString, valStringOld) != 0)
  {
    strncpy(valStringOld, valString, 32);

    display.setTextDatum(CC_DATUM);
    display.setFont(&UniversCondensed20pt7b);
    display.setTextPadding(194);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.drawString(valString, x + offsetX, y + offsetY);
  }
}


// Get 24bits BMP
bool M5Screen24bmp()
{
  uint16_t image_height = display.height();
  uint16_t image_width = display.width();
  const uint16_t pad = (4 - (3 * image_width) % 4) % 4;
  uint32_t filesize = 54 + (3 * image_width + pad) * image_height;
  unsigned char swap;
  unsigned char line_data[image_width * 3 + pad];
  unsigned char header[54] = {
      'B', 'M',    // BMP signature (Windows 3.1x, 95, NT, …)
      0, 0, 0, 0,  // Image file size in bytes
      0, 0, 0, 0,  // Reserved
      54, 0, 0, 0, // Start of pixel array
      40, 0, 0, 0, // Info header size
      0, 0, 0, 0,  // Image width
      0, 0, 0, 0,  // Image height
      1, 0,        // Number of color planes
      24, 0,       // Bits per pixel
      0, 0, 0, 0,  // Compression
      0, 0, 0, 0,  // Image size (can be 0 for uncompressed images)
      0, 0, 0, 0,  // Horizontal resolution (dpm)
      0, 0, 0, 0,  // Vertical resolution (dpm)
      0, 0, 0, 0,  // Colors in color table (0 = none)
      0, 0, 0, 0}; // Important color count (0 = all colors are important)

  // Fill filesize, width and heigth in the header array
  for (uint8_t i = 0; i < 4; i++)
  {
    header[2 + i] = (char)((filesize >> (8 * i)) & 255);
    header[18 + i] = (char)((image_width >> (8 * i)) & 255);
    header[22 + i] = (char)((image_height >> (8 * i)) & 255);
  }
  // Write the header to the file
  httpClient.write(header, 54);

  // To keep the required memory low, the image is captured line by line
  // initialize padded pixel with 0
  for (uint16_t i = (image_width - 1) * 3; i < (image_width * 3 + pad); i++)
  {
    line_data[i] = 0;
  }
  // The coordinate origin of a BMP image is at the bottom left.
  // Therefore, the image must be read from bottom to top.
  for (uint16_t y = image_height; y > 0; y--)
  {
    // Get one line of the screen content
    display.readRectRGB(0, y - 1, image_width, 1, line_data);
    // BMP color order is: Blue, Green, Red
    // Return values from readRectRGB is: Red, Green, Blue
    // Therefore: R und B need to be swapped
    for (uint16_t x = 0; x < image_width; x++)
    {
      swap = line_data[x * 3];
      line_data[x * 3] = line_data[x * 3 + 2];
      line_data[x * 3 + 2] = swap;
    }
    // Write the line to the file
    httpClient.write(line_data, (image_width * 3) + pad);
  }
  return true;
}

// Get screenshot
void getScreenshot()
{
  unsigned long timeout_millis = millis() + 3000;
  String currentLine = "";

  httpClient = httpServer.available();

  if (WiFi.status() == WL_CONNECTED)
  {
    // httpClient.setNoDelay(1);
    if (httpClient)
    {
      // Force a disconnect after 3 seconds
      // Serial.println("New Client.");
      // Loop while the client's connected
      while (httpClient.connected())
      {
        // If the client is still connected after 3 seconds,
        // Something is wrong. So kill the connection
        if (millis() > timeout_millis)
        {
          // Serial.println("Force Client stop!");
          httpClient.stop();
        }
        // If there's bytes to read from the client,
        if (httpClient.available())
        {
          char c = httpClient.read();
          Serial.write(c);
          // If the byte is a newline character
          if (c == '\n')
          {
            // Two newline characters in a row (empty line) are indicating
            // The end of the client HTTP request, so send a response:
            if (currentLine.length() == 0)
            {
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line,
              // followed by the content:

              screensaverTimer = millis(); // Screensaver update !!!

              switch (htmlGetRequest)
              {
              case GET_index_page:
              {
                httpClient.println("HTTP/1.1 200 OK");
                httpClient.println("Content-type:text/html");
                httpClient.println();
                if (M5.getBoard() == m5::board_t::board_M5Stack)
                {
                  httpClient.write_P(index_m5stack_html, sizeof(index_m5stack_html));
                }
                else if (M5.getBoard() == m5::board_t::board_M5StackCore2)
                {
                  httpClient.write_P(index_core2_html, sizeof(index_core2_html));
                }
                break;
              }
              case GET_screenshot:
              {
                httpClient.println("HTTP/1.1 200 OK");
                httpClient.println("Content-type:image/bmp");
                httpClient.println();
                M5Screen24bmp();
                break;
              }
              default:
                httpClient.println("HTTP/1.1 404 Not Found");
                httpClient.println("Content-type:text/html");
                httpClient.println();
                httpClient.print("404 Page not found.<br>");
                break;
              }
              // The HTTP response ends with another blank line:
              // httpClient.println();
              // Break out of the while loop:
              break;
            }
            else
            { // if a newline is found
              // Analyze the currentLine:
              // detect the specific GET requests:
              if (currentLine.startsWith("GET /"))
              {
                htmlGetRequest = GET_unknown;
                // If no specific target is requested
                if (currentLine.startsWith("GET / "))
                {
                  htmlGetRequest = GET_index_page;
                }
                // If the screenshot image is requested
                if (currentLine.startsWith("GET /screenshot.bmp"))
                {
                  htmlGetRequest = GET_screenshot;
                }
                // If the button left was pressed on the HTML page
                if (currentLine.startsWith("GET /buttonLeft"))
                {
                  buttonLeftPressed = true;
                  htmlGetRequest = GET_index_page;
                }
                // If the button center was pressed on the HTML page
                if (currentLine.startsWith("GET /buttonCenter"))
                {
                  buttonCenterPressed = true;
                  htmlGetRequest = GET_index_page;
                }
                // If the button right was pressed on the HTML page
                if (currentLine.startsWith("GET /buttonRight"))
                {
                  buttonRightPressed = true;
                  htmlGetRequest = GET_index_page;
                }
              }
              currentLine = "";
            }
          }
          else if (c != '\r')
          {
            // Add anything else than a carriage return
            // character to the currentLine
            currentLine += c;
          }
        }
      }
      // Close the connection
      httpClient.stop();
      // Serial.println("Client Disconnected.");
    }
  }
}

// Manage screensaver
void wakeAndSleep()
{
  static uint16_t x = rand() % 232;
  static uint16_t y = rand() % 196;
  static boolean xDir = rand() & 1;
  static boolean yDir = rand() & 1;

  if (screensaverMode == false && millis() - screensaverTimer > screensaver * 60 * 1000)
  {
    settingsMode = false;
    screensaverMode = true;
    screensaverTimer = 0;
    display.fillScreen(TFT_BLACK);
  }
  else if (screensaverMode == true && screensaverTimer != 0)
  {
    display.fillScreen(TFT_BLACK);
    clearData();
    viewGUI();
    screensaverMode = false;
    settingsMode = false;

    vTaskDelay(100);
  }
  else if (screensaverMode == true)
  {
    display.fillRect(x + offsetX, y + offsetY, 44, 22, TFT_BLACK);

    if (xDir)
    {
      x += 1;
    }
    else
    {
      x -= 1;
    }

    if (yDir)
    {
      y += 1;
    }
    else
    {
      y -= 1;
    }

    if (x < 44)
    {
      xDir = true;
      x = 44;
    }
    else if (x > 232)
    {
      xDir = false;
      x = 232;
    }

    if (y < 22)
    {
      yDir = true;
      y = 22;
    }
    else if (y > 196)
    {
      yDir = false;
      y = 196;
    }

    logoSprite.pushSprite(x + offsetX, y + offsetY, TFT_TRANSPARENT);

    if (icModel == 705 && icConnect == BT && btConnected == false)
      vTaskDelay(75);
    else if (icConnect == USB && wifiConnected == false)
      vTaskDelay(75);
  }

  // Debug trace
  if (DEBUG == 1)
  {
    Serial.printf("%d %ld\n", screensaverMode, millis() - screensaverTimer);
  }
}

// Manage connexion error
boolean checkConnection()
{
  HTTPClient http;

  uint16_t httpCode;

  char message[24] = "";
  String command = "";
  String response = "";

  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x03, 0xFD};

  char s[4];

  settingLock = false;

  for (uint8_t i = 0; i < 6; i++)
  {
    sprintf(s, "%02x,", request[i]);
    command += String(s);
  }

  command += BAUD_RATE + String(",") + icSerialDevice;

  if (screenshot == false)
  {
    if(btClient)
    {
      btConnected = true;
    }
    else 
    {
      btConnected = false;
    }

    if (icModel == 705 && icConnect == BT && btConnected == false)
      snprintf(message, 24, "%s", "Need Pairing");
    else if (icConnect == USB && wifiConnected == false)
      snprintf(message, 24, "%s", "Check Wifi");
    else if (icConnect == USB && (proxyConnected == false || txConnected == false))
    {
      http.begin(civClient, PROXY_URL + String(":") + PROXY_PORT + String("/") + String("?civ=") + command); // Specify the URL
      http.addHeader("User-Agent", "M5Stack");                                                               // Specify header
      http.addHeader("Connection", "keep-alive");                                                            // Specify header
      http.setTimeout(100);                                                                                  // Set Time Out
      httpCode = http.GET();                                                                                 // Make the request
      if (httpCode == 200)
      {
        proxyConnected = true;

        response = http.getString(); // Get data
        response.trim();

        if (response != "")
        {
          Serial.println("TX connected");
          txConnected = true;
          snprintf(message, 24, "%s", "");
        }
        else
        {
          Serial.println("TX disconnected");
          txConnected = false;
          snprintf(message, 24, "%s %lu", "Check", icModel);
        }
      }
      else
      {
        snprintf(message, 24, "%s", "Check Proxy");
      }
      http.end();
    }

    // Shutdown screen if no TX connexion
    if (wakeup == true && startup == false)
    {
      if ((icConnect == BT && btConnected == false) || (icConnect == USB && txConnected == false))
      {
        display.sleep();
        wakeup = false;
      }
    }
    else if (wakeup == false && startup == false)
    {
      if ((icConnect == BT && btConnected == true) || (icConnect == USB && txConnected == true))
      {
        clearData();
        viewGUI();
        display.wakeup();
        wakeup = true;
        screensaverTimer = millis();
      }
    }

    // View message

    if (strcmp(message, "") != 0)
    {
      settingLock = true;

      if (screensaverMode == false && settingsMode == false)
      {
        value(message);
        vTaskDelay(750);
        snprintf(message, 24, "%s", "");
        value(message);
        frequencyOld = "";
        settingLock = false;
        vTaskDelay(250);
        return false;
      }
      else
      {
        settingLock = false;
        vTaskDelay(1000);
        return false;
      }
    }
  }
  return true;
}

// Manage voice
void voiceManager(uint8_t tx, uint8_t alternance)
{
  if (DEBUG)
  {
    Serial.print(voice);
    Serial.print("-");
    Serial.print(voiceMode);
    Serial.print("-");
    Serial.println(voiceCounter);
  }

  display.setFont(&tahoma8pt7b);
  display.setTextPadding(24);
  display.setTextDatum(CC_DATUM);

  if (voice == 0)
  {
    display.fillRect(32 + offsetX, 2 + offsetY, 28, 18, TFT_BLACK);
    return;
  }
  else
  {
    if (voiceMode == 2)
    {
      if (tx == 0)
      {
        sendVoice();
        voiceMode--;
        voiceCounter--;
        voiceRefresh = true;
      }
      else
      {
        if (voiceRefresh == true)
        {
          display.fillRoundRect(32 + offsetX, 2 + offsetY, 28, 18, 2, TFT_BLACK);
          display.drawRoundRect(32 + offsetX, 2 + offsetY, 28, 18, 2, TFT_RED);
          display.setTextColor(TFT_RED);
          display.drawString("T" + String(voice), 45 + offsetX, 12 + offsetY);
          voiceRefresh = false;
        }
      }
    }
    else if (voiceMode == 1)
    {
      if (tx == 1)
      {
        transmit = millis();
      }
      else
      {
        if (voiceCounter == 0)
        {
          voiceMode = 0;

          if (voiceRefresh == true)
          {
            display.fillRoundRect(32 + offsetX, 2 + offsetY, 28, 18, 2, TFT_BLACK);
            display.drawRoundRect(32 + offsetX, 2 + offsetY, 28, 18, 2, TFT_RED);
            display.setTextColor(TFT_RED);
            display.drawString("T" + String(voice), 45 + offsetX, 12 + offsetY);
            voiceRefresh = false;
          }
        }
        else if (millis() - transmit > voiceTimeout * 1000)
        {
          sendVoice();
          voiceCounter--;
          voiceRefresh = true;
        }
        else if (millis() - transmit < voiceTimeout * 1000)
        {
          uint8_t value = voiceTimeout - int((millis() - transmit) / 1000);

          display.fillRoundRect(32 + offsetX, 2 + offsetY, 28, 18, 2, TFT_RED);
          display.drawRoundRect(32 + offsetX, 2 + offsetY, 28, 18, 2, TFT_WHITE);
          display.setTextColor(TFT_WHITE);
          display.drawString(String(value) + "s", 45 + offsetX, 12 + offsetY);
        }
      }
    }
    else
    {
      if (voiceRefresh == true)
      {
        display.fillRoundRect(32 + offsetX, 2 + offsetY, 28, 18, 2, TFT_BLACK);
        display.drawRoundRect(32 + offsetX, 2 + offsetY, 28, 18, 2, TFT_RED);
        display.setTextColor(TFT_RED);
        display.drawString("T" + String(voice), 45 + offsetX, 12 + offsetY);
        voiceRefresh = false;
      }
    }
  }
}