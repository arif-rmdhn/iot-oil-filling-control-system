#define BLYNK_TEMPLATE_ID "TMPL6CY4DPSlH"
#define BLYNK_TEMPLATE_NAME "Project IoT"
#define BLYNK_AUTH_TOKEN "WSYu5Z2ynwanSlBLXNSER04wm8jLZVP4"

#include <WiFi.h>
#include <Keypad.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>

#define TRIG_PIN 12
#define ECHO_PIN 14

const byte ROWS = 4;  // 4 baris
const byte COLS = 4;  // 4 kolom

char lastKey = NO_KEY;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;  // 50 ms debounce

// const float slope = 0.01679897486038059;
// const float intercept = 0.42177218978540093;

const float slope = 0.017031406;
const float intercept = 0.098318307;

const float SLOPE_REGRESI = -89.93;       // Gradien (m) dari model regresi
const float INTERCEPT_REGRESI = 1331.86;  // Konstanta (c) atau volume maks (ml)

char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte pin_rows[ROWS] = { 25, 26, 19, 18 };
byte pin_column[COLS] = { 5, 17, 16, 4 };
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), pin_rows, pin_column, ROWS, COLS);

byte smiley[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b10001,
  0b01110,
  0b00000,
  0b00000
};

char ssid[] = "pitt";        // Masukkan nama wifi
char pass[] = "ffff123456";  // Masukkan password wifi

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

const int num_index = 5, relay_pump = 23, relay_pump2 = 32;
float duration_us, distance_cm, data_prev_sen, data_prev_SP, get_average_data[num_index], data1 = 0, data2 = 0, realData1, realData2, smtr, volume_ml;
int readIndex = 0;
int set_point_high = 500, set_point_low = 0, condition = 1;
char data_Keypad;
int status_pump1 = 0, interupsi = 1, interupsi_prev = 0, status_pump2 = 0, prev_status_pump2 = 0, prev_status_pump1 = 0;
String get_keyPad = "";

void dataJarak(float data) {
  Blynk.virtualWrite(V0, data);
}
void pump_stat(bool data) {
  Blynk.virtualWrite(V8, data);
}
void interupsi_(int data) {
  Blynk.virtualWrite(V13, data);
}
void pump_stat2(bool data) {
  Blynk.virtualWrite(V11, data);
  Blynk.virtualWrite(V12, status_pump2);
}
void dataSetpoint_high(int data) {
  Blynk.virtualWrite(V9, data);
}
void dataSetpoint_low(int data) {
  Blynk.virtualWrite(V4, data);
}
void alarm(bool data) {
  Blynk.virtualWrite(V2, data);
}
void stat_mode_1(int data) {
  Blynk.virtualWrite(V5, data);
}
void stat_mode_2(int data) {
  Blynk.virtualWrite(V6, data);
}

void mode_(int data) {
  int send1, send2;
  if (data == 1) {
    send1 = 1;
    send2 = 0;
    stat_mode_2(send2);
    stat_mode_1(send1);
  } else if (data == 2) {
    send1 = 0;
    send2 = 1;
    stat_mode_2(send2);
    stat_mode_1(send1);
  }
  Blynk.virtualWrite(V7, send1);
  Blynk.virtualWrite(V10, send2);
}

BLYNK_WRITE(V13) {
  int pinValue = param.asInt();
  if (pinValue == interupsi_prev) {
    if (pinValue == 1) {
      interupsi = 0;
    } else if (pinValue == 0) {
      interupsi = 1;
    }
    validation_pump1();
  }
}
BLYNK_WRITE(V12) {
  int pinValue = param.asInt();
  if (pinValue != prev_status_pump2) {
    status_pump2 = pinValue;
    validation_pump2();
  }
}
BLYNK_WRITE(V7) {
  int pinValue = param.asInt();
  if (pinValue == 1) {
    condition = 1;
  } else {
    condition = 2;
  }
  mode_(condition);
}
BLYNK_WRITE(V10) {
  int pinValue = param.asInt();
  if (pinValue == 1) {
    condition = 2;
  } else {
    condition = 1;
  }
  mode_(condition);
}

BLYNK_WRITE(V1) {
  int pinValue = param.asInt();

  set_point_high = pinValue;
  dataSetpoint_high(set_point_high);
  Blynk.virtualWrite(V1, 0);
}

BLYNK_WRITE(V3) {
  int pinValue = param.asInt();

  set_point_low = pinValue;
  dataSetpoint_low(set_point_low);
  Blynk.virtualWrite(V3, 0);
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, smiley);

  lcd.setCursor(1, 0);
  lcd.print("SELAMAT DATANG");
  lcd.setCursor(1, 1);
  lcd.print("~SEPTIAN HADI~");
  delay(5000);
  lcd.clear();

  pinMode(relay_pump, OUTPUT);
  pinMode(relay_pump2, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    lcd.setCursor(3, 0);
    lcd.print("CONNECTING");
    lcd.setCursor(4, 1);
    lcd.print("TO WIFI!");
    delay(500);
    lcd.clear();
    delay(500);
  }
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("CONNECTED");
  lcd.setCursor(2, 1);
  lcd.print("SUCCESSFULLY");
  delay(2000);
  lcd.clear();

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  for (int i = 0; i < num_index; i++) {
    get_average_data[i] = 0;
  }
  stat_mode_1(1);
  stat_mode_2(0);
  mode_(condition);
  pump_stat2(status_pump2);
  dataSetpoint_high(set_point_high);
  dataSetpoint_low(set_point_low);
}

void loop() {
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(relay_pump, 0);
    WiFi.begin(ssid, pass);
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("RECONNECTING");
    lcd.setCursor(4, 1);
    lcd.print("TO WIFI!");
    delay(500);
    lcd.clear();
    delay(500);
  }

  Blynk.run();
  read_keyPad();
  read_sensor();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Volume:" + String(data_prev_sen) + "ml");
  cek_kondisi();
}

void read_sensor() {
  while (readIndex < num_index) {
    read_keyPad();
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    duration_us = pulseIn(ECHO_PIN, HIGH);

    /* Tanpa Menggunakan Regresi Liniear */
    // distance_cm = 0.0343 * duration_us / 2;


    /* Menggunakan Regresi Liniear */
    distance_cm = slope * duration_us + intercept;


    get_average_data[readIndex] = distance_cm;

    readIndex += 1;
    delay(50);
  }

  for (int i = 0; i < num_index; i++) {
    data1 += get_average_data[i];
  }
  data1 = data1 / readIndex;  // Nilai Rata-Rata
  // realData1 = 14.5 - data1;           // Konversi ke CM
  realData2 = SLOPE_REGRESI * data1 + INTERCEPT_REGRESI;
  ;  // Konversi ml | rumus volume
  /*
  Serial.println(data1);
  */
  realData2 = constrain(realData2, 0, INTERCEPT_REGRESI);
  if (data_prev_sen != realData2) {
    data_prev_sen = realData2;
    data_prev_sen = constrain(data_prev_sen, 0, 1200);
    dataJarak(data_prev_sen);
    // if (data_prev_sen <= 10 && status_pump2) status_pump2 = 0;  validation_pump2();
  }


  readIndex = 0;
  data1 = 0;
  data2 = 0;

  for (int i = 0; i < num_index; i++) {
    get_average_data[i] = 0;
  }
}

void cek_kondisi() {
  switch (condition) {
    case 1:
      cek_level_1();
      break;
    case 2:
      cek_level_2();
      break;
  }
  pump_stat(status_pump1);
}

void cek_level_1() {
  if (data_prev_sen < set_point_high && !interupsi) {
    status_pump1 = 1;
  } else if (data_prev_sen >= set_point_high && !interupsi) {
    status_pump1 = 0;
  } else {
    status_pump1 = 0;
  }

  digitalWrite(relay_pump, status_pump1);
  lcd.setCursor(0, 1);
  lcd.print("D:Set|SP:" + String(set_point_high) + "ml");
}

void cek_level_2() {
  if (!interupsi) {
    if (data_prev_sen <= set_point_low && !status_pump1) {
      status_pump1 = 1;
    } else if (data_prev_sen >= set_point_high && status_pump1) {
      status_pump1 = 0;
    }
  } else {
    status_pump1 = 0;
  }

  digitalWrite(relay_pump, status_pump1);
  lcd.setCursor(0, 1);
  lcd.print("HIGH:" + String(set_point_high) + "|LOW:" + String(set_point_low));
}

void validation_pump2() {
  if (status_pump2 != prev_status_pump2) {
    pump_stat2(status_pump2);
    digitalWrite(relay_pump2, status_pump2);
    prev_status_pump2 = status_pump2;
  }
}

void validation_pump1() {
  if (interupsi != interupsi_prev) {
    if (interupsi == 1) {
      interupsi_(0);
    } else if (interupsi == 0) {
      interupsi_(1);
    }
    interupsi_prev = interupsi;
  }
}

void read_keyPad() {
  data_Keypad = customKeypad.getKey();
  if (data_Keypad != NO_KEY) {
    if (data_Keypad != lastKey || (millis() - lastDebounceTime > debounceDelay)) {
      lastDebounceTime = millis();
      lastKey = data_Keypad;

      if (data_Keypad == 'D') {
        Setting_Mode();
      }
      if (data_Keypad == 'A') {
        data_Keypad = ' ';
        stat_mode_1(1);
        stat_mode_2(0);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Masuk ke Mode 1");
        delay(2000);
        lcd.clear();
        condition = 1;
        mode_(condition);
      }
      if (data_Keypad == 'B') {
        data_Keypad = ' ';
        status_pump1 = 1;
        stat_mode_1(0);
        stat_mode_2(1);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Masuk ke Mode 2");
        delay(2000);
        lcd.clear();
        condition = 2;
        mode_(condition);
      }
      if (data_Keypad == 'C' && !interupsi) {
        lcd.clear();
        lcd.setCursor(6, 0);
        lcd.print("STATUS");
        lcd.setCursor(0, 1);
        lcd.print("Pump Non Active");
        delay(2000);
        lcd.clear();
        data_Keypad = ' ';
        interupsi = 1;
        validation_pump1();
      }
      if (data_Keypad == 'C' && interupsi) {
        lcd.clear();
        lcd.setCursor(6, 0);
        lcd.print("STATUS");
        lcd.setCursor(1, 1);
        lcd.print("Pump is Active");
        delay(2000);
        lcd.clear();
        data_Keypad = ' ';
        interupsi = 0;
        validation_pump1();
      }
      if (data_Keypad == '#') {
        informasi_button();
      }
      if (data_Keypad == '*' && !status_pump2) {
        data_Keypad = ' ';
        status_pump2 = 1;
        validation_pump2();
        lcd.clear();
        lcd.setCursor(6, 0);
        lcd.print("STATUS");
        lcd.setCursor(0, 1);
        lcd.print("Pump 2 is Active");
        delay(2000);
        lcd.clear();
      }
      if (data_Keypad == '*' && status_pump2) {
        data_Keypad = ' ';
        status_pump2 = 0;
        validation_pump2();
        lcd.clear();
        lcd.setCursor(6, 0);
        lcd.print("STATUS");
        lcd.setCursor(0, 1);
        lcd.print("Pump 2 OFF");
        delay(2000);
        lcd.clear();
      }
    } else {
      lastKey = NO_KEY;
    }
  }
}

void informasi_button() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Informasi button:");
  for (int i = 0; i <= 9; i++) {
    if (i % 2 == 0) {
      lcd.setCursor(0, 1);
      lcd.print("                     ");
    } else {
      if (i == 1) {
        lcd.setCursor(0, 1);
        lcd.print("A: Mode 1 SP");
        delay(1500);
      } else if (i == 3) {
        lcd.setCursor(0, 1);
        lcd.print("B: Mode 2 SP");
        delay(1500);
      } else if (i == 5) {
        lcd.setCursor(0, 1);
        lcd.print("C:PUMP1 ON & OFF");
        delay(1500);
      } else if (i == 7) {
        lcd.setCursor(0, 1);
        lcd.print("D: Setting Mode");
        delay(1500);
      } else {
        lcd.setCursor(0, 1);
        lcd.print("*:PUMP2 ON & OFF");
        delay(1500);
      }
    }
  }
}

void Setting_Mode() {
  lcd.clear();

  if (condition == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Input Set Point:");
    lcd.setCursor(0, 1);
    lcd.print(get_keyPad + "ml |# : DONE");
    while (true) {
      data_Keypad = customKeypad.getKey();

      if (data_Keypad) {
        // ENTER
        if (data_Keypad == '#') {
          if (get_keyPad.length() > 0) {
            set_point_high = get_keyPad.toInt();
            dataSetpoint_high(set_point_high);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("INPUT SUCCESSFUL");
            lcd.setCursor(7, 1);
            lcd.write(0);
            lcd.write(0);
            delay(3000);
            get_keyPad = "";  // Reset input data
            break;
          } else {
            break;
          }
        } else if (data_Keypad == 'C') {
          break;
        }
        // DELETE
        else if (data_Keypad == '*') {
          if (get_keyPad.length() > 0) {
            get_keyPad.remove(get_keyPad.length() - 1);
            lcd.clear();
          }
        } else if (data_Keypad >= '0' && data_Keypad <= '9') {
          get_keyPad += data_Keypad;
          lcd.clear();
        } else {
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("HANYA ANGKA");
          lcd.setCursor(2, 1);
          lcd.print("YANG DITERIMA");
          delay(2000);
          lcd.clear();
        }

        lcd.setCursor(0, 0);
        lcd.print("Input Set Point:");
        lcd.setCursor(0, 1);
        lcd.print(get_keyPad + "ml |# : DONE");
      }
    }
  }

  if (condition == 2) {
    int step_stat = 1;
    int old_value = set_point_high;
    bool flag = 0;
    while (true) {
      data_Keypad = customKeypad.getKey();
      if (flag) break;
      switch (step_stat) {
        case 1:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Input SP HIGH:");
          lcd.setCursor(0, 1);
          lcd.print(get_keyPad + "ml |#: NEXT");
          while (true) {
            data_Keypad = customKeypad.getKey();

            if (data_Keypad) {
              // ENTER
              if (data_Keypad == '#') {
                if (get_keyPad.length() > 0) {
                  set_point_high = get_keyPad.toInt();
                  dataSetpoint_high(set_point_high);
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("INPUT SUCCESSFUL");
                  lcd.setCursor(7, 1);
                  lcd.write(0);
                  lcd.write(0);
                  delay(3000);
                  get_keyPad = "";  // Reset input data
                  step_stat = 2;
                  break;
                }
              }
              // DELETE
              else if (data_Keypad == '*') {
                if (get_keyPad.length() > 0) {
                  get_keyPad.remove(get_keyPad.length() - 1);
                  lcd.clear();
                }
                // CANCEL
              } else if (data_Keypad == 'C') {
                flag = 1;
                break;
              } else if (data_Keypad >= '0' && data_Keypad <= '9') {
                get_keyPad += data_Keypad;
                lcd.clear();
              } else {
                lcd.clear();
                lcd.setCursor(3, 0);
                lcd.print("HANYA ANGKA");
                lcd.setCursor(2, 1);
                lcd.print("YANG DITERIMA");
                delay(2000);
                lcd.clear();
              }

              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Input SP HIGH:");
              lcd.setCursor(0, 1);
              lcd.print(get_keyPad + "ml |#: NEXT");
            }
          }

        case 2:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Input SP LOW:");
          lcd.setCursor(0, 1);
          lcd.print(get_keyPad + "ml |#: DONE");
          while (true) {
            data_Keypad = customKeypad.getKey();

            if (data_Keypad) {
              // ENTER
              if (data_Keypad == '#') {
                if (get_keyPad.length() > 0) {
                  set_point_low = get_keyPad.toInt();
                  dataSetpoint_low(set_point_low);
                  lcd.clear();
                  lcd.setCursor(0, 0);
                  lcd.print("INPUT SUCCESSFUL");
                  lcd.setCursor(7, 1);
                  lcd.write(0);
                  lcd.write(0);
                  delay(3000);
                  get_keyPad = " ";  // Reset input data
                  step_stat = 0;
                  status_pump1 = 1;
                  flag = 1;
                  break;
                } else {
                  break;
                }
              }
              // DELETE
              else if (data_Keypad == '*') {
                if (get_keyPad.length() > 0) {
                  get_keyPad.remove(get_keyPad.length() - 1);
                  lcd.clear();
                }
              } else if (data_Keypad == 'C') {
                step_stat = 1;
                set_point_high = old_value;
                // break;

              } else if (data_Keypad >= '0' && data_Keypad <= '9') {
                get_keyPad += data_Keypad;
                lcd.clear();
              } else {
                lcd.clear();
                lcd.setCursor(3, 0);
                lcd.print("HANYA ANGKA");
                lcd.setCursor(2, 1);
                lcd.print("YANG DITERIMA");
                delay(2000);
                lcd.clear();
              }

              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Input SP LOW:");
              lcd.setCursor(0, 1);
              lcd.print(get_keyPad + "ml |#: DONE");
            }
          }
        default:
          break;
      }
    }
  }
  data_Keypad = ' ';
}