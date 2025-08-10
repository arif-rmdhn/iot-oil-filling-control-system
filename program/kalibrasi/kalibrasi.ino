#define TRIG_PIN 13
#define ECHO_PIN 14
float duration_us, distance_cm;

const int num_index = 10;

float get_average_data[num_index], data1 = 0, data2 = 0, realData, realData2;
int readIndex = 0;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  for (int i = 0; i < num_index; i++) {
    get_average_data[i] = 0;
  }
}

void loop() {
  read_sensor();
  delay(100);
}

void read_sensor() {
  while (readIndex < num_index) {

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    duration_us = pulseIn(ECHO_PIN, HIGH);
    distance_cm = 0.0343 * duration_us / 2;

    get_average_data[readIndex] = distance_cm;
    data1 += distance_cm;
    readIndex += 1;
    delay(100);
  }

  for (int i = 0; i < num_index; i++) {
    data2 += get_average_data[i];
  }
  data2 = data2 / readIndex;
  data2 = 14.5 - data2;
  realData = data2*9.3*9.3;

  Serial.println("--------------------------------");
  Serial.println(data2);
  Serial.println(realData);
  Serial.println("--------------------------------");
  for (int i = 0; i < num_index; i++) {
    get_average_data[i] = 0;
  }
  readIndex = 0;
  data1 = 0;
  data2 = 0;
}