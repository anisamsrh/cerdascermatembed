#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd1(0x27, 16, 2); 
LiquidCrystal_I2C lcd2(0x26, 16, 2);
LiquidCrystal_I2C lcd3(0x25, 16, 2);

#define LED1 26
#define LED2 27
#define LED3 14
#define BUZZ 12

#define BTN1 32
#define BTN2 33
#define BTN3 25

#define JURI_BENAR 13
#define JURI_SALAH 4

int skor[3] = {0, 0, 0};
const int STATE_STANDBY = -1;
int aktifTim = STATE_STANDBY;
bool timDiskualifikasi[3] = {false, false, false};
int jumlahSalah = 0;

volatile bool btn1Pressed = false;
volatile bool btn2Pressed = false;
volatile bool btn3Pressed = false;
volatile bool juriBenarPressed = false;
volatile bool juriSalahPressed = false;

void IRAM_ATTR onBtn1() { btn1Pressed = true; }
void IRAM_ATTR onBtn2() { btn2Pressed = true; }
void IRAM_ATTR onBtn3() { btn3Pressed = true; }
void IRAM_ATTR onJuriBenar() { juriBenarPressed = true; }
void IRAM_ATTR onJuriSalah() { juriSalahPressed = true; }

void setup() {
  Serial.begin(115200);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(BUZZ, OUTPUT);

  pinMode(BTN1, INPUT_PULLDOWN);
  pinMode(BTN2, INPUT_PULLDOWN);
  pinMode(BTN3, INPUT_PULLDOWN);
  pinMode(JURI_BENAR, INPUT_PULLDOWN);
  pinMode(JURI_SALAH, INPUT_PULLDOWN);

  attachInterrupt(digitalPinToInterrupt(BTN1), onBtn1, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN2), onBtn2, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN3), onBtn3, RISING);
  attachInterrupt(digitalPinToInterrupt(JURI_BENAR), onJuriBenar, RISING);
  attachInterrupt(digitalPinToInterrupt(JURI_SALAH), onJuriSalah, RISING);

  Wire.begin(21, 22); 
  lcd1.init(); lcd1.backlight();
  Serial.println("LCD1 OK");
  lcd2.init(); lcd2.backlight();
  Serial.println("LCD2 OK");
  lcd3.init(); lcd3.backlight();
  Serial.println("LCD3 OK");
  resetSkorDanDisplay();
}

void loop() {
  if (aktifTim == STATE_STANDBY) {
    timAktif();
  }

  if (aktifTim != STATE_STANDBY) {
    juriAktif();
  }
}

void timAktif() {
    if (btn1Pressed && !timDiskualifikasi[0]) { setAktif(0); btn1Pressed = false; }
    if (btn2Pressed && !timDiskualifikasi[1]) { setAktif(1); btn2Pressed = false; }
    if (btn3Pressed && !timDiskualifikasi[2]) { setAktif(2); btn3Pressed = false; }
}

void juriAktif() {
    if (juriBenarPressed) {
      skor[aktifTim] += 10;
      tone(BUZZ, 1500, 100); tone(BUZZ, 2000, 150);
      juriBenarPressed = false;
      resetSystem();
    }
    if (juriSalahPressed) {
      skor[aktifTim] -= 5;
      tone(BUZZ, 500, 500);
      timDiskualifikasi[aktifTim] = true;
      aktifTim = STATE_STANDBY;
      jumlahSalah++;

      if (jumlahSalah >=2) {
        resetSystem(); 
      } else {
        updateLedsForStandby(); 
      }

      juriSalahPressed = false;
    }
}

void setAktif(int tim) {
  aktifTim = tim;
  digitalWrite(LED1, (tim == 0) ? HIGH : LOW);
  digitalWrite(LED2, (tim == 1) ? HIGH : LOW);
  digitalWrite(LED3, (tim == 2) ? HIGH : LOW);
  tone(BUZZ, 1000, 400);
}

void updateLedsForStandby() {
  digitalWrite(LED1, !timDiskualifikasi[0]);
  digitalWrite(LED2, !timDiskualifikasi[1]);
  digitalWrite(LED3, !timDiskualifikasi[2]);
}

void resetSystem() {
  aktifTim = STATE_STANDBY;
  for (int i = 0; i < 3; i++) timDiskualifikasi[i] = false;
  jumlahSalah = 0;
  updateLedsForStandby();
  updateAllDisplays();
}

void resetSkorDanDisplay() {
  for (int i = 0; i < 3; i++) skor[i] = 0;
  resetSystem();
}

void updateLcd(LiquidCrystal_I2C &lcd, int timIndex) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tim ");
  lcd.print(timIndex + 1);
  lcd.setCursor(0, 1);
  lcd.print("Skor: ");
  lcd.print(skor[timIndex]);
}

void updateAllDisplays() {
  updateLcd(lcd1, 0);
  updateLcd(lcd2, 1);
  updateLcd(lcd3, 2);
}

