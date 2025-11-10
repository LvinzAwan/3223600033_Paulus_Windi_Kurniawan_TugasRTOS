#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 36
#define OLED_SCL 35
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
SemaphoreHandle_t dispMutex;
volatile int activeCore = 0; // default core 0

void oledWorker(void *p) {
  int myCore = (int)p;
  const char* txt0 = "RTOS Core0";
  const char* txt1 = "RTOS Core1";
  const char* txt = (myCore==0)?txt0:txt1;
  for (;;) {
    if (activeCore == myCore) {
      if (xSemaphoreTake(dispMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
        display.clearDisplay();
        display.setCursor(0, 20);
        for (int i=0; txt[i]; ++i) {
          display.print(txt[i]);
          display.display();
          vTaskDelay(pdMS_TO_TICKS(120));
        }
        xSemaphoreGive(dispMutex);
      }
      vTaskDelay(pdMS_TO_TICKS(500));
    } else {
      vTaskDelay(pdMS_TO_TICKS(200));
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 failed");
    while(1);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  dispMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(oledWorker, "OLED0", 3072, (void*)0, 1, NULL, 0);
  xTaskCreatePinnedToCore(oledWorker, "OLED1", 3072, (void*)1, NULL, NULL, 1);

  Serial.println("Core yang aktif = 0. Ketik '0' atau '1' di Serial.");
}

void loop() {
  if (Serial.available()) {
    String s = Serial.readStringUntil('\n');
    s.trim();
    if (s == "0") {
      activeCore = 0;
      Serial.println("Core yang aktif = 0");
    } else if (s == "1") {
      activeCore = 1;
      Serial.println("Core yang aktif = 1");
    } else {
      Serial.println("Gunakan '0' atau '1'");
    }
  }
  vTaskDelay(pdMS_TO_TICKS(50));
}
