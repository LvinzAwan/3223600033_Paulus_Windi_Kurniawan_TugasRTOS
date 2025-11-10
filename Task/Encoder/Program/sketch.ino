#define CLK 18
#define DT  19

volatile int count = 0;
volatile bool resetCommand = false;
volatile bool newCoreCommand = false;
volatile int currentCore = 1;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

TaskHandle_t inputTaskHandle;
TaskHandle_t encoderTaskHandle;

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("\nDual Core Rotary Encoder (ESP32)");
  Serial.println("Ketik 'core0' atau 'core1' untuk pindah core encoder");
  Serial.println("Ketik 'reset' untuk mengatur ulang nilai encoder\n");

  xTaskCreatePinnedToCore(inputTask, "InputTask", 4096, NULL, 1, &inputTaskHandle, 0);
  xTaskCreatePinnedToCore(encoderTask, "EncoderTask", 4096, NULL, 1, &encoderTaskHandle, currentCore);
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
          vTaskDelete(encoderTaskHandle);
          portENTER_CRITICAL(&mux);
          currentCore = buffer.endsWith("0") ? 0 : 1;
          portEXIT_CRITICAL(&mux);
          xTaskCreatePinnedToCore(encoderTask, "EncoderTask", 4096, NULL, 1, &encoderTaskHandle, currentCore);
          Serial.printf("Encoder task dipindah ke core %d\n", currentCore);
        } 
        else if (buffer.equalsIgnoreCase("reset")) {
          portENTER_CRITICAL(&mux);
          count = 0;
          portEXIT_CRITICAL(&mux);
          Serial.println("Encoder direset ke 0");
        }
        buffer = "";
      } else buffer += c;
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void encoderTask(void *pvParameters) {
  Serial.print("Encoder Task core ");
  Serial.println(xPortGetCoreID());

  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);

  int lastState = digitalRead(CLK);

  for (;;) {
    int currentState = digitalRead(CLK);
    if (currentState != lastState) {
      if (digitalRead(DT) != currentState) count++;
      else count--;

      Serial.printf("Encoder: %d\n", count);
    }
    lastState = currentState;
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(10));
}
