// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

int btnA, btnB, btnC, btnL, btnM, btnR;

// Get Battery level
int8_t getBatteryLevel(bool type)
{
  return M5.Power.getBatteryLevel();
}

// Control if charging
bool isCharging()
{
  return M5.Power.isCharging();
}

// Get button
void getButton()
{
  M5.update();
  
  btnA = M5.BtnA.isPressed();
  btnB = M5.BtnB.isPressed();
  btnC = M5.BtnC.isPressed();

  /*
  btnL = M5.BtnA.pressedFor(2000);
  btnM = M5.BtnB.pressedFor(2000);
  btnR = M5.BtnC.pressedFor(2000);

  Serial.print(btnA);
  Serial.print(btnB);
  Serial.println(btnC);
  */
}

// Manage brightness
void setBrightness(uint8_t value)
{
  display.setBrightness(value);
}

// Shutdown
void shutdown()
{
  M5.Power.powerOff();
}