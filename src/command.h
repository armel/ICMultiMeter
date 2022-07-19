// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Send CI-V Command by Bluetooth
void sendCommandBt(char *request, size_t n, char *buffer, uint8_t limit, boolean sendOnly = false)
{
  uint8_t byte1, byte2, byte3;
  uint8_t counter = 0;

  while (counter != limit)
  {
    for (uint8_t i = 0; i < n; i++)
    {
      serialBT.write(request[i]);
    }

    vTaskDelay(50);

    if(sendOnly == true) return;

    while (serialBT.available())
    {
      byte1 = serialBT.read();
      byte2 = serialBT.read();

      if (byte1 == 0xFE && byte2 == 0xFE)
      {
        counter = 0;
        byte3 = serialBT.read();
        while (byte3 != 0xFD)
        {
          buffer[counter] = byte3;
          byte3 = serialBT.read();
          counter++;
          if (counter > limit)
          {
            if (DEBUG)
            {
              Serial.print(" Overflow");
            }
            break;
          }
        }
      }
    }
    startup = false;
  }
  // Serial.println(" Ok");
}

// Send CI-V Command by Wifi
void sendCommandWifi(char *request, size_t n, char *buffer, uint8_t limit, boolean sendOnly = false)
{
  static uint8_t proxyError = 0;

  HTTPClient http;
  uint16_t httpCode;

  String command = "";
  String response = "";

  char s[4];

  for (uint8_t i = 0; i < n; i++)
  {
    sprintf(s, "%02x,", request[i]);
    command += String(s);
  }

  command += BAUD_RATE + String(",") + icSerialDevice;

  http.begin(civClient, PROXY_URL + String(":") + PROXY_PORT + String("/") + String("?civ=") + command); // Specify the URL
  http.addHeader("User-Agent", "M5Stack");                                                               // Specify header
  http.addHeader("Connection", "keep-alive");                                                            // Specify header
  http.setTimeout(100);                                                                                  // Set Time Out
  httpCode = http.GET();                                                                                 // Make the request
  if (httpCode == 200)
  {
    proxyConnected = true;
    proxyError = 0;

    response = http.getString(); // Get data
    response.trim();
    response = response.substring(4);

    if (response == "")
    {
      txConnected = false;
    }
    else
    {
      txConnected = true;
      startup = false;

      for (uint8_t i = 0; i < limit; i++)
      {
        buffer[i] = strtol(response.substring(i * 2, (i * 2) + 2).c_str(), NULL, 16);
      }

      if (DEBUG)
      {
        Serial.println("-----");
        Serial.print(response);
        Serial.print(" ");
        Serial.println(response.length());

        for (uint8_t i = 0; i < limit; i++)
        {
          Serial.print(int(buffer[i]));
          Serial.print(" ");
        }
        Serial.println(" ");
        Serial.println("-----");
      }
    }
  }
  else
  {
    proxyError++;
    if (proxyError > 10)
    {
      proxyError = 10;
      proxyConnected = false;
    }
  }
  http.end(); // Free the resources
}

// Send CI-V Command dispatcher
void sendCommand(char *request, size_t n, char *buffer, uint8_t limit, boolean sendOnly = false)
{
  if (icModel == 705 && icConnect == BT)
    sendCommandBt(request, n, buffer, limit, sendOnly);
  else
    sendCommandWifi(request, n, buffer, limit);
}

// Get Smeter Level
void getSmeterLevel()
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x15, 0x02, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  sprintf(str, "%02x%02x", buffer[4], buffer[5]);
  value = atoi(str);

  if(DEBUG) {
    Serial.print("Smeter Level ");
    Serial.println(value);
  }

  if (value != sOld)
  {
    sOld = value;

    for (uint8_t i = 0; i <= 180; i += STEP)
    {
      gaugeSprite.drawFastVLine(i, 0, 8, TFT_FIL_BACK);
    }

    for (uint8_t i = 0; i <= 4; i++)
    {
      gaugeSprite.drawFastVLine((i * 45), 7, 2, TFT_FIL_BORDER);
    }

    if (value != 0)
    {
      limit = map(value, 0, 241, 0, 180);

      for (uint8_t i = 0; i <= limit; i += STEP)
      {
        if (i == 0)
        {
          gaugeSprite.drawFastVLine(i, 0, 8, TFT_FIL_BORDER);
        }
        else if (i <= 90)
        {
          gaugeSprite.drawFastVLine(i, 0, 8, TFT_BLUE);
        }
        else
        {
          gaugeSprite.drawFastVLine(i, 0, 8, TFT_RED);
        }
      }
    }
    gaugeSprite.pushSprite(30 + offsetX, 122 + offsetY, TFT_TRANSPARENT);
  }
}

// Get SWR Level
void getSWRLevel()
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x15, 0x12, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  sprintf(str, "%02x%02x", buffer[4], buffer[5]);
  value = atoi(str);

  if(DEBUG) {
    Serial.print("SWR Level ");
    Serial.println(value);
  }

  if (needClear) SWROld = 255;

  if (value != SWROld)
  {
    SWROld = value;

    for (uint8_t i = 0; i <= 180; i += STEP)
    {
      gaugeSprite.drawFastVLine(i, 0, 8, TFT_FIL_BACK);
    }

    for (uint8_t i = 0; i <= 4; i += 1)
    {
      gaugeSprite.drawFastVLine((i * 22), 6, 2, TFT_FIL_BORDER);
    }

    if (value != 0)
    {
      if (value <= 48)
      {
        limit = map(value, 0, 48, 0, 22);
      }
      else if (value <= 80)
      {
        limit = 22 + map(value, 49, 80, 0, 22);
      }
      else if (value <= 120)
      {
        limit = 44 + map(value, 81, 120, 0, 44);
      }
      else
      {
        limit = 88 + map(value, 121, 255, 0, 92);
      }

      for (uint8_t i = 0; i <= limit; i += STEP)
      {
        if (i <= 88)
        {
          gaugeSprite.drawFastVLine(i, 0, 8, TFT_BLUE);
        }
        else
        {
          gaugeSprite.drawFastVLine(i, 0, 8, TFT_RED);
        }
      }
    }
    gaugeSprite.pushSprite(30 + offsetX, 194 + offsetY, TFT_TRANSPARENT);
  }
}

// Get Power Level
void getPowerLevel(uint8_t charge = 0)
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x15, 0x11, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  sprintf(str, "%02x%02x", buffer[4], buffer[5]);
  value = atoi(str);

  if(DEBUG) {
    Serial.print("Po Level ");
    Serial.println(value);
  }

  if (needClear) powerOld = 255;

  if (value > 213)
  {
    value = 213;
  }

  if (value != powerOld)
  {
    powerOld = value;

    for (uint8_t i = 0; i <= 180; i += STEP)
    {
      gaugeSprite.drawFastVLine(i, 0, 8, TFT_FIL_BACK);
    }

    for (uint8_t i = 0; i <= 4; i++)
    {
      gaugeSprite.drawFastVLine((i * 45), 7, 2, TFT_FIL_BORDER);
    }

    if (value != 0)
    {
      if (value <= 95)
      {
        limit = map(value, 0, 95, 0, 45);
      }
      else if (value <= 143)
      {
        limit = 45 + map(value, 96, 143, 0, 45);
      }
      else
      {
        limit = 90 + map(value, 144, 213, 0, 90);
      }

      if (charge == 1)
      {
        limit *= 2;
      }

      for (uint8_t i = 0; i <= limit; i += STEP)
      {
        if (i <= 90)
        {
          gaugeSprite.drawFastVLine(i, 0, 8, TFT_BLUE);
        }
        else
        {
          gaugeSprite.drawFastVLine(i, 0, 8, TFT_RED);
        }
      }
    }
    gaugeSprite.pushSprite(30 + offsetX, 122 + offsetY, TFT_TRANSPARENT);
  }
}

// Get Power Type
uint8_t getPowerType()
{
  static char buffer[5];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x1A, 0x0B, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 5);

  return buffer[4];
}

// Get Frequency
void getFrequency()
{
  String frequency;
  String frequencyNew;

  static char buffer[8];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x03, 0xFD};

  double freq; // Current frequency in Hz
  const uint32_t decMulti[] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};

  uint8_t lenght = 0;

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 8);

  freq = 0;
  for (uint8_t i = 2; i < 7; i++)
  {
    freq += (buffer[9 - i] >> 4) * decMulti[(i - 2) * 2];
    freq += (buffer[9 - i] & 0x0F) * decMulti[(i - 2) * 2 + 1];
  }

  if(transverter > 0)
    freq += double(choiceTransverter[transverter]);

  if(freq < 74800000)
  {
    bandeLow = true;
  }
  else
  {
    bandeLow = false;
  }

  frequency = String(freq);
  lenght = frequency.length();

  if(frequency != "0")
  {
    int8_t i;

    for(i = lenght - 6; i >= 0; i -= 3) 
    {
      frequencyNew = "." + frequency.substring(i, i + 3) + frequencyNew;
    }

    if(i == -3) 
    {
      frequencyNew = frequencyNew.substring(1, frequencyNew.length());
    }
    else 
    {
      frequencyNew = frequency.substring(0, i + 3) + frequencyNew;
    }
  }
  else {
    frequencyNew = "-";
  }

  frequency = frequencyNew;

  // If too long...

  if(frequency.length() >= 14) {
    frequency = frequency.substring(0, frequency.length() - 1);
  }

  if(frequency == "" || frequency == "-" || frequency == "163.163.163")
    txConnected = false;
  else
    txConnected = true;

  if (frequency != frequencyOld && txConnected == true)
  {
    frequencyOld = frequency;

    display.setTextDatum(CC_DATUM);
    display.setFont(&UniversCondensed20pt7b);
    display.setTextPadding(200);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    // valString.replace(".", ",");
    display.drawString(frequency, 160 + offsetX, 76 + offsetY);
  }

  if(DEBUG) {
    Serial.print("Frequency ");
    Serial.println(frequency);
  }
}

// Get Data Mode
uint8_t getModeData()
{
  static char buffer[6];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x1A, 0x06, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  return buffer[4];
}

// Get Mode
uint8_t getModeFilter()
{
  String value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x04, 0xFD};

  const char *mode[] = {"LSB", "USB", "AM", "CW", "RTTY", "FM", "WFM", "CW-R", "RTTY-R", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "DV"};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 5);

  display.setFont(&tahoma8pt7b);
  display.setTextPadding(24);
  display.setTextColor(TFT_WHITE);
  display.setTextDatum(CC_DATUM);

  if(uint8_t(buffer[4]) >= 1 && uint8_t(buffer[4]) <= 3)
  {
    value = "FIL" + String(uint8_t(buffer[4]));

    if (value != filterOld)
    {
      filterOld = value;
      display.fillRoundRect(70 + offsetX, 24 + offsetY, 44, 18, 2, TFT_FIL_BACK);
      display.drawRoundRect(70 + offsetX, 24 + offsetY, 44, 18, 2, TFT_FIL_BORDER);
      display.drawString(value, 92 + offsetX, 34 + offsetY);
    }

    if(DEBUG) {
      Serial.print("FIL ");
      Serial.println(value);
    }
  }

  if(uint8_t(buffer[4]) >= 0 && uint8_t(buffer[4]) <= 23)
  {

    value = String(mode[(uint8_t)buffer[3]]);

    if (getModeData() == 1)
    {
      value += "-D";
    }
    if (value != modeOld)
    {
      modeOld = value;
      display.fillRoundRect(2 + offsetX, 24 + offsetY, 60, 18, 2, TFT_MODE_BACK);
      display.drawRoundRect(2 + offsetX, 24 + offsetY, 60, 18, 2, TFT_MODE_BORDER);
      display.drawString(value, 31 + offsetX, 34 + offsetY);
    }

    if(DEBUG) {
      Serial.print("Mode ");
      Serial.println(value);
    }
  }

  return uint8_t(buffer[3]);
}

// Get Vd Level
void getVdLevel()
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x15, 0x15, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  sprintf(str, "%02x%02x", buffer[4], buffer[5]);
  value = atoi(str);

  if(DEBUG) {
    Serial.print("Vd Level ");
    Serial.println(value);
  }

  if(icModel == 705) {
    if (value < 75)
    {
      value = 75;
    }
  }
  else {
    if (value < 13)
    {
      value = 13;
    }
  }

  if (value != VdOld)
  {
    VdOld = value;

    for (uint8_t i = 0; i <= 80; i += 2)
    {
      display.drawFastVLine(230 + i + offsetX, 226 + offsetY, 8, TFT_FIL_BACK);
    }

    if(icModel == 705) {
      limit = map(value, 75, 241, 0, 80);
    }
    else 
    {
      limit = map(value, 13, 241, 0, 80);
    }

    for (uint8_t i = 0; i <= limit; i += 2)
    {
      display.drawFastVLine(230 + i + offsetX, 226 + offsetY, 8, TFT_BLUE);
    }
  }
}

// Get Id Level
void getIdLevel()
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x15, 0x16, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  sprintf(str, "%02x%02x", buffer[4], buffer[5]);
  value = atoi(str);

  if(DEBUG) {
    Serial.print("Id Level ");
    Serial.println(value);
  }

  if (needClear) IdOld = 255;

  if (value != IdOld)
  {
    IdOld = value;

    for (uint8_t i = 0; i <= 180; i += STEP)
    {
      gaugeSprite.drawFastVLine(i, 0, 8, TFT_FIL_BACK);
    }

    if(icModel == 705 || icModel == 9700)
    {
      for (uint8_t i = 0; i <= 12; i++)
      {
        if (i % 3 == 0)
        {
          gaugeSprite.drawFastVLine((i * 14), 6, 2, TFT_FIL_BORDER);
        }
      }
    }
    else
    {
      for (uint8_t i = 0; i <= 15; i++)
      {
        if (i % 3 == 0)
        {
          gaugeSprite.drawFastVLine((i * 11.3), 6, 2, TFT_FIL_BORDER);
        }
      }
    }

    if(value != 0) {
      limit = map(value, 0, 241, 0, 180);

      for (uint8_t i = 0; i <= limit; i += STEP)
      {
        gaugeSprite.drawFastVLine(i, 218, 8, TFT_CYAN);
      }
    }
  }
  gaugeSprite.pushSprite(30 + offsetX, 218 + offsetY, TFT_TRANSPARENT);
}

// Get ALC Level
void getALCLevel()
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x15, 0x13, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  sprintf(str, "%02x%02x", buffer[4], buffer[5]);
  value = atoi(str);

  if(DEBUG) {
    Serial.print("ALC Level ");
    Serial.println(value);
  }

  if (value > 120)
  {
    value = 120;
  }

  if (value != ALCOld)
  {
    ALCOld = value;

    for (uint8_t i = 0; i <= 180; i += STEP)
    {
      gaugeSprite.drawFastVLine(i, 0, 8, TFT_FIL_BACK);
    }

    if (value != 0)
    {
      limit = map(value, 0, 120, 0, 90);

      for (uint8_t i = 0; i <= limit; i += STEP)
      {
        gaugeSprite.drawFastVLine(i, 0, 8, TFT_BLUE);
      }
    }

    gaugeSprite.pushSprite(30 + offsetX, 146 + offsetY, TFT_TRANSPARENT);
  }
}

// Get TX
uint8_t getTX()
{
  uint8_t value;
  boolean control;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x1C, 0x00, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 5);

  if (buffer[4] <= 1)
  {
    value = buffer[4];
  }
  else
  {
    value = 0;
  }

  if (icModel == 705 && icConnect == BT && btConnected == true)
    control = true;
  else if (icConnect == USB && wifiConnected == true && proxyConnected == true && txConnected == true)
    control = true;
  else
    control = false;

  if (value != TXOld && control)
  {
    TXOld = value;

    display.setFont(&tahoma8pt7b);
    display.setTextPadding(24);
    display.setTextDatum(CC_DATUM);

    if (value == 1)
    {
      display.fillRoundRect(2 + offsetX, 2 + offsetY, 28, 18, 2, TFT_RED);
      display.drawRoundRect(2 + offsetX, 2 + offsetY, 28, 18, 2, TFT_WHITE);
      display.setTextColor(TFT_WHITE);
      display.drawString("TX", 15 + offsetX, 12 + offsetY);
    }
    else
    {
      display.fillRoundRect(2 + offsetX, 2 + offsetY, 28, 18, 2, TFT_BLACK);
      display.drawRoundRect(2 + offsetX, 2 + offsetY, 28, 18, 2, TFT_RED);
      display.setTextColor(TFT_RED);
      display.drawString("TX", 15 + offsetX, 12 + offsetY);
    }
  }

  return value;
}

// Get AGC
void getAGC()
{
  uint8_t value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x16, 0x12, 0xFD};

  const char *mode[] = {"", "AGC-F", "AGC-M", "AGC-S"};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 5);

  if (buffer[4] <= 3)
  {
    value = buffer[4];
  }
  else
  {
    value = 0;
  }

  if(DEBUG) {
    Serial.print("AGC ");
    Serial.println(String(mode[value]));
  }

  if (value != agcOld)
  {
    agcOld = value;

    display.setFont(&tahoma8pt7b);
    display.setTextPadding(50);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(CR_DATUM);
    display.drawString(String(mode[value]), 316 + offsetX, 34 + offsetY);
  }
}

// Get AN
void getAN()
{
  uint8_t value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x16, 0x41, 0xFD};

  const char *mode[] = {"  ", "AN"};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 5);

  if (buffer[4] <= 1)
  {
    value = buffer[4];
  }
  else
  {
    value = 0;
  }

  if(DEBUG) {
    Serial.print("AN ");
    Serial.println(String(mode[value]));
  }

  if (value != ANOld)
  {
    ANOld = value;

    display.setFont(&tahoma8pt7b);
    display.setTextPadding(24);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(CL_DATUM);
    display.drawString(String(mode[value]), 120 + offsetX, 12 + offsetY);
  }
}

// Get NB
boolean getNB()
{
  uint8_t value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x16, 0x22, 0xFD};

  const char *mode[] = {"  ", "NB"};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 5);

  if (buffer[4] <= 1)
  {
    value = buffer[4];
  }
  else
  {
    value = 0;
  }

  if(DEBUG) {
    Serial.print("NB ");
    Serial.println(String(mode[value]));
  }

  if (value != NBOld)
  {
    NBOld = value;

    display.setFont(&tahoma8pt7b);
    display.setTextPadding(24);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(CL_DATUM);
    display.drawString(String(mode[value]), 150 + offsetX, 12 + offsetY);
  }

  return value;
}

// Get NR
boolean getNR()
{
  uint8_t value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x16, 0x40, 0xFD};

  const char *mode[] = {"  ", "NR"};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 5);

  if (buffer[4] <= 1)
  {
    value = buffer[4];
  }
  else
  {
    value = 0;
  }

  if(DEBUG) {
    Serial.print("NR ");
    Serial.println(String(mode[value]));
  }

  if (value != NROld)
  {
    NROld = value;

    display.setFont(&tahoma8pt7b);
    display.setTextPadding(24);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(CL_DATUM);
    display.drawString(String(mode[value]), 180 + offsetX, 12 + offsetY);
  }

  return value;
}

// Get AMP
void getAMP()
{
  uint8_t value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x16, 0x02, 0xFD};

  const char *mode[] = {"  ", "P.AMP1", "P.AMP2"};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 5);

  if (buffer[4] <= 2)
  {
    value = buffer[4];
  }
  else
  {
    value = 0;
  }

  if(DEBUG) {
    Serial.print("AMP ");
    Serial.println(String(mode[value]));
  }

  if (value != AMPOld)
  {
    AMPOld = value;

    display.setFont(&tahoma8pt7b);
    display.setTextPadding(55);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(CL_DATUM);
    
    if(bandeLow == false && value != 0) {
      display.drawString("P.AMP", 120 + offsetX, 34 + offsetY);
    }
    else {
      display.drawString(String(mode[value]), 120 + offsetX, 34 + offsetY);
    }
  }
}

// Get Tone
void getTone(boolean retry = true)
{
  uint8_t value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x16, 0x5D, 0xFD};

  const char *mode[] = {" ", "TONE", "TSQL", "DTCS", "", "", "DTCS (T)", "TONE (T)/DTCS (R)", "DTCS (T)/TSQL (R)", "TONE (T)/TSQL (R)"};

  size_t n = sizeof(request) / sizeof(request[0]);

  if(retry == true) {
    sendCommand(request, n, buffer, 5);

    if (buffer[4] <= 9)
    {
      value = buffer[4];
    }
    else
    {
      value = 0;
    }
  }
  else {
    value = 0;
  }

  if(DEBUG) {
    Serial.print("TONE ");
    Serial.println(String(mode[value]));
  }

  if (value != toneOld)
  {
    toneOld = value;

    display.setFont(&tahoma8pt7b);
    display.setTextPadding(90);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(CL_DATUM);
    display.drawString(String(mode[value]), 180 + offsetX, 34 + offsetY);
  }
}

// Get AF Gain
uint8_t getAF()
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x14, 0x01, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  sprintf(str, "%02x%02x", buffer[4], buffer[5]);
  value = atoi(str);

  if(DEBUG) {
    Serial.print("AF ");
    Serial.println(value);
  }

  if (value != AFOld)
  {
    AFOld = value;

    levelSprite.fillRect(0, 0, 44, 10, TFT_BLACK);
    levelSprite.drawRect(0, 0, 44, 10, TFT_FIL_BORDER);

    if (value != 0)
    {
      limit = map(value, 0, 255, 0, 42);
      levelSprite.drawFastHLine(1, 1, limit, TFT_GAUGE_1);
      levelSprite.drawFastHLine(1, 2, limit, TFT_GAUGE_1);
      levelSprite.drawFastHLine(1, 3, limit, TFT_GAUGE_2);
      levelSprite.drawFastHLine(1, 4, limit, TFT_GAUGE_2);
      levelSprite.drawFastHLine(1, 5, limit, TFT_GAUGE_2);
      levelSprite.drawFastHLine(1, 6, limit, TFT_GAUGE_1);
      levelSprite.drawFastHLine(1, 7, limit, TFT_GAUGE_3);
      levelSprite.drawFastHLine(1, 8, limit, TFT_GAUGE_3);
    }

    levelSprite.pushSprite(245 + offsetX, 121 + offsetY, TFT_TRANSPARENT);

    display.setFont(0);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextPadding(18);
    display.setTextDatum(CL_DATUM);
    display.drawString("AF", 225 + offsetX, 126 + offsetY);
    display.setTextPadding(24);
    display.setTextDatum(CR_DATUM);
    display.drawString(String(map(value, 0, 255, 0, 100)) + "%", 316 + offsetX, 126 + offsetY);
  }

  return value;
}

// Get MIC Gain
uint8_t getMIC()
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x14, 0x0B, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  sprintf(str, "%02x%02x", buffer[4], buffer[5]);
  value = atoi(str);

  if(DEBUG) {
    Serial.print("MIC ");
    Serial.println(value);
  }

  if (value != MICOld)
  {
    MICOld = value;

    levelSprite.fillRect(0, 0, 44, 10, TFT_BLACK);
    levelSprite.drawRect(0, 0, 44, 10, TFT_FIL_BORDER);

    if (value != 0)
    {
      limit = map(value, 0, 255, 0, 42);
      levelSprite.drawFastHLine(1, 1, limit, TFT_GAUGE_1);
      levelSprite.drawFastHLine(1, 2, limit, TFT_GAUGE_1);
      levelSprite.drawFastHLine(1, 3, limit, TFT_GAUGE_2);
      levelSprite.drawFastHLine(1, 4, limit, TFT_GAUGE_2);
      levelSprite.drawFastHLine(1, 5, limit, TFT_GAUGE_2);
      levelSprite.drawFastHLine(1, 6, limit, TFT_GAUGE_1);
      levelSprite.drawFastHLine(1, 7, limit, TFT_GAUGE_3);
      levelSprite.drawFastHLine(1, 8, limit, TFT_GAUGE_3);
    }

    levelSprite.pushSprite(245 + offsetX, 138 + offsetY, TFT_TRANSPARENT);

    display.setFont(0);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextPadding(18);
    display.setTextDatum(CL_DATUM);
    display.drawString("MIC", 225 + offsetX, 143 + offsetY);
    display.setTextPadding(24);
    display.setTextDatum(CR_DATUM);
    display.drawString(String(map(value, 0, 255, 0, 100)) + "%", 316 + offsetX, 143 + offsetY);
  }

  return value;
}

// Get SQL
uint8_t getSQL()
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x14, 0x03, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  sprintf(str, "%02x%02x", buffer[4], buffer[5]);
  value = atoi(str);

  if(DEBUG) {
    Serial.print("SQL ");
    Serial.println(value);
  }

  if (value != SQLOld)
  {
    SQLOld = value;

    levelSprite.fillRect(0, 0, 44, 10, TFT_BLACK);
    levelSprite.drawRect(0, 0, 44, 10, TFT_FIL_BORDER);

    if (value != 0)
    {
      limit = map(value, 0, 255, 0, 42);
      levelSprite.drawFastHLine(1, 1, limit, TFT_GAUGE_1);
      levelSprite.drawFastHLine(1, 2, limit, TFT_GAUGE_1);
      levelSprite.drawFastHLine(1, 3, limit, TFT_GAUGE_2);
      levelSprite.drawFastHLine(1, 4, limit, TFT_GAUGE_2);
      levelSprite.drawFastHLine(1, 5, limit, TFT_GAUGE_2);
      levelSprite.drawFastHLine(1, 6, limit, TFT_GAUGE_1);
      levelSprite.drawFastHLine(1, 7, limit, TFT_GAUGE_3);
      levelSprite.drawFastHLine(1, 8, limit, TFT_GAUGE_3);
    }

    levelSprite.pushSprite(245 + offsetX, 155 + offsetY, TFT_TRANSPARENT);

    display.setFont(0);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextPadding(18);
    display.setTextDatum(CL_DATUM);
    display.drawString("SQL", 225 + offsetX, 160 + offsetY);
    display.setTextPadding(24);
    display.setTextDatum(CR_DATUM);
    display.drawString(String(map(value, 0, 255, 0, 100)) + "%", 316 + offsetX, 160 + offsetY);
  }

  return value;
}

// Get NR Level
void getNRLevel()
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x14, 0x06, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  sprintf(str, "%02x%02x", buffer[4], buffer[5]);
  value = atoi(str);

  if(DEBUG) {
    Serial.print("NR Level ");
    Serial.println(value);
  }

  if (value != NRLevelOld)
  {
    NRLevelOld = value;

    levelSprite.fillRect(0, 0, 44, 10, TFT_BLACK);
    levelSprite.drawRect(0, 0, 44, 10, TFT_FIL_BORDER);

    if (value > 0)
    {
      limit = map(value, 8, 248, 0, 42);
      levelSprite.drawFastHLine(1, 1, limit, TFT_GAUGE_1);
      levelSprite.drawFastHLine(1, 2, limit, TFT_GAUGE_1);
      levelSprite.drawFastHLine(1, 3, limit, TFT_GAUGE_2);
      levelSprite.drawFastHLine(1, 4, limit, TFT_GAUGE_2);
      levelSprite.drawFastHLine(1, 5, limit, TFT_GAUGE_2);
      levelSprite.drawFastHLine(1, 6, limit, TFT_GAUGE_1);
      levelSprite.drawFastHLine(1, 7, limit, TFT_GAUGE_3);
      levelSprite.drawFastHLine(1, 8, limit, TFT_GAUGE_3);
    }

    levelSprite.pushSprite(245 + offsetX, 172 + offsetY, TFT_TRANSPARENT);

    display.setFont(0);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextPadding(18);
    display.setTextDatum(CL_DATUM);
    display.drawString("NR", 225 + offsetX, 177 + offsetY);
    display.setTextPadding(24);
    display.setTextDatum(CR_DATUM);
    display.drawString(String(map(value, 8, 248, 0, 100)) + "%", 316 + offsetX, 177 + offsetY);
  }
}

// Clean NR Level
void cleanNRLevel()
{
  NRLevelOld = 0;

  display.fillRect(245 + offsetX, 172 + offsetY, 44, 10, TFT_BLACK);
  display.drawRect(245 + offsetX, 172 + offsetY, 44, 10, TFT_FIL_BORDER);

  display.setFont(0);
  display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  display.setTextPadding(18);
  display.setTextDatum(CL_DATUM);
  display.drawString("NR", 225 + offsetX, 177 + offsetY);
  display.setTextPadding(24);
  display.setTextDatum(CR_DATUM);
  display.drawString(String(map(8, 8, 248, 0, 100)) + "%", 316 + offsetX, 177 + offsetY);
}

// Get NB Level
void getNBLevel()
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x14, 0x12, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  sprintf(str, "%02x%02x", buffer[4], buffer[5]);
  value = atoi(str);

  if(DEBUG) {
    Serial.print("NB Level ");
    Serial.println(value);
  }

  if (value != NBLevelOld)
  {
    NBLevelOld = value;

    levelSprite.fillRect(0, 0, 44, 10, TFT_BLACK);
    levelSprite.drawRect(0, 0, 44, 10, TFT_FIL_BORDER);

    if (value > 0)
    {
      limit = map(value, 0, 255, 0, 42);
      levelSprite.drawFastHLine(1, 1, limit, TFT_GAUGE_1);
      levelSprite.drawFastHLine(1, 2, limit, TFT_GAUGE_1);
      levelSprite.drawFastHLine(1, 3, limit, TFT_GAUGE_2);
      levelSprite.drawFastHLine(1, 4, limit, TFT_GAUGE_2);
      levelSprite.drawFastHLine(1, 5, limit, TFT_GAUGE_2);
      levelSprite.drawFastHLine(1, 6, limit, TFT_GAUGE_1);
      levelSprite.drawFastHLine(1, 7, limit, TFT_GAUGE_3);
      levelSprite.drawFastHLine(1, 8, limit, TFT_GAUGE_3);
    }

    levelSprite.pushSprite(245 + offsetX, 189 + offsetY, TFT_TRANSPARENT);

    display.setFont(0);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextPadding(18);
    display.setTextDatum(CL_DATUM);
    display.drawString("NB", 225 + offsetX, 194 + offsetY);
    display.setTextPadding(24);
    display.setTextDatum(CR_DATUM);
    display.drawString(String(map(value, 0, 255, 0, 100)) + "%", 316 + offsetX, 194 + offsetY);
  }
}

// Clean NB Level
void cleanNBLevel()
{
  NBLevelOld = 0;

  display.fillRect(245 + offsetX, 189 + offsetY, 44, 10, TFT_BLACK);
  display.drawRect(245 + offsetX, 189 + offsetY, 44, 10, TFT_FIL_BORDER);

  display.setFont(0);
  display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  display.setTextPadding(18);
  display.setTextDatum(CL_DATUM);
  display.drawString("NB", 225 + offsetX, 194 + offsetY);
  display.setTextPadding(24);
  display.setTextDatum(CR_DATUM);
  display.drawString(String(map(0, 0, 255, 0, 100)) + "%", 316 + offsetX, 194 + offsetY);
}

// Get COMP
uint8_t getCOMP(boolean retry = true)
{
  uint8_t value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x16, 0x44, 0xFD};

  const char *mode[] = {" ", "COMP"};

  size_t n = sizeof(request) / sizeof(request[0]);

  if(retry == true) {
    sendCommand(request, n, buffer, 5);

    if (buffer[4] <= 1)
    {
      value = buffer[4];
    }
    else
    {
      value = 0;
    }
  }
  else {
    value = 0;
  }

  if(DEBUG) {
    Serial.print("COMP ");
    Serial.println(String(mode[value]));
  }

  if (value != COMPOld)
  {
    COMPOld = value;

    display.setFont(&tahoma8pt7b);
    display.setTextPadding(50);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(CL_DATUM);
    display.drawString(String(mode[value]), 4 + offsetX, 79 + offsetY);
  }
  return value;
}

// Get COMP Level
void getCOMPLevel()
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x15, 0x14, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  sprintf(str, "%02x%02x", buffer[4], buffer[5]);
  value = atoi(str);

  if(value > 210) {
    value = 210;
  }

  if (needClear) COMPLOld = 255;

  if(DEBUG) {
    Serial.print("COMP Level ");
    Serial.println(value);
  }

  if (value != COMPLOld)
  {
    COMPLOld = value;

    const char *legendeCOMP[] = {"0", "5", "10", "15", "20"};

    for (uint8_t i = 0; i <= 180; i += STEP)
    {
      gaugeSprite.drawFastVLine(i, 170, 8, TFT_FIL_BACK);
    }

    limit = map(value, 0, 210, 0, 180);

    for (uint8_t i = 0; i <= limit; i += STEP)
    {
      gaugeSprite.drawFastVLine(i, 0, 8, TFT_BLUE);
    }

    gaugeSprite.pushSprite(30 + offsetX, 170 + offsetY, TFT_TRANSPARENT);

    display.setFont(0);
    display.setTextDatum(CC_DATUM);
    display.setTextColor(TFT_WHITE);

    for (uint8_t i = 0; i <= 4; i += 1)
    {
      display.drawString(legendeCOMP[i], 34 + (i * 34) + offsetX, 174 + offsetY);
    }
    display.drawString("dB", 204 + offsetX, 174 + offsetY);
  }
}

// Get RIT
void getRIT()
{
  static char buffer[8];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x21, 0x00, 0xFD};

  String RIT;

  String val0;
  String val1;
  String val2;

  uint16_t freq; // Current frequency in Hz
  const uint32_t decMulti[] = {1000, 100, 10, 1};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 7);

  if((buffer[5] >> 4) * decMulti[0] >= 1000) {
    val0 = String((buffer[5] >> 4) * decMulti[3]);
  }
  else {
    val0 = "0";
  }

  if(buffer[6] != 0) {
    val0 = "-" + val0;
  }

  freq = 0;
  freq += (buffer[5] & 0x0F) * decMulti[1];
  freq += (buffer[4] >> 4) * decMulti[2];
  freq += (buffer[4] & 0x0F) * decMulti[3];

  if(freq < 10) {
    val1 = "00" + String(freq);
  }
  else if(freq < 100) {
    val1 = "0" + String(freq);
  }
  else {
    val1 = String(freq);
  }
  
  RIT = val0 + "." + val1;

  if(DEBUG) {
    Serial.print("RIT ");
    Serial.println(RIT);
  }

  if (RIT != RITOld)
  {
    RITOld = RIT;

    display.setFont(&tahoma8pt7b);
    display.setTextPadding(50);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(CR_DATUM);
    display.drawString("RIT", 316 + offsetX, 66 + offsetY);
    display.drawString(RIT, 316 + offsetX, 81 + offsetY);
  }
}

// Get IP+
void getIP()
{
  uint8_t value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x1A, 0x07, 0xFD};

  const char *mode[] = {"  ", "IP+"};

  size_t n = sizeof(request) / sizeof(request[0]);

  if(icModel != 7300) return;

  sendCommand(request, n, buffer, 5);

  if (buffer[4] <= 1)
  {
    value = buffer[4];
  }
  else
  {
    value = 0;
  }

  if(DEBUG) {
    Serial.print("IP+ ");
    Serial.println(String(mode[value]));
  }

  if (value != IPOld)
  {
    IPOld = value;

    display.setFont(&tahoma8pt7b);
    display.setTextPadding(26);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(CL_DATUM);
    display.drawString(String(mode[value]), 210 + offsetX, 12 + offsetY);
  }
}

// Send Voice
void sendVoice()
{
  uint8_t value;
  static char buffer[7];
  size_t n;

  // Repeat Time
  char repeat[][9] = {
    {0xFE, 0xFE, icCIVAddress, 0xE0, 0x1A, 0x05, 0x02, 0x45, 0xFD},   // IC-705
    {0xFE, 0xFE, icCIVAddress, 0xE0, 0x1A, 0x05, 0x01, 0x81, 0xFD},   // IC-7300
    {0xFE, 0xFE, icCIVAddress, 0xE0, 0x1A, 0x05, 0x02, 0x17, 0xFD}    // IC-9700
  };

  if(icModel == 705)
  {
    n = sizeof(repeat[0]) / sizeof(repeat[0][0]);
    sendCommand(repeat[0], n, buffer, 7);
  }
  else if(icModel == 7300)
  {
    n = sizeof(repeat[1]) / sizeof(repeat[1][0]);
    sendCommand(repeat[1], n, buffer, 7);
  }
  else {
    n = sizeof(repeat[2]) / sizeof(repeat[2][0]);
    sendCommand(repeat[2], n, buffer, 7);
  }

  if (buffer[6] < 9)
  {
    value = buffer[6];
  }
  else if (buffer[6] > 9)
  {
    value = buffer[6] - 6; // Bug in the documentation !!! Fix it Icom !!!
  }
  else
  {
    value = 0;
  }

  voiceTimeout = value;
  Serial.println(voiceTimeout);

  // Send
  char request[] = {0xFE, 0xFE, icCIVAddress, 0xE0, 0x28, 0x00, 0x00, 0xFD};

  request[6] += voice;

  n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 5, true);

  display.setFont(&tahoma8pt7b);
  display.setTextPadding(24);
  display.setTextDatum(CC_DATUM);

  if (voiceCounter > 0)
  {
    display.fillRoundRect(32 + offsetX, 2 + offsetY, 28, 18, 2, TFT_RED);
    display.drawRoundRect(32 + offsetX, 2 + offsetY, 28, 18, 2, TFT_WHITE);
    display.setTextColor(TFT_WHITE);
    display.drawString("T" + String(voice), 45 + offsetX, 12 + offsetY);
  }

  if(DEBUG) {
    Serial.print("Voice TX ");
    Serial.println(String(choiceVoice[voice]));
  }
}