// ESP32-S3 dual-core button example (ringkas)
// Button1 -> GPIO20 (pinned to core 0)
// Button2 -> GPIO21 (pinned to core 1)
// Buttons wired to GND; use INPUT_PULLUP (pressed = LOW)

const int button1Pin = 20;
const int button2Pin = 21;
const unsigned long DEBOUNCE_MS = 50;

void reportPress(int btnNum, int pin, int state) {
  // state: LOW = pressed, HIGH = released
  if (state == LOW) {
    Serial.printf("[Core %d] Button %d PRESSED (pin %d) at %lu ms\n", xPortGetCoreID(), btnNum, pin, millis());
  } else {
    Serial.printf("[Core %d] Button %d RELEASED (pin %d) at %lu ms\n", xPortGetCoreID(), btnNum, pin, millis());
  }
}

void taskBtn1(void* pv) {
  int last = HIGH, stable = HIGH;
  unsigned long t = 0;
  for (;;) {
    int r = digitalRead(button1Pin);
    if (r != last) t = millis();
    if ((millis() - t) > DEBOUNCE_MS && r != stable) {
      stable = r;
      reportPress(1, button1Pin, stable);
    }
    last = r;
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void taskBtn2(void* pv) {
  int last = HIGH, stable = HIGH;
  unsigned long t = 0;
  for (;;) {
    int r = digitalRead(button2Pin);
    if (r != last) t = millis();
    if ((millis() - t) > DEBOUNCE_MS && r != stable) {
      stable = r;
      reportPress(2, button2Pin, stable);
    }
    last = r;
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup() {
  Serial.begin(115200);
  delay(50);
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  xTaskCreatePinnedToCore(taskBtn1, "BTN1", 2048, NULL, 1, NULL, 0); // core 0
  xTaskCreatePinnedToCore(taskBtn2, "BTN2", 2048, NULL, 1, NULL, 1); // core 1
  Serial.println("Dual-core button demo started (btn1->core0, btn2->core1).");
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
