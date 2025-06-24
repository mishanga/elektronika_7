#include <LedControl.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

#define showClockDelay 10000
#define showWeatherDelay 3000
#define showDateDelay 3000

const char fullDateFormat[] = "DD.MM.YYYY hh:mm:ss";
const char dateFormat[] = "DDMM";
const char timeFormat[] = "hhmm";

LedControl lc = LedControl(8, 10, 9, 1); // DIN, CLK, CS, num | layout: fc16
LiquidCrystal_I2C lcd(0x3F, 16, 2);      // custom address!
RTC_DS3231 rtc;

DateTime rtcNow;
uint32_t curMillis = 0;
uint32_t lastClockStateChange = 0;
uint32_t lastSetStateChange = 0;
uint8_t clockState = 0;
uint8_t setState = 0;

/*
uint8_t matrix[10][3] = {
  {31, 1, 7, 1, 7}, // 0
  {7, 1, 7, 1, 7}, // 3
};
*/

uint8_t matrix[8] = {0, 1, 6, 5, 4, 31, 255, 0};

void printMatrix(uint8_t *m, const uint8_t len)
{
  m[0] = 100;

  for (uint8_t i = 0; i < len; i++)
  {
    Serial.println(matrix[i], BIN);
    // std::cout << std::endl;
  }
}

void setup()
{
  Serial.begin(115200);
  lcd.begin(16, 2);
  rtc.begin();

  rtcNow = rtc.now();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(rtcNow.toString(fullDateFormat));

  lc.shutdown(0, false);
  lc.setIntensity(0, 10);
  lc.clearDisplay(0);

  lc.setLed(0, 0, 0, true);
  lc.setLed(0, 1, 1, true);
  lc.setLed(0, 2, 2, true);
  lc.setLed(0, 0, 1, true);
  lc.setLed(0, 0, 2, true);
  lc.setLed(0, 1, 0, true);
  lc.setLed(0, 2, 0, true);

  // 0: 31, 17, 31
  // 1: 0, 0 (8?), 31
  // 2: 23, 21, 29
  // 3: 17, 21, 31
  // 4: 28, 4, 31
  // 8: 31, 21, 31

  // Serial.print(rtc.getTemperature());
  Serial.print("init done");
}

void loop()
{
  curMillis = millis();

  updateClockState();

  delay(100);
}

void updateClockState()
{
  switch (clockState % 3)
  {
  case 0:
    showClock();
    if (lastClockStateChange + showClockDelay < curMillis)
    {
      clockState++;
      lastClockStateChange = curMillis;
      showDate();
    }
    break;
  case 1:
    if (lastClockStateChange + showDateDelay < curMillis)
    {
      clockState++;
      lastClockStateChange = curMillis;
      showWeather();
    }
    break;
  case 2:
    if (lastClockStateChange + showWeatherDelay < curMillis)
    {
      clockState++;
      lastClockStateChange = curMillis;
      showClock();
    }
    break;
  }
}

void showClock()
{
  rtcNow = rtc.now();
  showData(rtcNow.toString(timeFormat), rtcNow.second() % 2 == 0);
}

void showDate()
{
  rtcNow = rtc.now();
  showData(rtcNow.toString(dateFormat), true);
}

void showWeather()
{
  showData(" +23", false);
}

void showData(char str[4], bool showDot)
{
  lcd.setCursor(5, 1); // bottom right
  lcd.print('[');
  lcd.print(str[0]);
  lcd.print(str[1]);
  if (showDot)
  {
    lcd.print(".");
  }
  else
  {
    lcd.print(" ");
  }
  lcd.print(str[2]);
  lcd.print(str[3]);
  lcd.print(']');
}