// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

int btnA, btnB, btnC, btnL, btnM, btnR;

// Expand virtual fucking button
Button myBtnA(0, 200, 100, 80);
Button myBtnB(110, 200, 100, 80);
Button myBtnC(220, 200, 100, 80);

// Power init
void power()
{
  // By default, M5.begin() will initialize AXP192 chip on Core2
  M5.Axp.SetLed(0);
}

// Get Battery level
int8_t getBatteryLevel(bool type)
{
  float baterryVoltage = M5.Axp.GetBatVoltage();
  int16_t batteryPercentage = (baterryVoltage < 3.2) ? 0 : (baterryVoltage - 3.2) * 100;

  if (batteryPercentage > 100)
  {
    batteryPercentage = 100;
  }

  if (type == 0)
  {
    if (batteryPercentage > 80)
    {
      return 100;
    }
    else if (batteryPercentage > 60)
    {
      return 75;
    }
    else if (batteryPercentage > 40)
    {
      return 50;
    }
    else if (batteryPercentage > 20)
    {
      return 25;
    }
    return 0;
  }
  else
  {
    return batteryPercentage;
  }
}

// Control if charging
bool isCharging()
{
  if (M5.Axp.GetBatCurrent() < 0)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

// Get button
void getButton()
{
  M5.update();
  btnA = myBtnA.read();
  btnB = myBtnB.read();
  btnC = myBtnC.read();
  btnL = myBtnA.pressedFor(2000);
  btnM = myBtnB.pressedFor(2000);
  btnR = myBtnC.pressedFor(2000);

  if (btnL)
  {
    btnA = 0;
    //Serial.println("Left");
  }
  else if (btnM)
  {
    btnB = 0;
    //Serial.println("Middle");
  }
  else if (btnR)
  {
    btnC = 0;
    //Serial.println("Right");
  } 

  /*
  Serial.print(btnA);
  Serial.print(btnB);
  Serial.print(btnC);
  Serial.print(btnL);
  Serial.println(btnR);
  */
}

// Manage brightness
void setBrightness(uint8_t value)
{
  uint16_t tmp = map(value, 0, 128, 2500, 3300);
  M5.Axp.SetLcdVoltage(tmp); // from 2500 to 3300
}

// Shutdown
void shutdown()
{
  M5.Axp.PowerOff();
}