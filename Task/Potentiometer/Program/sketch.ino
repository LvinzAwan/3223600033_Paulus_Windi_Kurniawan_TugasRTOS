#include <Arduino.h>

#define POT_PIN 14  
TaskHandle_t TaskReadPot;
TaskHandle_t TaskDisplayPot;
volatile int potValue = 0;
void readPotTask(void *pvParameters) {
  Serial.print("Task readPotTask berjalan di Core: ");
  Serial.println(xPortGetCoreID());
  analogReadResolution(12);
  analogSetPinAttenuation(POT_PIN, ADC_11db);

  for (;;) {
    potValue = analogRead(POT_PIN);
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void displayPotTask(void *pvParameters) {
  Serial.print("Task displayPotTask berjalan di Core: ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    Serial.print("[Core ");
    Serial.print(xPortGetCoreID());
    Serial.print("] Nilai Potensiometer (raw 0–4095): ");
    Serial.println(potValue);
    float pct = (potValue / 4095.0f) * 100.0f;
    Serial.print("Persentase: ");
    Serial.print(pct, 1);
    Serial.println(" %");
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(pdMS_TO_TICKS(1000));

  pinMode(POT_PIN, INPUT);

  xTaskCreatePinnedToCore(readPotTask, "ReadPot", 2048, NULL, 1, &TaskReadPot, 0); // Core 0
  xTaskCreatePinnedToCore(displayPotTask, "DisplayPot", 2048, NULL, 1, &TaskDisplayPot, 1); // Core 1

  Serial.println("\nTask Potensiometer dibuat di Core 0 & Core 1");
  Serial.println("Task readPotTask -> Core 0");
  Serial.println("Task displayPotTask -> Core 1");
  Serial.println("PASTIKAN SIG <= 3.3V! (gunakan VCC 3.3V atau divider)");
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
