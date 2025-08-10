// ======================================================================
//  Program Kalibrasi HC-SR04 dengan Regresi Linier pada ESP32
//  Output jarak dalam satuan cm
// ======================================================================

#define TRIG_PIN  12     // Sesuaikan dengan wiring kamu
#define ECHO_PIN  14    // Sesuaikan dengan wiring kamu

// Koefisien hasil kalibrasi regresi linier:
//   distance (cm) = slope * duration (µs) + intercept
const float slope     = 0.01679897486038059;
const float intercept = 0.42177218978540093;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println();
  Serial.println("=== HC-SR04 Calibrated Distance ===");
  Serial.println("Output: jarak (cm)");
  Serial.println("------------------------------------");
}

void loop() {
  long duration;
  float distanceCm;

  // --- Kirim pulsa trigger ---
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // --- Baca durasi echo (HIGH) dalam mikrodetik ---
  duration = pulseIn(ECHO_PIN, HIGH);

  // --- Hitung jarak (cm) pakai regresi linier ---
  distanceCm = slope * duration + intercept;

  // --- Tampilkan hasil ---
  Serial.print("Durasi: ");
  Serial.print(duration);
  Serial.print(" µs  |  Jarak: ");
  Serial.print(distanceCm, 2);  // dua angka di belakang koma
  Serial.println(" cm");

  delay(500);  // jeda setengah detik
}
