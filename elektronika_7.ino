#include <LedControl.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

#define showTimeDelay 10000
#define showTempDelay 3000
#define showDateDelay 3000

const char fullDateFormat[] = "DD.MM.YYYY hh:mm:ss";
const char dateFormat[] = "DDMM";
const char timeFormat[] = "hhmm";

LedControl lc = LedControl(8, 10, 9, 1);  // DIN, CLK, CS, num | layout: fc16
LiquidCrystal_I2C lcd(0x3F, 16, 2);       // custom I2C address
RTC_DS3231 rtc;                           // in real project using DS3231 instead of DS1307

DateTime rtcNow;
uint32_t curMillis = 0;
uint32_t lastViewStateChange = 0;
uint32_t lastSetStateChange = 0;
uint8_t viewState = 0;
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

  updateClockView();
  updateClockState();

  delay(100);
}

void updateClockView() {
  switch (viewState % 3) {
    case 0:
      showTime();
      if (lastViewStateChange + showTimeDelay < curMillis) {
        viewState++;
        lastViewStateChange = curMillis;
        showDate();
      }
      break;
    case 1:
      if (lastViewStateChange + showDateDelay < curMillis) {
        viewState++;
        lastViewStateChange = curMillis;
        showWeather();
      }
      break;
    case 2:
      if (lastViewStateChange + showTempDelay < curMillis) {
        viewState++;
        lastViewStateChange = curMillis;
        showTime();
      }
      break;
  }
}

void updateClockState() {
  // TODO: написать логику установки времени и даты на часах
  // switch (setState)
}

void showTime() {
  rtcNow = rtc.now();
  showData(rtcNow.toString(timeFormat), rtcNow.second() % 2 == 0);
}

void showDate() {
  rtcNow = rtc.now();
  showData(rtcNow.toString(dateFormat), true);
}

void showWeather() {
  float fTemp = rtc.getTemperature();
  uint8_t iTemp = round(abs(fTemp));
  char sTemp[5];

  if (iTemp == 0) {
    snprintf(sTemp, 5, "  0C");
  } else if (iTemp < 10) {
    snprintf(sTemp, 5, " %c%d%c", fTemp < 0 ? '-' : '+', iTemp, 'C');
  } else {
    snprintf(sTemp, 5, " %c%02d", fTemp < 0 ? '-' : '+', iTemp);
  }
  showData(sTemp, false);
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