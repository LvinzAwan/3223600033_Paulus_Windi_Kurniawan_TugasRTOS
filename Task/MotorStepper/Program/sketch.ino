#include <AccelStepper.h>

#define STEP_PIN 13
#define DIR_PIN 12

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

volatile long targetSteps = 0;
volatile bool newCommand = false;
volatile int currentCore = 1; 
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

TaskHandle_t inputTaskHandle;
TaskHandle_t motorTaskHandle;

void setup() {
  Serial.begin(115200);
  delay(200);

  stepper.setMaxSpeed(800);
  stepper.setAcceleration(400);
  stepper.setCurrentPosition(0);

  xTaskCreatePinnedToCore(inputTask, "InputTask", 4096, NULL, 1, &inputTaskHandle, 0);
  xTaskCreatePinnedToCore(motorTask, "MotorTask", 4096, NULL, 1, &motorTaskHandle, currentCore);
}

void inputTask(void *pvParameters) {
  Serial.print("Input Task running on core ");
  Serial.println(xPortGetCoreID());

  String buffer = "";

  for (;;) {
    while (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        if (buffer.equalsIgnoreCase("core0") || buffer.equalsIgnoreCase("core1")) {
          vTaskDelete(motorTaskHandle);

          portENTER_CRITICAL(&mux);
          currentCore = buffer.endsWith("0") ? 0 : 1;
          portEXIT_CRITICAL(&mux);

          xTaskCreatePinnedToCore(motorTask, "MotorTask", 4096, NULL, 1, &motorTaskHandle, currentCore);
          Serial.printf("Motor task dipindah ke core %d\n", currentCore);

        } else if (buffer.length() > 0) {
          portENTER_CRITICAL(&mux);
          targetSteps = buffer.toInt();
          newCommand = true;
          portEXIT_CRITICAL(&mux);
          Serial.printf("Perintah %ld langkah diterima\n", targetSteps);
        }

        buffer = "";
      } else {
        buffer += c;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void motorTask(void *pvParameters) {
  Serial.print("Motor Task running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    if (newCommand) {
      long stepsToMove;
      portENTER_CRITICAL(&mux);
      stepsToMove = targetSteps;
      newCommand = false;
      portEXIT_CRITICAL(&mux);

      long targetPos = stepper.currentPosition() + stepsToMove;
      Serial.printf("Bergerak %ld langkah ke posisi %ld...\n", stepsToMove, targetPos);
      stepper.moveTo(targetPos);
    }

    stepper.run();
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(10));
}
