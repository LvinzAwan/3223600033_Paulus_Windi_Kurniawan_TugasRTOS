#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
#include <AccelStepper.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Servo myservo;
AccelStepper stepper(AccelStepper::DRIVER, 13, 14);

#define LED_PIN 5
#define BUTTON_PIN 6
#define BUZZ_PIN 7
#define POT_PIN 3
#define SERVO_PIN 12
#define CLK_PIN 10
#define DT_PIN 11
#define STEP_PIN 13
#define DIR_PIN 14

SemaphoreHandle_t xMutex;
volatile unsigned long core0TaskCount = 0;
volatile unsigned long core1TaskCount = 0;
volatile int potValue = 0;
volatile bool buttonPressed = false;
volatile int encoderCount = 0;
int lastStateCLK = 0;
volatile bool newServoCmd = false;
volatile bool newStepperCmd = false;
volatile int servoAngle = 0;
volatile long stepTarget = 0;

TaskHandle_t taskLEDHandle;
TaskHandle_t taskButtonHandle;
TaskHandle_t taskOLEDHandle;
TaskHandle_t taskServoHandle;
TaskHandle_t taskInputHandle;
TaskHandle_t taskEncoderHandle;
TaskHandle_t taskStepperHandle;
TaskHandle_t taskPotHandle;

void TaskLED(void *pv) {
  pinMode(LED_PIN, OUTPUT);
  while (1) {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(300 / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(300 / portTICK_PERIOD_MS);
    if (xSemaphoreTake(xMutex, 10) == pdTRUE) {
      core0TaskCount++;
      xSemaphoreGive(xMutex);
    }
  }
}

void beepBuzzer(int freq, int duration_ms) {
  long delayMicro = 1000000L / (freq * 2);
  long cycles = (long)freq * duration_ms / 1000;
  for (long i = 0; i < cycles; i++) {
    digitalWrite(BUZZ_PIN, HIGH);
    delayMicroseconds(delayMicro);
    digitalWrite(BUZZ_PIN, LOW);
    delayMicroseconds(delayMicro);
  }
}

void TaskButton(void *pv) {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZ_PIN, OUTPUT);
  bool lastState = HIGH;
  while (1) {
    bool currentState = digitalRead(BUTTON_PIN);
    if (currentState == LOW && lastState == HIGH) {
      if (xSemaphoreTake(xMutex, 10) == pdTRUE) {
        buttonPressed = !buttonPressed;
        xSemaphoreGive(xMutex);
      }
      Serial.printf("[BUTTON] Pressed, buzzer: %s\n", buttonPressed ? "ON" : "OFF");
    }

    if (buttonPressed) {
      beepBuzzer(2000, 100);  // bunyi 2kHz 100 ms
      vTaskDelay(100 / portTICK_PERIOD_MS);
    } else {
      digitalWrite(BUZZ_PIN, LOW);
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    lastState = currentState;
  }
}

void TaskOLED(void *pv) {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) vTaskDelete(NULL);
  display.clearDisplay();
  display.display();

  unsigned long lastPrint = 0;

  while (1) {
    int localPot, localEnc;
    unsigned long c0, c1;
    bool btn;
    if (xSemaphoreTake(xMutex, 10) == pdTRUE) {
      localPot = potValue;
      localEnc = encoderCount;
      c0 = core0TaskCount;
      c1 = core1TaskCount;
      btn = buttonPressed;
      xSemaphoreGive(xMutex);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("ESP32-S3 DualCore");
    display.setCursor(0, 12);
    display.printf("Core0:%lu  Core1:%lu\n", c0, c1);
    display.printf("POT:%d ENC:%d\n", localPot, localEnc);
    display.printf("BTN:%s\n", btn ? "ON" : "OFF");
    display.display();

    if (millis() - lastPrint >= 1000) {
      Serial.printf("[MONITOR] Core0:%lu Core1:%lu | POT:%d | ENC:%d | BTN:%s\n",
                    c0, c1, localPot, localEnc, btn ? "ON" : "OFF");
      lastPrint = millis();
    }

    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void TaskServo(void *pv) {
  myservo.setPeriodHertz(50);
  myservo.attach(SERVO_PIN, 500, 2400);
  int pos = 0;
  while (1) {
    if (newServoCmd) {
      if (xSemaphoreTake(xMutex, 10) == pdTRUE) {
        pos = servoAngle;
        newServoCmd = false;
        xSemaphoreGive(xMutex);
      }
      myservo.write(pos);
      Serial.printf("[SERVO] Moved to %d°\n", pos);
    }
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void TaskEncoder(void *pv) {
  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);
  lastStateCLK = digitalRead(CLK_PIN);
  while (1) {
    int currentState = digitalRead(CLK_PIN);
    if (currentState != lastStateCLK) {
      if (xSemaphoreTake(xMutex, 10) == pdTRUE) {
        encoderCount += (digitalRead(DT_PIN) != currentState) ? 1 : -1;
        xSemaphoreGive(xMutex);
      }
      Serial.printf("[ENCODER] Count: %d\n", encoderCount);
    }
    lastStateCLK = currentState;
    vTaskDelay(2 / portTICK_PERIOD_MS);
  }
}

void TaskStepper(void *pv) {
  stepper.setMaxSpeed(800);
  stepper.setAcceleration(400);
  stepper.setPinsInverted(true, false, false);  // membalik arah gerak
  while (1) {
    if (newStepperCmd) {
      if (xSemaphoreTake(xMutex, 10) == pdTRUE) {
        stepper.moveTo(stepTarget);
        newStepperCmd = false;
        xSemaphoreGive(xMutex);
      }
      Serial.printf("[STEPPER] Move to position: %ld\n", stepTarget);
    }
    stepper.run();
    vTaskDelay(1);
  }
}

void TaskPot(void *pv) {
  while (1) {
    int raw = analogRead(POT_PIN);
    if (xSemaphoreTake(xMutex, 10) == pdTRUE) {
      potValue = raw;
      xSemaphoreGive(xMutex);
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void TaskInput(void *pv) {
  String input = "";
  while (1) {
    while (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        if (input.startsWith("servo")) {
          int val = input.substring(5).toInt();
          if (val >= 0 && val <= 180) {
            if (xSemaphoreTake(xMutex, 10) == pdTRUE) {
              servoAngle = val;
              newServoCmd = true;
              xSemaphoreGive(xMutex);
            }
            Serial.printf("[INPUT] Servo set to %d°\n", val);
          }
        } else if (input.startsWith("step")) {
          long val = input.substring(4).toInt();
          if (xSemaphoreTake(xMutex, 10) == pdTRUE) {
            stepTarget = val;
            newStepperCmd = true;
            xSemaphoreGive(xMutex);
          }
          Serial.printf("[INPUT] Stepper move %ld steps\n", val);
        }
        input = "";
      } else input += c;
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  xMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(TaskLED, "LED", 2048, NULL, 1, &taskLEDHandle, 0);
  xTaskCreatePinnedToCore(TaskButton, "Button", 2048, NULL, 1, &taskButtonHandle, 0);
  xTaskCreatePinnedToCore(TaskOLED, "OLED", 4096, NULL, 1, &taskOLEDHandle, 0);
  xTaskCreatePinnedToCore(TaskServo, "Servo", 4096, NULL, 1, &taskServoHandle, 0);
  xTaskCreatePinnedToCore(TaskInput, "Input", 4096, NULL, 1, &taskInputHandle, 0);

  xTaskCreatePinnedToCore(TaskEncoder, "Encoder", 2048, NULL, 1, &taskEncoderHandle, 1);
  xTaskCreatePinnedToCore(TaskStepper, "Stepper", 4096, NULL, 1, &taskStepperHandle, 1);
  xTaskCreatePinnedToCore(TaskPot, "Pot", 2048, NULL, 1, &taskPotHandle, 1);

  Serial.println("\n[SETUP DONE] Dual-core system started.\n");
}

void loop() {
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
