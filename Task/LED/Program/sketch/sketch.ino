TaskHandle_t led1TaskHandle, led2TaskHandle, led3TaskHandle;

const int led1 = 4;
const int led2 = 17;
const int led3 = 10;  

void setup() {
  Serial.begin(115200);
  delay(200); 

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

  xTaskCreatePinnedToCore(LedTask1code, "LedTask1", 4096, NULL, 1, &led1TaskHandle, 0);
  xTaskCreatePinnedToCore(LedTask2code, "LedTask2", 4096, NULL, 1, &led2TaskHandle, 1);
  xTaskCreatePinnedToCore(LedTask3code, "LedTask3", 4096, NULL, 1, &led3TaskHandle, 0);
}

void LedTask1code(void *pv) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  for (;;) {
    digitalWrite(led1, HIGH); delay(1000);
    digitalWrite(led1, LOW);  delay(1000);
  }
}

void LedTask2code(void *pv) {
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  for (;;) {
    digitalWrite(led2, HIGH); delay(700);
    digitalWrite(led2, LOW);  delay(700);
  }
}

void LedTask3code(void *pv) {
  Serial.print("Task3 running on core ");
  Serial.println(xPortGetCoreID());
  for (;;) {
    digitalWrite(led3, HIGH); delay(500);
    digitalWrite(led3, LOW);  delay(500);
  }
}

void loop() {
  delay(10); 
}
