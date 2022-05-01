// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Bluetooth callback
void callbackBT(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  if (event == ESP_SPP_SRV_OPEN_EVT)
  {
    screensaver = millis();
    M5.Lcd.wakeup();
    Serial.println("BT Client Connected");
    frequencyOld = "";
    btConnected = true;
  }
  if (event == ESP_SPP_CLOSE_EVT)
  {
    M5.Lcd.sleep();
    Serial.println("BT Client disconnected");
    btConnected = false;
  }
}

// Wifi callback Off
void callbackWifiOff(WiFiEvent_t event, WiFiEventInfo_t info)
{
  //M5.Lcd.sleep();
  Serial.println("Wifi Client disconnected");
  wifiConnected = false;

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

// Wifi callback On
void callbackWifiOn(WiFiEvent_t event, WiFiEventInfo_t info)
{
  screensaver = millis();
  //M5.Lcd.wakeup();
  Serial.println("Wifi Client Connected");
  frequencyOld = "";
  wifiConnected = true;
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

    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(2);

    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.drawString("Flash File System", 160, 20);
    M5.Lcd.drawString("needs to be formated.", 160, 50);
    M5.Lcd.drawString("It takes around 4 minutes.", 160, 100);
    M5.Lcd.drawString("Please, wait until ", 160, 150);
    M5.Lcd.drawString("the application starts !", 160, 180);

    Serial.println("SPIFFS Formating...");

    SPIFFS.format(); // Format SPIFFS...

    M5.Lcd.setTextFont(0);
    M5.Lcd.setTextSize(0);

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
    M5.Lcd.qrcode("https://github.com/armel/ICMultiMeter", 90, 80, 140, 6);

    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(1);

    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CC_DATUM);

    for (uint8_t i = TIMEOUT_BIN_LOADER * 10; i > 0; i--)
    {
      getButton();

      if (i % 10 == 0)
      {
        tmpName += ".";
        M5.Lcd.drawString(tmpName, 160, 20);
      }

      if (btnA || btnC)
      {
        SD.end(); // If not Bluetooth doesn't work !!!
        return;
      }
      else if (btnB)
      {
        click = 1;
        M5.Lcd.fillRect(0, 0, 320, 240, TFT_BLACK);

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

    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(2);

    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.drawString("Bin Loader V0.2", 160, 20);

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
      M5.Lcd.setTextPadding(320);

      uint8_t i = 0;
      for (uint8_t j = (start * limit); j < stop; j++)
      {
        tmpName = binFilename[j].substring(4);

        if (cursor == j)
        {
          tmpName = ">> " + tmpName + " <<";

          if (binFilename[cursor].substring(0, 4) == "SP_/")
          {
            M5.Lcd.setTextSize(1);
            M5.Lcd.drawString("SPI Flash File Storage", 160, 50);
          }
          else
          {
            M5.Lcd.setTextSize(1);
            M5.Lcd.drawString("SD Card Storage", 160, 50);
          }
        }

        M5.Lcd.setTextSize(2);
        M5.Lcd.drawString(tmpName, 160, 80 + i * 20);
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

      M5.Lcd.drawRect(294, 4, 20, 12, TFT_WHITE);
      M5.Lcd.drawRect(313, 7, 4, 6, TFT_WHITE);
      M5.Lcd.fillRect(296, 6, 16, 8, TFT_BLACK);
      M5.Lcd.fillRect(296, 6, batteryLevel, 8, TFT_WHITE);

      if (batteryCharging)
      {
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Lcd.setFont(0);
        M5.Lcd.setTextDatum(CR_DATUM);
        M5.Lcd.setTextPadding(28);
        M5.Lcd.drawString("+", 290, 11);
      }
      else
      {
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Lcd.setFont(0);
        M5.Lcd.setTextDatum(CR_DATUM);
        M5.Lcd.setTextPadding(28);
        M5.Lcd.drawString(String(getBatteryLevel(1)) + "%", 290, 11);
      }
    }
  }
}

// View GUI
void viewGUI()
{
  // Clear
  M5.Lcd.fillRect(0, 0, 320, 240, TFT_BLACK);
  
  // IC Connect

  M5.Lcd.setFont(0);
  M5.Lcd.setTextDatum(CC_DATUM);

  M5.Lcd.fillRoundRect(261, 90, 56, 13, 2, TFT_MODE_BACK);
  M5.Lcd.drawRoundRect(261, 90, 56, 13, 2, TFT_MODE_BORDER);
  M5.Lcd.setTextColor(TFT_WHITE);

  if (IC_CONNECT == BT)
    M5.Lcd.drawString(String(IC_MODEL) + " BT", 289, 97);
  else
    M5.Lcd.drawString(String(IC_MODEL) + " USB", 289, 97);

  if (transverter == 1) {
    M5.Lcd.fillRoundRect(4, 90, 24, 13, 2, TFT_MODE_BACK);
    M5.Lcd.drawRoundRect(4, 90, 24, 13, 2, TFT_MODE_BORDER);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.drawString("LO", 16, 97);
  }

  /*
  M5.Lcd.setFont(0);
  M5.Lcd.setTextDatum(CC_DATUM);

  M5.Lcd.fillRoundRect(283, 90, 34, 20, 2, TFT_MODE_BACK);
  M5.Lcd.drawRoundRect(283, 90, 34, 20, 2, TFT_MODE_BORDER);
  M5.Lcd.setTextColor(TFT_WHITE);

  if (IC_CONNECT == BT)
    M5.Lcd.drawString("BT", 300, 96);
  else
    M5.Lcd.drawString("USB", 300, 96);

  M5.Lcd.drawString(String(IC_MODEL), 300, 105);

  // M5.Lcd.drawFastHLine(0, 57, 320, TFT_FIL_BORDER);
  // M5.Lcd.drawFastHLine(0, 86, 320, TFT_FIL_BORDER);

  // M5.Lcd.drawJpg(logo, sizeof(logo), 272, 0, 44, 22);
  // M5.Lcd.drawJpg(logo, sizeof(logo), 40, 0, 44, 22);
  // M5.Lcd.drawJpg(logo, sizeof(logo), 272, 94, 44, 22);
  */

  M5.Lcd.drawJpg(logo, sizeof(logo), 0, 49, 44, 22);

  M5.Lcd.setFont(0);
  M5.Lcd.setTextPadding(0);

  // Id
  M5.Lcd.setTextDatum(CR_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE);

  M5.Lcd.drawFastHLine(30, 226, 180, TFT_FIL_BORDER);
  M5.Lcd.drawString("Id", 28, 223);

  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE);

  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    M5.Lcd.drawFastVLine(30 + i, 218, 8, TFT_FIL_BACK);
  }

  for (uint8_t i = 0; i <= 12; i++)
  {
    if (i == 12)
    {
      M5.Lcd.drawString(String(" 4 A"), 30 + (i * 14), 234);
      M5.Lcd.drawFastVLine(30 + (i * 14), 224, 2, TFT_FIL_BORDER);
    }
    else if (i % 3 == 0)
    {
      M5.Lcd.drawString(String(i / 3), 30 + (i * 14), 234);
      M5.Lcd.drawFastVLine(30 + (i * 14), 224, 2, TFT_FIL_BORDER);
    }
    else
    {
      M5.Lcd.drawString(String("."), 30 + (i * 14), 230);
    }
  }
  M5.Lcd.drawFastVLine(30, 218, 8, TFT_FIL_BORDER);

  // SWR
  M5.Lcd.setTextDatum(CR_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE);

  M5.Lcd.drawFastHLine(30, 202, 180, TFT_FIL_BORDER);
  M5.Lcd.drawString("SWR", 28, 200);

  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE);

  const char *legendeSWR[] = {"1", "1.5", "2", "2.5", "3"};

  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    M5.Lcd.drawFastVLine(30 + i, 194, 8, TFT_FIL_BACK);
  }

  for (uint8_t i = 0; i <= 4; i += 1)
  {
    M5.Lcd.drawString(legendeSWR[i], 30 + (i * 22), 210);
    M5.Lcd.drawFastVLine(30 + (i * 22), 200, 2, TFT_FIL_BORDER);
  }
  M5.Lcd.drawFastVLine(30, 194, 8, TFT_FIL_BORDER);
  M5.Lcd.drawString("o", 201, 208);
  M5.Lcd.drawString("o", 206, 208);

  // COMP
  M5.Lcd.setTextDatum(CR_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE);

  M5.Lcd.drawString("COMP", 28, 174);

  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE);

  const char *legendeCOMP[] = {"0", "5", "10", "15", "20"};

  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    M5.Lcd.drawFastVLine(30 + i, 170, 8, TFT_FIL_BACK);
  }

  for (uint8_t i = 0; i <= 4; i += 1)
  {
    M5.Lcd.drawString(legendeCOMP[i], 34 + (i * 34), 174);
  }
  M5.Lcd.drawString("dB", 204, 174);

  // ALC
  M5.Lcd.setTextDatum(CR_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE);

  M5.Lcd.drawFastHLine(30, 154, 90, TFT_RED);
  M5.Lcd.drawString("ALC", 28, 150);

  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE);

  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    M5.Lcd.drawFastVLine(30 + i, 146, 8, TFT_FIL_BACK);
  }

  // S and Po
  M5.Lcd.setTextDatum(CR_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE);

  M5.Lcd.drawFastHLine(30, 120, 90, TFT_FIL_BORDER);
  M5.Lcd.drawFastHLine(120, 120, 90, TFT_RED);

  M5.Lcd.drawFastHLine(30, 130, 180, TFT_FIL_BORDER);
  M5.Lcd.drawString("S", 28, 114);
  M5.Lcd.drawString("Po", 28, 138);

  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE);

  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    M5.Lcd.drawFastVLine(30 + i, 122, 8, TFT_FIL_BACK);
  }

  for (uint8_t i = 1; i <= 9; i++)
  {
    if (i % 2 != 0)
    {
      M5.Lcd.drawString(String(i), 30 + (i * 10), 114);
    }
    M5.Lcd.drawFastVLine(30 + (i * 10), 119, 2, TFT_FIL_BORDER);
  }

  const char *legendePO[] = {" 0", "25", "50", "", "100%"};

  for (uint8_t i = 0; i <= 4; i++)
  {
    M5.Lcd.drawString(legendePO[i], 30 + (i * 45), 138);
    M5.Lcd.drawFastVLine(30 + (i * 45), 129, 2, TFT_FIL_BORDER);
  }

  M5.Lcd.setTextColor(TFT_RED);

  for (uint8_t i = 20; i <= 60; i += 20)
  {
    if (i % 2 == 0)
    {
      if (i != 60)
      {
        M5.Lcd.drawString("+" + String(i), 120 + (i * 1.5), 114);
      }
      else
      {
        M5.Lcd.drawString("+" + String(i) + "dB", 120 + (i * 1.5), 114);
      }
    }
    M5.Lcd.drawFastVLine(120 + (i * 1.5), 119, 2, TFT_RED);
  }

  M5.Lcd.drawFastVLine(30, 121, 8, TFT_FIL_BORDER);

  // Vd
  M5.Lcd.setTextDatum(CL_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawRect(220, 200, 100, 40, TFT_FIL_BORDER);
  M5.Lcd.drawString("Vd", 220, 200);

  if(IC_MODEL == 705) {
    M5.Lcd.drawString("5", 230, 216);
  }
  else {
    M5.Lcd.drawString("10", 230, 216);
  }

  M5.Lcd.setTextDatum(CR_DATUM);
  M5.Lcd.drawString("16V", 312, 216);

  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE);

  for (uint8_t i = 0; i <= 80; i += STEP)
  {
    M5.Lcd.drawFastVLine(230 + i, 224, 8, TFT_FIL_BACK);
  }

  if(IC_MODEL == 705) {
    M5.Lcd.drawFastHLine(230, 232, 50, TFT_RED);
    M5.Lcd.drawFastHLine(280, 232, 30, TFT_FIL_BORDER);
  }
  else {
    M5.Lcd.drawFastHLine(230, 232, 25, TFT_RED);
    M5.Lcd.drawFastHLine(255, 232, 55, TFT_FIL_BORDER);
  }

  // AF Gain & SQL
  // M5.Lcd.fillRect(220, 146, 100, 45, TFT_BLACK);
  // M5.Lcd.drawRect(220, 146, 100, 45, TFT_FIL_BORDER);

  M5.Lcd.fillRect(230, 135, 60, 10, TFT_BLACK);
  M5.Lcd.fillRect(230, 157, 60, 10, TFT_BLACK);
  M5.Lcd.fillRect(230, 179, 60, 10, TFT_BLACK);

  M5.Lcd.drawRect(230, 135, 60, 10, TFT_FIL_BORDER);
  M5.Lcd.drawRect(230, 157, 60, 10, TFT_FIL_BORDER);
  M5.Lcd.drawRect(230, 179, 60, 10, TFT_FIL_BORDER);

  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setTextDatum(CL_DATUM);
  M5.Lcd.drawString("AF GAIN", 230, 130);

  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setTextDatum(CL_DATUM);
  M5.Lcd.drawString("MIC GAIN", 230, 152);

  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setTextDatum(CL_DATUM);
  M5.Lcd.drawString("SQL", 230, 174);
}

// Clear GUI
void clearGUI()
{
  // Power
  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    M5.Lcd.drawFastVLine(30 + i, 122, 8, TFT_FIL_BACK);
  }

  for (uint8_t i = 0; i <= 4; i++)
  {
    M5.Lcd.drawFastVLine(30 + (i * 45), 129, 2, TFT_FIL_BORDER);
  }

  // SWR
  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    M5.Lcd.drawFastVLine(30 + i, 194, 8, TFT_FIL_BACK);
  }

  for (uint8_t i = 0; i <= 4; i += 1)
  {
    M5.Lcd.drawFastVLine(30 + (i * 22), 200, 2, TFT_FIL_BORDER);
  }

  // Id
  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    M5.Lcd.drawFastVLine(30 + i, 218, 8, TFT_FIL_BACK);
  }

  for (uint8_t i = 0; i <= 12; i++)
  {
    if (i % 3 == 0)
    {
      M5.Lcd.drawFastVLine(30 + (i * 14), 224, 2, TFT_FIL_BORDER);
    }
  }

  // COMP
  const char *legendeCOMP[] = {"0", "5", "10", "15", "20"};

  for (uint8_t i = 0; i <= 180; i += STEP)
  {
    M5.Lcd.drawFastVLine(30 + i, 170, 8, TFT_FIL_BACK);
  }

  M5.Lcd.setFont(0);
  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE);

  for (uint8_t i = 0; i <= 4; i += 1)
  {
    M5.Lcd.drawString(legendeCOMP[i], 34 + (i * 34), 174);
  }
  M5.Lcd.drawString("dB", 204, 174);
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
  VdOld = 0;
  IPOld = 0;
  batteryLevelOld = 0;

  frequencyOld = "";
  filterOld = "";
  modeOld = "";
  RITOld = "";

  batteryCharginglOld = true;
}

// Get 24bits BMP
bool M5Screen24bmp()
{
  uint16_t image_height = M5.Lcd.height();
  uint16_t image_width = M5.Lcd.width();
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
    M5.Lcd.readRectRGB(0, y - 1, image_width, 1, line_data);
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

              screensaver = millis(); // Screensaver update !!!

              switch (htmlGetRequest)
              {
              case GET_index_page:
              {
                httpClient.println("HTTP/1.1 200 OK");
                httpClient.println("Content-type:text/html");
                httpClient.println();
                if(M5.getBoard() == m5::board_t::board_M5Stack) {
                  httpClient.write_P(index_m5stack_html, sizeof(index_m5stack_html));
                }
                else if(M5.getBoard() == m5::board_t::board_M5StackCore2) {
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

  if (screensaverMode == false && millis() - screensaver > TIMEOUT_SCREENSAVER)
  {
    settingsMode = false;
    screensaverMode = true;
    screensaver = 0;
    M5.Lcd.fillScreen(TFT_BLACK);
  }
  else if (screensaverMode == true && screensaver != 0)
  {
    M5.Lcd.fillScreen(TFT_BLACK);
    clearData();
    viewGUI();
    screensaverMode = false;
    settingsMode = false;

    vTaskDelay(100);
  }
  else if (screensaverMode == true)
  {

    M5.Lcd.fillRect(x, y, 44, 22, TFT_BLACK);

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

    M5.Lcd.drawJpg(logo, sizeof(logo), x, y, 44, 22);

    if (IC_MODEL == 705 && IC_CONNECT == BT && btConnected == false)
      vTaskDelay(75);
    else if (IC_CONNECT == USB && wifiConnected == false)
      vTaskDelay(75);
  }

  if (DEBUG)
  {
    Serial.print(screensaverMode);
    Serial.print(" ");
    Serial.println(millis() - screensaver);
  }
}

// Manage connexion error
boolean checkConnection()
{
  HTTPClient http;

  uint16_t httpCode;

  String message = "";
  String command = "";
  String response = "";

  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x03, 0xFD};
  char s[4];
  
  settingLock = false;

  for (uint8_t i = 0; i < 6; i++)
  {
    sprintf(s, "%02x,", request[i]);
    command += String(s);
  }

  command += BAUDE_RATE + String(",") + SERIAL_DEVICE;

  if (screenshot == false)
  {
    if (IC_MODEL == 705 && IC_CONNECT == BT && btConnected == false)
      message = "Check Pairing";
    else if (IC_CONNECT == USB && wifiConnected == false)
      message = "Check Wifi";
    else if (IC_CONNECT == USB && (proxyConnected == false || txConnected == false))
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
          if(startup == false)
          {
            clearData();
            screensaver = millis();
            M5.Lcd.wakeup();
            Serial.println("TX connected");
          }

          txConnected = true;
          message = "";
        }
        else
        {
          if(startup == false)
          {
            M5.Lcd.sleep();
            Serial.println("TX disconnected");
          }

          message = "Check TX";
        }
      }
      else
      {
        message = "Check Proxy";
      }
      http.end();
    }

    settingLock = true;

    if (message != "")
    {
      if(screensaverMode == false && settingsMode == false)
      {
        M5.Lcd.setTextDatum(CC_DATUM);
        M5.Lcd.setFont(&UniversCondensed20pt7b);
        M5.Lcd.setTextPadding(200);
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Lcd.drawString(message, 160, 70);
        vTaskDelay(750);
        M5.Lcd.drawString("", 160, 70);
        vTaskDelay(250);
        frequencyOld = "";
        return false;
      }
      else {
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
  if(DEBUG) 
  {
    Serial.print(voice);
    Serial.print("-");
    Serial.print(voiceMode);
    Serial.print("-");
    Serial.println(voiceCounter);
  }

  M5.Lcd.setFont(&tahoma8pt7b);
  M5.Lcd.setTextPadding(24);
  M5.Lcd.setTextDatum(CC_DATUM);

  if(voice == 0)
  {
    M5.Lcd.fillRect(32, 2, 28, 18, TFT_BLACK);
    return;
  }
  else {
    if(voiceMode == 2) {
      if(tx == 0) {
        sendVoice();
        voiceMode--;
        voiceCounter--;
        voiceRefresh = true;
      }
      else {
        if(voiceRefresh == true) {
          M5.Lcd.fillRoundRect(32, 2, 28, 18, 2, TFT_BLACK);
          M5.Lcd.drawRoundRect(32, 2, 28, 18, 2, TFT_RED);
          M5.Lcd.setTextColor(TFT_RED);
          M5.Lcd.drawString("T" + String(voice), 45, 12);
          voiceRefresh = false;
        }
      }
    }
    else if(voiceMode == 1) {
      if(tx == 1) {
        transmit = millis();
      }
      else 
      {
        if(voiceCounter == 0) {
          voiceMode = 0;

          if(voiceRefresh == true) {
            M5.Lcd.fillRoundRect(32, 2, 28, 18, 2, TFT_BLACK);
            M5.Lcd.drawRoundRect(32, 2, 28, 18, 2, TFT_RED);
            M5.Lcd.setTextColor(TFT_RED);
            M5.Lcd.drawString("T" + String(voice), 45, 12);
            voiceRefresh = false;
          }
        }
        else if(millis() - transmit > voiceTimeout * 1000) {
          sendVoice();
          voiceCounter--;
          voiceRefresh = true;
        }
        else if(millis() - transmit < voiceTimeout * 1000) {
            uint8_t value = voiceTimeout - int((millis() - transmit) / 1000);

            M5.Lcd.fillRoundRect(32, 2, 28, 18, 2, TFT_RED);
            M5.Lcd.drawRoundRect(32, 2, 28, 18, 2, TFT_WHITE);
            M5.Lcd.setTextColor(TFT_WHITE);
            M5.Lcd.drawString(String(value) + "s", 45, 12);  
        }
      }        
    }
    else {
      if(voiceRefresh == true) {
        M5.Lcd.fillRoundRect(32, 2, 28, 18, 2, TFT_BLACK);
        M5.Lcd.drawRoundRect(32, 2, 28, 18, 2, TFT_RED);
        M5.Lcd.setTextColor(TFT_RED);
        M5.Lcd.drawString("T" + String(voice), 45, 12);
        voiceRefresh = false;
      }
    }
  }
}