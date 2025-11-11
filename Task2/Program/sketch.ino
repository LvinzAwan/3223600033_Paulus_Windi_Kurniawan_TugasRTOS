#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
#include <AccelStepper.h>

// ==================== DISPLAY ====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ==================== PERIPHERALS ====================
Servo myservo;
// Stepper driver: STEP=13, DIR=14 (A4988/DRV8825 style)
AccelStepper stepper(AccelStepper::DRIVER, 13, 14);

// ==================== PINS ====================
#define BUTTON1_PIN 5
#define BUTTON2_PIN 6
#define BUZZ_PIN    7
#define POT_PIN     3
#define SERVO_PIN   18
#define CLK_PIN     10
#define DT_PIN      11
#define LED1_PIN    15
#define LED2_PIN    16
#define LED3_PIN    17

// ==================== GLOBALS ====================
volatile int  potValue = 0;
volatile long encoderCount = 0;

// ==================== TASKS ====================
void TaskLED(void *pv) {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  while (1) {
    digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
    vTaskDelay(pdMS_TO_TICKS(200));
    digitalWrite(LED2_PIN, !digitalRead(LED2_PIN));
    vTaskDelay(pdMS_TO_TICKS(300));
    digitalWrite(LED3_PIN, !digitalRead(LED3_PIN));
    vTaskDelay(pdMS_TO_TICKS(400));
  }
}

void TaskButton(void *pv) {
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  while (1) {
    if (!digitalRead(BUTTON1_PIN)) Serial.println("[BUTTON] 1 pressed");
    if (!digitalRead(BUTTON2_PIN)) Serial.println("[BUTTON] 2 pressed");
    vTaskDelay(pdMS_TO_TICKS(80));
  }
}

void TaskOLED(void *pv) {
  // Pastikan I2C sesuai wiring Wokwi (SDA=8, SCL=9)
  Wire.begin(8, 9);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) vTaskDelete(NULL);

  while (1) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 8);
    display.println("ESP32-S3 RTOS Demo");
    display.setCursor(0, 24);
    display.println("Paulus Windi");
    display.setCursor(0, 36);
    display.println("Kurniawan");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// -- Buzzer helper tanpa LEDC/tone()
static inline void buzzerBeep(unsigned freqHz, unsigned durMs) {
  if (freqHz == 0 || durMs == 0) return;
  pinMode(BUZZ_PIN, OUTPUT);
  unsigned long halfPeriodUs = 1000000UL / (freqHz * 2UL);
  unsigned long endTime = millis() + durMs;
  while (millis() < endTime) {
    digitalWrite(BUZZ_PIN, HIGH);
    delayMicroseconds(halfPeriodUs);
    digitalWrite(BUZZ_PIN, LOW);
    delayMicroseconds(halfPeriodUs);
    taskYIELD(); // beri kesempatan task lain
  }
}

void TaskBuzzer(void *pv) {
  pinMode(BUZZ_PIN, OUTPUT);
  while (1) {
    // Beep 150ms setiap 850ms (tidak pakai LEDC)
    buzzerBeep(1200, 150);
    vTaskDelay(pdMS_TO_TICKS(850));
  }
}

void TaskServo(void *pv) {
  Serial.printf("TaskServo on core %d\n", xPortGetCoreID());
  // Pastikan period 50Hz (servo) dan attach di pin 18
  myservo.setPeriodHertz(50);
  myservo.attach(SERVO_PIN, 500, 2400);

  // Sweep sederhana
  int angle = 0;
  while (1) {
    for (angle = 0; angle <= 180; angle += 5) {
      myservo.write(angle);
      vTaskDelay(pdMS_TO_TICKS(25));
    }
    for (angle = 180; angle >= 0; angle -= 5) {
      myservo.write(angle);
      vTaskDelay(pdMS_TO_TICKS(25));
    }
    vTaskDelay(pdMS_TO_TICKS(150));
  }
}

void TaskStepper(void *pv) {
  stepper.setMaxSpeed(800);      // langkah/detik
  stepper.setAcceleration(400);  // langkah/detik^2
  long pos = 0;
  while (1) {
    pos = (pos == 0) ? 800 : 0;
    stepper.moveTo(pos);
    while (stepper.distanceToGo()) {
      stepper.run();  // menghasilkan pulsa STEP via digitalWrite, bukan LEDC
      vTaskDelay(1);
    }
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void TaskEncoder(void *pv) {
  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN,  INPUT_PULLUP);
  int last = digitalRead(CLK_PIN);
  while (1) {
    int now = digitalRead(CLK_PIN);
    if (now != last) {
      encoderCount += (digitalRead(DT_PIN) != now) ? 1 : -1;
      Serial.printf("[ENC] %ld\n", encoderCount);
    }
    last = now;
    vTaskDelay(pdMS_TO_TICKS(2));
  }
}

void TaskPot(void *pv) {
  while (1) {
    int val = analogRead(POT_PIN);
    if (abs(val - potValue) > 40) {
      potValue = val;
      Serial.printf("[POT] %d\n", val);
    }
    vTaskDelay(pdMS_TO_TICKS(150));
  }
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== ESP32-S3 Dual-Core Priority & Stack Demo ===");

  int scenario = 3; // ubah ke 2 atau 3 untuk uji perbedaan

  if (scenario == 1) {
    Serial.println("Scenario 1: PRIORITAS & STACK IDEAL (semua lancar)");
    // Core 0: UI/I-O ringan
    xTaskCreatePinnedToCore(TaskLED,    "LED",    2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(TaskButton, "Button", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(TaskOLED,   "OLED",   4096, NULL, 1, NULL, 0);

    // Core 1: timing lebih kritis
    xTaskCreatePinnedToCore(TaskServo,   "Servo",   4096, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(TaskStepper, "Stepper", 4096, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(TaskEncoder, "Encoder", 2048, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(TaskPot,     "Pot",     2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(TaskBuzzer,  "Buzzer",  2048, NULL, 1, NULL, 1);
  }
  else if (scenario == 2) {
    Serial.println("Scenario 2: PRIORITAS CAMPUR (beberapa delay)");
    xTaskCreatePinnedToCore(TaskLED,     "LED",     2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(TaskButton,  "Button",  2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(TaskOLED,    "OLED",    2048, NULL, 4, NULL, 0);

    xTaskCreatePinnedToCore(TaskServo,   "Servo",   4096, NULL, 0, NULL, 1);
    xTaskCreatePinnedToCore(TaskStepper, "Stepper", 4096, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(TaskEncoder, "Encoder", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(TaskPot,     "Pot",     2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(TaskBuzzer,  "Buzzer",  2048, NULL, 4, NULL, 1);
  }
  else {
    Serial.println("Scenario 3: PRIORITAS RENDAH & STACK KECIL (macet)");
    xTaskCreatePinnedToCore(TaskLED,     "LED",     512, NULL, 0, NULL, 0);
    xTaskCreatePinnedToCore(TaskButton,  "Button",  512, NULL, 0, NULL, 0);
    xTaskCreatePinnedToCore(TaskOLED,    "OLED",    512, NULL, 0, NULL, 0);

    xTaskCreatePinnedToCore(TaskServo,   "Servo",   512, NULL, 0, NULL, 1);
    xTaskCreatePinnedToCore(TaskStepper, "Stepper", 512, NULL, 0, NULL, 1);
    xTaskCreatePinnedToCore(TaskEncoder, "Encoder", 512, NULL, 0, NULL, 1);
    xTaskCreatePinnedToCore(TaskPot,     "Pot",     512, NULL, 0, NULL, 1);
    xTaskCreatePinnedToCore(TaskBuzzer,  "Buzzer",  512, NULL, 0, NULL, 1);
  }

  Serial.println("===============================================\n");
}

// ==================== LOOP ====================
void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
