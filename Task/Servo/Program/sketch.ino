#include <ESP32Servo.h>

#define SERVO_PIN 5
Servo myservo;

volatile int targetAngle = 0;
volatile bool newCommand = false;
volatile int currentCore = 1;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

TaskHandle_t inputTaskHandle;
TaskHandle_t servoTaskHandle;

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("\nDual Core Servo Controller (ESP32)");
  Serial.println("Ketik sudut servo (0–180)");
  Serial.println("Ketik core0/core1 untuk pindah core servo\n");

  xTaskCreatePinnedToCore(inputTask, "InputTask", 4096, NULL, 1, &inputTaskHandle, 0);
  xTaskCreatePinnedToCore(servoTask, "ServoTask", 4096, NULL, 1, &servoTaskHandle, currentCore);
}

void inputTask(void *pvParameters) {
  Serial.print("Input Task core ");
  Serial.println(xPortGetCoreID());
  String buffer = "";
  for (;;) {
    while (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        if (buffer.equalsIgnoreCase("core0") || buffer.equalsIgnoreCase("core1")) {
          vTaskDelete(servoTaskHandle);
          portENTER_CRITICAL(&mux);
          currentCore = buffer.endsWith("0") ? 0 : 1;
          portEXIT_CRITICAL(&mux);
          xTaskCreatePinnedToCore(servoTask, "ServoTask", 4096, NULL, 1, &servoTaskHandle, currentCore);
          Serial.printf("Servo task dipindah ke core %d\n", currentCore);
        } 
        else if (buffer.length() > 0) {
          int angle = buffer.toInt();
          if (angle >= 0 && angle <= 180) {
            portENTER_CRITICAL(&mux);
            targetAngle = angle;
            newCommand = true;
            portEXIT_CRITICAL(&mux);
            Serial.printf("Perintah sudut %d diterima\n", angle);
          } else {
            Serial.println("Masukkan nilai 0–180 saja!");
          }
        }
        buffer = "";
      } else buffer += c;
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void servoTask(void *pvParameters) {
  Serial.print("Servo Task core ");
  Serial.println(xPortGetCoreID());
  myservo.setPeriodHertz(50);
  myservo.attach(SERVO_PIN, 500, 2400);
  int currentAngle = 0;
  for (;;) {
    if (newCommand) {
      int target;
      portENTER_CRITICAL(&mux);
      target = targetAngle;
      newCommand = false;
      portEXIT_CRITICAL(&mux);
      Serial.printf("Servo ke %d°\n", target);
      int step = (target > currentAngle) ? 1 : -1;
      for (int pos = currentAngle; pos != target; pos += step) {
        myservo.write(pos);
        vTaskDelay(pdMS_TO_TICKS(15));
      }
      currentAngle = target;
      Serial.printf("Selesai di %d°\n\n", currentAngle);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(10));
}
