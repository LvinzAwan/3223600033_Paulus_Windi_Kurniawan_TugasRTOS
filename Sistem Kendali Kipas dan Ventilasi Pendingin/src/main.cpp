// ============================================================================
//  FULL RTOS COOLING SYSTEM DEMO (ESP32-S3)
//  FINAL FIXED VERSION
// ============================================================================

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
#include <AccelStepper.h>
#include <DHT.h>

// ---------------- PIN DEFINITIONS ----------------
#define DHT_PIN     4
#define DHT_TYPE    DHT22

#define ENC_CLK     10
#define ENC_DT      11
#define ENC_SW      2

#define POT_PIN     1

#define LED_GREEN   17
#define LED_YELLOW  16
#define LED_RED     15

#define BUZZ_PIN    7
#define SERVO_PIN   18

#define STEP_PIN    13
#define DIR_PIN     14

#define EMG_PIN     6

#define OLED_SDA    8
#define OLED_SCL    9

// ---------------- WORKING MODES ----------------
enum Mode { MODE_AUTO, MODE_MANUAL };
Mode currentMode = MODE_AUTO;

// ---------------- SCENARIOS ----------------
enum DemoScenario {
  SCENARIO_GOOD = 0,
  SCENARIO_NO_QUEUE,
  SCENARIO_NO_MUTEX,
  SCENARIO_NO_SEMAPHORE,
  SCENARIO_BAD_ISR,
  SCENARIO_NO_MULTICORE,
  SCENARIO_DEADLOCK,
  SCENARIO_STARVATION
};

volatile DemoScenario gScenario = SCENARIO_GOOD;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================
volatile bool emergency = false;
volatile long encoderCount = 0;

int potValue = 0;
float temperature_dht = 25.0f;
int ventManualBase = 0;
long encoderAtModeChange = 0;
bool justEnteredManual = false;

struct SharedState {
  float temp;
  int   fanSpeed;
  int   ventAngle;
  bool  warn;
  bool  danger;
  bool  emergency;
};
SharedState gState;

// ============================================================================
// OBJECTS
// ============================================================================
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
Servo ventServo;
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// ============================================================================
// QUEUES & SEMAPHORES
// ============================================================================
enum InputEvent { EVENT_TOGGLE_MODE };

QueueHandle_t qModeEvent = nullptr;
QueueHandle_t qTempVirtual = nullptr;

SemaphoreHandle_t mState = nullptr;
SemaphoreHandle_t sOLED = nullptr;

// ============================================================================
// TASK HANDLES
// ============================================================================
TaskHandle_t hTaskSensor;
TaskHandle_t hTaskInputUser;
TaskHandle_t hTaskControl;
TaskHandle_t hTaskFanMotor;
TaskHandle_t hTaskVentCtrl;
TaskHandle_t hTaskTempDisp;
TaskHandle_t hTaskStatusDisp;
TaskHandle_t hTaskAlarm;

// ============================================================================
// MENU
// ============================================================================
void printScenarioMenu() {
  Serial.println();
  Serial.println("==================================================");
  Serial.println("   DEMO RTOS - PILIH SKENARIO (0 - 7)");
  Serial.println("--------------------------------------------------");
  Serial.println(" 0 - GOOD");
  Serial.println(" 1 - NO_QUEUE");
  Serial.println(" 2 - NO_MUTEX");
  Serial.println(" 3 - NO_SEMAPHORE");
  Serial.println(" 4 - BAD_ISR");
  Serial.println(" 5 - NO_MULTICORE");
  Serial.println(" 6 - DEADLOCK");
  Serial.println(" 7 - STARVATION");
  Serial.println("==================================================");
  Serial.print("Pilih (0-7): ");
}

DemoScenario waitScenarioFromSerial() {
  printScenarioMenu();
  while (!Serial.available()) vTaskDelay(10);

  int sel = Serial.parseInt();
  Serial.println(sel);
  return (DemoScenario)sel;
}

// ============================================================================
// MUTEX STATE
// ============================================================================
SharedState readState() {
  if (gScenario == SCENARIO_NO_MUTEX) return gState;

  xSemaphoreTake(mState, portMAX_DELAY);
  SharedState s = gState;
  xSemaphoreGive(mState);
  return s;
}

void writeState(const SharedState &s) {
  if (gScenario == SCENARIO_NO_MUTEX) {
    gState = s;
    return;
  }
  xSemaphoreTake(mState, portMAX_DELAY);
  gState = s;
  xSemaphoreGive(mState);
}

// ============================================================================
// ISR EMERGENCY
// ============================================================================
void IRAM_ATTR emgISR() {
  if (gScenario == SCENARIO_BAD_ISR) {
    for (volatile int i = 0; i < 200000; i++) {}
    delayMicroseconds(50000);
  }

  static uint32_t last = 0;
  uint32_t now = millis();
  if (now - last > 200) {
    emergency = !emergency;
    last = now;
  }
}

// ============================================================================
// TASK SENSOR
// ============================================================================
void TaskSensor(void *pv) {
  dht.begin();
  float lastValid = 25.0f;

  while (1) {
    float t = dht.readTemperature();
    if (!isnan(t)) lastValid = t;

    temperature_dht = lastValid;

    vTaskDelay(100);
  }
}

// ============================================================================
// TASK INPUT USER
// ============================================================================
void TaskInputUser(void *pv) {

  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT,  INPUT_PULLUP);
  pinMode(ENC_SW,  INPUT_PULLUP);

  int lastCLK = digitalRead(ENC_CLK);
  int lastBtn = HIGH;
  uint32_t lastBtnTime = 0;

  while (1) {

    // ROTARY
    int nowCLK = digitalRead(ENC_CLK);
    if (nowCLK != lastCLK) {
      if (digitalRead(ENC_DT) != nowCLK) encoderCount++;
      else                               encoderCount--;
      lastCLK = nowCLK;
    }

    // BUTTON
    int btn = digitalRead(ENC_SW);
    if (btn == LOW && lastBtn == HIGH && millis() - lastBtnTime > 250) {

      if (gScenario == SCENARIO_NO_QUEUE) {
        // direct mode toggle
        Mode prev = currentMode;
        currentMode = (currentMode == MODE_AUTO ? MODE_MANUAL : MODE_AUTO);

        if (prev == MODE_AUTO && currentMode == MODE_MANUAL) {
          SharedState s = readState();
          ventManualBase = s.ventAngle;
          encoderAtModeChange = encoderCount;
          justEnteredManual = true;
        }
      } else {
        InputEvent ev = EVENT_TOGGLE_MODE;
        xQueueSend(qModeEvent, &ev, 0);
      }
      lastBtnTime = millis();
    }
    lastBtn = btn;
    potValue = analogRead(POT_PIN);
    vTaskDelay(5);
  }
}

// ============================================================================
// TASK CONTROL (FIXED)
// ============================================================================
void TaskControl(void *pv) {

  float tempVirtual = 25.0f;
  float tempBase    = 25.0f;
  float ventSmooth  = 0;

  uint32_t start = millis();

  while (1) {

    // DEADLOCK simulation
    if (gScenario == SCENARIO_DEADLOCK && 
      millis() - start > 5000) 
    {
      xSemaphoreTake(mState, portMAX_DELAY);
      while (1) vTaskDelay(1000);
    }

    SharedState s = readState();
    s.emergency = emergency;

    // ======================================================
    // MODE EVENT (queue)
    // ======================================================
    if (gScenario != SCENARIO_NO_QUEUE) {
      InputEvent ev;
      if (xQueueReceive(qModeEvent, &ev, pdMS_TO_TICKS(10)) == pdTRUE) {

        Mode prev = currentMode;
        currentMode = (currentMode == MODE_AUTO ? MODE_MANUAL : MODE_AUTO);

        if (prev == MODE_AUTO && currentMode == MODE_MANUAL) {
          ventManualBase = s.ventAngle;
          encoderAtModeChange = encoderCount;
          ventSmooth = ventManualBase;
          justEnteredManual = true;
        }
      }
    }

    // ======================================================
    // SUHU SENSOR LANGSUNG
    // ======================================================
    tempBase = temperature_dht;

    // model suhu virtual
    tempVirtual += (tempBase - tempVirtual) * 0.2f;
    if (s.fanSpeed > 40)  tempVirtual -= s.fanSpeed / 2500.0f;
    if (s.ventAngle > 60) tempVirtual -= s.ventAngle / 700.0f;
    if (s.fanSpeed < 30 && s.ventAngle < 20) tempVirtual += 0.015f;

    tempVirtual = constrain(tempVirtual, -10, 60);
    s.temp = tempVirtual;

    // kirim suhu virtual
    if (qTempVirtual) xQueueOverwrite(qTempVirtual, &tempVirtual);

    // ======================================================
    // EMERGENCY
    // ======================================================
    if (s.emergency) {
      s.fanSpeed  = 0;
      s.ventAngle = 0;
      s.danger = true;
      s.warn   = false;
    }
    else {

      // AUTO MODE
      if (currentMode == MODE_AUTO) {
        if      (tempVirtual < 20) s.ventAngle = 10;
        else if (tempVirtual > 40) s.ventAngle = 160;
        else s.ventAngle = 10 + (tempVirtual - 20) * 7.5f;

        if      (tempVirtual < 25) s.fanSpeed = 0;
        else if (tempVirtual > 40) s.fanSpeed = 800;
        else s.fanSpeed = map(tempVirtual * 10, 250, 400, 0, 800);
      }

      // MANUAL MODE
      else {

        // FAN manual
        if (potValue < 50) s.fanSpeed = 0;
        else               s.fanSpeed = map(potValue, 0, 4095, 0, 800);

        // VENT manual smooth
        if (justEnteredManual) {
          ventSmooth = ventManualBase;
          s.ventAngle = ventManualBase;
          justEnteredManual = false;
        } else {
          long delta = encoderCount - encoderAtModeChange;
          int rawOffset = map(delta, -50, 50, -60, 60);
          rawOffset = constrain(rawOffset, -60, 60);
          if (abs(rawOffset) < 3) rawOffset = 0;

          int target = constrain(ventManualBase + rawOffset, 0, 180);
          ventSmooth += (target - ventSmooth) * 0.25f;

          s.ventAngle = ventSmooth;
        }
      }
    }

    // ======================================================
    // FIX — UPDATE WARNING/DANGER DI PALING AKHIR
    // ======================================================
    if (!s.emergency) {
      s.danger = (tempVirtual >= 35);
      s.warn   = (tempVirtual >= 25 && tempVirtual < 35);
    }

    writeState(s);
    vTaskDelay(50);
  }
}

// ============================================================================
// TASK FAN MOTOR
// ============================================================================
void TaskFanMotor(void *pv) {

  stepper.setMaxSpeed(800);
  stepper.setAcceleration(400);
  stepper.setPinsInverted(true, false);

  float speedNow = 0;

  while (1) {

    SharedState s = readState();
    float target = s.emergency ? 0 : s.fanSpeed;

    speedNow += (target - speedNow) * 0.25f;

    stepper.setSpeed(speedNow);
    stepper.runSpeed();

    vTaskDelay(5);
  }
}

// ============================================================================
// TASK VENT
// ============================================================================
void TaskVentControl(void *pv) {

  float cur = 0;

  while (1) {
    SharedState s = readState();
    float target = s.emergency ? 0 : s.ventAngle;

    if (fabs(target - cur) > 1.5f) {
      cur += (target > cur ? 2 : -2);
    }

    cur = constrain(cur, 0, 180);
    ventServo.write(cur);

    vTaskDelay(20);
  }
}

// ============================================================================
// TASK TEMPERATURE DISPLAY
// ============================================================================
void TaskTemperatureDisplay(void *pv) {

  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  float lastT = 25.0f;

  while (1) {
    if (gScenario != SCENARIO_NO_SEMAPHORE)
      xSemaphoreTake(sOLED, portMAX_DELAY);

    float tv;
    if (qTempVirtual && xQueueReceive(qTempVirtual, &tv, 0) == pdTRUE)
      lastT = tv;

    display.fillRect(0, 0, 128, 32, BLACK);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(3);
    display.setCursor(0, 0);
    display.printf("%.1fC", lastT);
    display.display();

    if (gScenario != SCENARIO_NO_SEMAPHORE)
      xSemaphoreGive(sOLED);

    if (gScenario == SCENARIO_STARVATION) vTaskDelay(5);
    else                                  vTaskDelay(50);
  }
}

// ============================================================================
// TASK STATUS DISPLAY
// ============================================================================
void TaskStatusDisplay(void *pv) {

  while (1) {
    if (gScenario != SCENARIO_NO_SEMAPHORE)
      xSemaphoreTake(sOLED, portMAX_DELAY);

    SharedState s = readState();

    display.fillRect(0, 32, 128, 32, BLACK);

    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 34);
    display.printf("MODE:%s",
      currentMode == MODE_AUTO ? "AUTO" : "MANUAL");

    display.setCursor(0, 44);
    display.printf("Fan:%3d Vent:%3d", s.fanSpeed, s.ventAngle);

    display.setCursor(0, 54);
    if (s.emergency)      display.print("EMERGENCY");
    else if (s.danger)    display.print("DANGER");
    else if (s.warn)      display.print("WARNING");
    else                  display.print("NORMAL");

    display.display();

    if (gScenario != SCENARIO_NO_SEMAPHORE)
      xSemaphoreGive(sOLED);

    vTaskDelay(gScenario == SCENARIO_STARVATION ? 150 : 80);
  }
}

// ============================================================================
// TASK ALARM (FIXED PRIORITY)
// ============================================================================
void TaskAlarm(void *pv) {

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  ledcAttachPin(BUZZ_PIN, 7);
  ledcSetup(7, 2000, 10);

  uint32_t lastBeep = 0;
  bool beep = false;

  while (1) {
    SharedState s = readState();

    if (s.emergency) {
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_RED, HIGH);
      ledcWriteTone(7, 1200);
    }
    else if (s.danger) {
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_RED, HIGH);

      if (millis() - lastBeep > 500) {
        beep = !beep;
        ledcWriteTone(7, beep ? 900 : 0);
        lastBeep = millis();
      }
    }
    else if (s.warn) {
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_YELLOW, HIGH);
      digitalWrite(LED_RED, LOW);
      ledcWriteTone(7, 0);
    }
    else {
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_RED, LOW);
      ledcWriteTone(7, 0);
    }

    vTaskDelay(60);
  }
}

// ============================================================================
// CREATE TASKS
// ============================================================================
void createTasksForScenario() {

  UBaseType_t prioSensor      = 3;
  UBaseType_t prioInputUser   = 3;
  UBaseType_t prioTempDisplay = 2;
  UBaseType_t prioStatusMsg   = 2;
  UBaseType_t prioAlarm       = 4; 
  UBaseType_t prioControl     = 5;
  UBaseType_t prioFanMotor    = 4;
  UBaseType_t prioVentCtrl    = 4;

  BaseType_t coreSensor      = 0;
  BaseType_t coreInputUser   = 0;
  BaseType_t coreTempDisplay = 0;
  BaseType_t coreStatusMsg   = 0;
  BaseType_t coreAlarm       = 0;

  BaseType_t coreControl     = 1;
  BaseType_t coreFanMotor    = 1;
  BaseType_t coreVentCtrl    = 1;

  if (gScenario == SCENARIO_NO_MULTICORE) {
    coreControl = coreFanMotor = coreVentCtrl = 0;
  }

  if (gScenario == SCENARIO_STARVATION) {
    coreControl = coreFanMotor = coreVentCtrl = 0;
    prioTempDisplay = 7;
    prioStatusMsg   = 7;
    prioFanMotor    = 1;
    prioVentCtrl    = 1;
  }

  xTaskCreatePinnedToCore(TaskSensor,           "Sensor",     2048, nullptr, prioSensor,      &hTaskSensor,      coreSensor);
  xTaskCreatePinnedToCore(TaskInputUser,        "InputUser",  2048, nullptr, prioInputUser,   &hTaskInputUser,   coreInputUser);
  xTaskCreatePinnedToCore(TaskTemperatureDisplay,"TempDisp",  4096, nullptr, prioTempDisplay, &hTaskTempDisp,    coreTempDisplay);
  xTaskCreatePinnedToCore(TaskStatusDisplay,    "Status",     2048, nullptr, prioStatusMsg,   &hTaskStatusDisp,  coreStatusMsg);
  xTaskCreatePinnedToCore(TaskAlarm,            "Alarm",      2048, nullptr, prioAlarm,       &hTaskAlarm,       coreAlarm);
  xTaskCreatePinnedToCore(TaskControl,          "Control",    4096, nullptr, prioControl,     &hTaskControl,     coreControl);
  xTaskCreatePinnedToCore(TaskFanMotor,         "FanMotor",   3072, nullptr, prioFanMotor,    &hTaskFanMotor,    coreFanMotor);
  xTaskCreatePinnedToCore(TaskVentControl,      "VentCtrl",   3072, nullptr, prioVentCtrl,    &hTaskVentCtrl,    coreVentCtrl);
}

// ============================================================================
// SETUP
// ============================================================================
void setup() {

  Serial.begin(115200);
  delay(300);

  Serial.println("\n====================================");
  Serial.println("  SISTEM KIPAS & VENTILASI – RTOS DEMO");
  Serial.println("  ESP32-S3 FINAL FIXED VERSION");
  Serial.println("====================================");

  gScenario = waitScenarioFromSerial();

  qModeEvent   = xQueueCreate(8, sizeof(InputEvent));

  qTempVirtual = xQueueCreate(1, sizeof(float));

  mState = xSemaphoreCreateMutex();

  sOLED  = xSemaphoreCreateBinary();
  xSemaphoreGive(sOLED);

  SharedState init{};
  init.temp = 25;
  init.fanSpeed = 0;
  init.ventAngle = 0;
  init.warn = false;
  init.danger = false;
  init.emergency = false;

  writeState(init);

  pinMode(EMG_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(EMG_PIN), emgISR, FALLING);

  ventServo.setPeriodHertz(50);
  ventServo.attach(SERVO_PIN, 544, 2500);

  ledcAttachPin(BUZZ_PIN, 7);
  ledcSetup(7, 2000, 10);

  createTasksForScenario();

  Serial.println("Sistem siap.\n");
}

// ============================================================================
// LOOP — kosong
// ============================================================================
void loop() {
  vTaskDelay(1000);
}
