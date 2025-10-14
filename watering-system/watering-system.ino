#define PUMP_PIN 10
#define VALVE1_PIN 9
#define VALVE2_PIN 8
#define VALVE3_PIN 7

const int valves[] = {VALVE1_PIN, VALVE2_PIN, VALVE3_PIN};
const int durations[] = {3000, 2000, 1000}; // valves open duration [ms]
const int valveCount = sizeof(valves) / sizeof(valves[0]);

void setup() {
  pinMode(PUMP_PIN, OUTPUT);
  for (int i = 0; i < valveCount; i++) {
    pinMode(valves[i], OUTPUT);
    digitalWrite(valves[i], LOW);
  }

  digitalWrite(PUMP_PIN, HIGH);

  for (int i = 0; i < valveCount; i++) {
    digitalWrite(valves[i], HIGH);
    delay(durations[i]);
    digitalWrite(valves[i], LOW);
    delay(100);
  }

  digitalWrite(PUMP_PIN, LOW);

  // Deep sleep 12h
  esp_sleep_enable_timer_wakeup(12ULL * 60 * 60 * 1000000)
  esp_deep_sleep_start();
}

void loop() {
}
