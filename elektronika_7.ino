#include <LedControl.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

#define showClockDelay 10000
#define showWeatherDelay 3000
#define showDateDelay 3000

const char fullDateFormat[] = "DD.MM.YYYY hh:mm:ss";
const char dateFormat[] = "DDMM";
const char timeFormat[] = "hhmm";

LedControl lc = LedControl(8, 10, 9, 1);  // DIN, CLK, CS, num | layout: fc16
LiquidCrystal_I2C lcd(0x3F, 16, 2);       // custom address!
RTC_DS3231 rtc;                           // in real project using DS3231 instead of DS1307

DateTime rtcNow;
uint32_t curMillis = 0;
uint32_t lastClockStateChange = 0;
uint32_t lastSetStateChange = 0;
uint8_t clockState = 0;
uint8_t setState = 0;

uint8_t matrixState[8] = { 60, 66, 165, 129, 165, 153, 66, 60 };  // ☺

void setup() {
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

  printMatrix();

  // 0: 31, 17, 31
  // 1: 0, 0 (8?), 31
  // 2: 23, 21, 29
  // 3: 17, 21, 31
  // 4: 28, 4, 31
  // 8: 31, 21, 31
}

void loop() {
  curMillis = millis();

  updateClockState();

  // TODO: updateSetState();

  delay(100);
}

void updateClockState() {
  switch (clockState % 3) {
    case 0:
      showClock();
      if (lastClockStateChange + showClockDelay < curMillis) {
        clockState++;
        lastClockStateChange = curMillis;
        showDate();
      }
      break;
    case 1:
      if (lastClockStateChange + showDateDelay < curMillis) {
        clockState++;
        lastClockStateChange = curMillis;
        showWeather();
      }
      break;
    case 2:
      if (lastClockStateChange + showWeatherDelay < curMillis) {
        clockState++;
        lastClockStateChange = curMillis;
        showClock();
      }
      break;
  }
}

void showClock() {
  rtcNow = rtc.now();
  showData(rtcNow.toString(timeFormat), rtcNow.second() % 2 == 0);
}

void showDate() {
  rtcNow = rtc.now();
  showData(rtcNow.toString(dateFormat), true);
}

void showWeather() {
  int8_t curTemp = (int)rtc.getTemperature();
  // 1) добавить 100 битовым сдвигом: -103 / 103
  // 2) превратить в строку, удалив 1, добавив + и пробел: " -03" / " +03"
  // TODO: use rtc.getTemperature()

  showData(" +23", false);
}

void showData(char str[4], bool showDot) {
  // TODO: use led matrix
  lcd.setCursor(5, 1);
  lcd.print('[');
  lcd.print(str[0]);
  lcd.print(str[1]);
  if (showDot) {
    lcd.print(".");
  } else {
    lcd.print(" ");
  }
  lcd.print(str[2]);
  lcd.print(str[3]);
  lcd.print(']');
}

void printMatrix() {
  for (uint8_t i = 0; i < 8; i++) {
    lc.setRow(0, i, matrixState[i]);
  }
}