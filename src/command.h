// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Get Smeter Level
void getSmeterLevel()
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x15, 0x02, 0xFD};

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
      M5.Lcd.drawFastVLine(30 + i, 122, 8, TFT_FIL_BACK);
    }

    for (uint8_t i = 0; i <= 4; i++)
    {
      M5.Lcd.drawFastVLine(30 + (i * 45), 129, 2, TFT_FIL_BORDER);
    }

    if (value != 0)
    {
      limit = map(value, 0, 241, 0, 180);

      for (uint8_t i = 0; i <= limit; i += STEP)
      {
        if (i == 0)
        {
          M5.Lcd.drawFastVLine(30 + i, 122, 8, TFT_FIL_BORDER);
        }
        else if (i <= 90)
        {
          M5.Lcd.drawFastVLine(30 + i, 122, 8, TFT_BLUE);
        }
        else
        {
          M5.Lcd.drawFastVLine(30 + i, 122, 8, TFT_RED);
        }
      }
    }
  }
}

// Get SWR Level
void getSWRLevel()
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x15, 0x12, 0xFD};

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
      M5.Lcd.drawFastVLine(30 + i, 194, 8, TFT_FIL_BACK);
    }

    for (uint8_t i = 0; i <= 4; i += 1)
    {
      M5.Lcd.drawFastVLine(30 + (i * 22), 200, 2, TFT_FIL_BORDER);
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
          M5.Lcd.drawFastVLine(30 + i, 194, 8, TFT_BLUE);
        }
        else
        {
          M5.Lcd.drawFastVLine(30 + i, 194, 8, TFT_RED);
        }
      }
    }
  }
}

// Get Power Level
void getPowerLevel(uint8_t charge = 0)
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x15, 0x11, 0xFD};

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
      M5.Lcd.drawFastVLine(30 + i, 122, 8, TFT_FIL_BACK);
    }

    for (uint8_t i = 0; i <= 4; i++)
    {
      M5.Lcd.drawFastVLine(30 + (i * 45), 129, 2, TFT_FIL_BORDER);
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
          M5.Lcd.drawFastVLine(30 + i, 122, 8, TFT_BLUE);
        }
        else
        {
          M5.Lcd.drawFastVLine(30 + i, 122, 8, TFT_RED);
        }
      }
    }
  }
}

// Get Power Type
uint8_t getPowerType()
{
  static char buffer[5];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x1A, 0x0B, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 5);

  return buffer[4];
}

// Get Frequency
void getFrequency()
{
  static char buffer[8];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x03, 0xFD};

  String frequency;

  String val0;
  String val1;
  String val2;

  uint32_t freq; // Current frequency in Hz
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

  frequency = String(freq);
  lenght = frequency.length();
  val0 = frequency.substring(lenght - 3, lenght);
  val1 = frequency.substring(lenght - 6, lenght - 3);
  val2 = frequency.substring(0, lenght - 6);

  frequency = val2 + "." + val1 + "." + val0;
  bande = val2.toInt();

  if(frequency == "" || frequency == "0.." || frequency == "163.163.163")
    txConnected = false;
  else
    txConnected = true;

  if (frequency != frequencyOld && txConnected == true)
  {
    frequencyOld = frequency;

    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setFreeFont(&UniversCondensed20pt7b);
    M5.Lcd.setTextPadding(200);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    // valString.replace(".", ",");
    M5.Lcd.drawString(frequency, 160, 70);
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
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x1A, 0x06, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  return buffer[4];
}

// Get Mode
uint8_t getModeFilter()
{
  String value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x04, 0xFD};

  const char *mode[] = {"LSB", "USB", "AM", "CW", "RTTY", "FM", "WFM", "CW-R", "RTTY-R", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "DV"};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 5);

  M5.Lcd.setFreeFont(&tahoma8pt7b);
  M5.Lcd.setTextPadding(24);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextDatum(CC_DATUM);

  if(uint8_t(buffer[4]) >= 1 && uint8_t(buffer[4]) <= 3)
  {
    value = "FIL" + String(uint8_t(buffer[4]));

    if (value != filterOld)
    {
      filterOld = value;
      M5.Lcd.fillRoundRect(70, 24, 44, 18, 2, TFT_FIL_BACK);
      M5.Lcd.drawRoundRect(70, 24, 44, 18, 2, TFT_FIL_BORDER);
      M5.Lcd.drawString(value, 92, 32);
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
      M5.Lcd.fillRoundRect(2, 24, 60, 18, 2, TFT_MODE_BACK);
      M5.Lcd.drawRoundRect(2, 24, 60, 18, 2, TFT_MODE_BORDER);
      M5.Lcd.drawString(value, 31, 32);
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
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x15, 0x15, 0xFD};

  size_t n = sizeof(request) / sizeof(request[0]);

  sendCommand(request, n, buffer, 6);

  sprintf(str, "%02x%02x", buffer[4], buffer[5]);
  value = atoi(str);

  if(DEBUG) {
    Serial.print("Vd Level ");
    Serial.println(value);
  }

  if (value < 75)
  {
    value = 75;
  }

  if (value != VdOld)
  {
    VdOld = value;

    for (uint8_t i = 0; i <= 80; i += 2)
    {
      M5.Lcd.drawFastVLine(230 + i, 224, 8, TFT_FIL_BACK);
    }

    limit = map(value, 75, 241, 0, 80);

    for (uint8_t i = 0; i <= limit; i += 2)
    {
      M5.Lcd.drawFastVLine(230 + i, 224, 8, TFT_BLUE);
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
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x15, 0x16, 0xFD};

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
      M5.Lcd.drawFastVLine(30 + i, 218, 8, TFT_FIL_BACK);
    }

    for (uint8_t i = 0; i <= 12; i++)
    {
      if (i % 3 == 0)
      {
        M5.Lcd.drawFastVLine(30 + (i * 14), 224, 2, TFT_FIL_BORDER);
      }
    }

    if(value != 0) {
      limit = map(value, 0, 241, 0, 180);

      for (uint8_t i = 0; i <= limit; i += STEP)
      {
        M5.Lcd.drawFastVLine(30 + i, 218, 8, TFT_CYAN);
      }
    }
  }
}

// Get ALC Level
void getALCLevel()
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x15, 0x13, 0xFD};

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
      M5.Lcd.drawFastVLine(30 + i, 146, 8, TFT_FIL_BACK);
    }

    if (value != 0)
    {
      limit = map(value, 0, 120, 0, 90);

      for (uint8_t i = 0; i <= limit; i += STEP)
      {
        M5.Lcd.drawFastVLine(30 + i, 146, 8, TFT_BLUE);
      }
    }
  }
}

// Get TX
uint8_t getTX()
{
  uint8_t value;
  boolean control;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x1C, 0x00, 0xFD};

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

  if (IC_MODEL == 705 && IC_CONNECT == BT && btConnected == true)
    control = true;
  else if (IC_CONNECT == USB && wifiConnected == true && proxyConnected == true && txConnected == true)
    control = true;
  else
    control = false;

  if (value != TXOld && control)
  {
    TXOld = value;

    M5.Lcd.setFreeFont(&tahoma8pt7b);
    M5.Lcd.setTextPadding(24);
    M5.Lcd.setTextDatum(CC_DATUM);

    if (value == 1)
    {
      M5.Lcd.fillRoundRect(2, 2, 28, 18, 2, TFT_RED);
      M5.Lcd.drawRoundRect(2, 2, 28, 18, 2, TFT_WHITE);
      M5.Lcd.setTextColor(TFT_WHITE);
      M5.Lcd.drawString("TX", 15, 10);
    }
    else
    {
      M5.Lcd.fillRoundRect(2, 2, 28, 18, 2, TFT_BLACK);
      M5.Lcd.drawRoundRect(2, 2, 28, 18, 2, TFT_RED);
      M5.Lcd.setTextColor(TFT_RED);
      M5.Lcd.drawString("TX", 15, 10);
    }
  }

  return value;
}

// Get AGC
void getAGC()
{
  uint8_t value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x16, 0x12, 0xFD};

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

    M5.Lcd.setFreeFont(&tahoma8pt7b);
    M5.Lcd.setTextPadding(50);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CR_DATUM);
    M5.Lcd.drawString(String(mode[value]), 316, 32);
  }
}

// Get AN
void getAN()
{
  uint8_t value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x16, 0x41, 0xFD};

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

    M5.Lcd.setFreeFont(&tahoma8pt7b);
    M5.Lcd.setTextPadding(24);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CL_DATUM);
    M5.Lcd.drawString(String(mode[value]), 120, 10);
  }
}

// Get NB
void getNB()
{
  uint8_t value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x16, 0x22, 0xFD};

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

    M5.Lcd.setFreeFont(&tahoma8pt7b);
    M5.Lcd.setTextPadding(24);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CL_DATUM);
    M5.Lcd.drawString(String(mode[value]), 160, 10);
  }
}

// Get NR
void getNR()
{
  uint8_t value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x16, 0x40, 0xFD};

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

    M5.Lcd.setFreeFont(&tahoma8pt7b);
    M5.Lcd.setTextPadding(24);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CL_DATUM);
    M5.Lcd.drawString(String(mode[value]), 200, 10);
  }
}

// Get AMP
void getAMP()
{
  uint8_t value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x16, 0x02, 0xFD};

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

    M5.Lcd.setFreeFont(&tahoma8pt7b);
    M5.Lcd.setTextPadding(55);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CL_DATUM);
    if(bande > 100 && value != 0) {
      M5.Lcd.drawString("P.AMP", 120, 32);
    }
    else {
      M5.Lcd.drawString(String(mode[value]), 120, 32);
    }
  }
}

// Get Tone
void getTone(boolean retry = true)
{
  uint8_t value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x16, 0x5D, 0xFD};

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

    M5.Lcd.setFreeFont(&tahoma8pt7b);
    M5.Lcd.setTextPadding(90);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CL_DATUM);
    M5.Lcd.drawString(String(mode[value]), 180, 32);
  }
}

// Get AF Gain
uint8_t getAF()
{
  uint8_t value;
  uint8_t limit;

  char str[2];
  static char buffer[6];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x14, 0x01, 0xFD};

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

    M5.Lcd.fillRect(230, 135, 60, 10, TFT_BLACK);
    M5.Lcd.drawRect(230, 135, 60, 10, TFT_FIL_BORDER);

    if (value != 0)
    {
      limit = map(value, 0, 255, 0, 56);
      for (uint8_t i = 0; i <= limit; i += STEP)
      {
        M5.Lcd.drawFastVLine(232 + i, 137, 6, TFT_DARKGREEN);
      }
      //M5.Lcd.fillRect(232, 137, limit, 6, TFT_DARKGREEN);
    }

    M5.Lcd.setFreeFont(0);
    M5.Lcd.setTextPadding(24);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CR_DATUM);
    M5.Lcd.drawString(String(map(value, 0, 255, 0, 100)) + "%", 316, 140);
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
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x14, 0x0B, 0xFD};

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

    M5.Lcd.fillRect(230, 157, 60, 10, TFT_BLACK);
    M5.Lcd.drawRect(230, 157, 60, 10, TFT_FIL_BORDER);

    if (value != 0)
    {
      limit = map(value, 0, 255, 0, 56);
      for (uint8_t i = 0; i <= limit; i += STEP)
      {
        M5.Lcd.drawFastVLine(232 + i, 159, 6, TFT_DARKGREEN);
      }
      //M5.Lcd.fillRect(232, 159, limit, 6, TFT_DARKGREEN);
    }

    M5.Lcd.setFreeFont(0);
    M5.Lcd.setTextPadding(24);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CR_DATUM);
    M5.Lcd.drawString(String(map(value, 0, 255, 0, 100)) + "%", 316, 162);
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
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x14, 0x03, 0xFD};

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

    M5.Lcd.fillRect(230, 179, 60, 10, TFT_BLACK);
    M5.Lcd.drawRect(230, 179, 60, 10, TFT_FIL_BORDER);

    if (value != 0)
    {
      limit = map(value, 0, 255, 0, 56);
      for (uint8_t i = 0; i <= limit; i += STEP)
      {
        M5.Lcd.drawFastVLine(232 + i, 181, 6, TFT_DARKGREEN);
      }
      //M5.Lcd.fillRect(232, 181, limit, 6, TFT_DARKGREEN);
    }

    M5.Lcd.setFreeFont(0);
    M5.Lcd.setTextPadding(24);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CR_DATUM);
    M5.Lcd.drawString(String(map(value, 0, 255, 0, 100)) + "%", 316, 184);
  }

  return value;
}

// Get COMP
uint8_t getCOMP(boolean retry = true)
{
  uint8_t value;

  static char buffer[5];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x16, 0x44, 0xFD};

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

    M5.Lcd.setFreeFont(&tahoma8pt7b);
    M5.Lcd.setTextPadding(50);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CL_DATUM);
    M5.Lcd.drawString(String(mode[value]), 4, 79);
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
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x15, 0x14, 0xFD};

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
      M5.Lcd.drawFastVLine(30 + i, 170, 8, TFT_FIL_BACK);
    }

    limit = map(value, 0, 210, 0, 180);

    for (uint8_t i = 0; i <= limit; i += STEP)
    {
      M5.Lcd.drawFastVLine(30 + i, 170, 8, TFT_BLUE);
    }

    M5.Lcd.setFreeFont(0);
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setTextColor(TFT_WHITE);

    for (uint8_t i = 0; i <= 4; i += 1)
    {
      M5.Lcd.drawString(legendeCOMP[i], 34 + (i * 34), 174);
    }
    M5.Lcd.drawString("dB", 204, 174);
  }
}

// Get RIT
void getRIT()
{
  static char buffer[8];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x21, 0x00, 0xFD};

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

    M5.Lcd.setFreeFont(&tahoma8pt7b);
    M5.Lcd.setTextPadding(50);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextDatum(CR_DATUM);
    M5.Lcd.drawString("RIT", 316, 64);
    M5.Lcd.drawString(RIT, 316, 79);
  }
}