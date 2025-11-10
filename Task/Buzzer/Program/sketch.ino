#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

const int buzzerPin = 14;
const int ledcResolution = 8;
const int baseFreq = 2000;

#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define REST     0

// --- Lagu 0: Twinkle Twinkle ---
int melody0[] = {
  NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_G4,
  NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4,
  NOTE_G4, NOTE_G4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4,
  NOTE_G4, NOTE_G4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4,
  NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_G4,
  NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4
};

int dur0[] = {
  4,4,4,4,4,4,2,
  4,4,4,4,4,4,2,
  4,4,4,4,4,4,2,
  4,4,4,4,4,4,2,
  4,4,4,4,4,4,2,
  4,4,4,4,4,4,2
};

// --- Lagu 1: Happy Birthday ---
int melody1[] = {
  NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_F4, NOTE_E4,
  NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_G4, NOTE_F4,
  NOTE_C4, NOTE_C4, NOTE_C5, NOTE_A4, NOTE_F4, NOTE_E4, NOTE_D4,
  NOTE_B4, NOTE_B4, NOTE_A4, NOTE_F4, NOTE_G4, NOTE_F4
};

int dur1[] = {
  8,8,4,4,4,2,
  8,8,4,4,4,2,
  8,8,4,4,4,4,2,
  8,8,4,4,4,2
};

const int tempo = 120; // bpm

SemaphoreHandle_t buzzerMutex = NULL;
volatile int currentSong = 0; // 0 = Twinkle (default), 1 = Happy Birthday

int N0 = sizeof(melody0) / sizeof(melody0[0]);
int N1 = sizeof(melody1) / sizeof(melody1[0]);

int noteMsFromDuration(int durationValue) {
  int quarterMs = (60000 / tempo);
  float ms = quarterMs * (4.0 / durationValue);
  return (int)(ms + 0.5);
}

void playNoteFreq(int frequency, int ms) {
  if (frequency == REST || ms <= 0) {
    ledcWriteTone(buzzerPin, 0);
    vTaskDelay(pdMS_TO_TICKS(ms > 0 ? ms : 1));
    return;
  }
  ledcWriteTone(buzzerPin, frequency);
  vTaskDelay(pdMS_TO_TICKS(ms));
  ledcWriteTone(buzzerPin, 0);
}

void playerTask(void *pvParameters) {
  Serial.printf("Player task running on core %d\n", xPortGetCoreID());
  for (;;) {
    // Ambil snapshot lagu yg dipilih
    int song = currentSong;
    if (xSemaphoreTake(buzzerMutex, portMAX_DELAY) == pdTRUE) {
      if (song == 0) {
        Serial.println("[Player] Playing Twinkle (song 0)");
        for (int i = 0; i < N0; ++i) {
          int note = melody0[i];
          int ms = noteMsFromDuration(dur0[i]);
          playNoteFreq(note, ms);
          vTaskDelay(pdMS_TO_TICKS(20));
          // jika user ganti lagu di tengah, bisa break untuk segera responsif
          if (currentSong != song) break;
        }
      } else if (song == 1) {
        Serial.println("[Player] Playing Happy Birthday (song 1)");
        for (int i = 0; i < N1; ++i) {
          int note = melody1[i];
          int ms = noteMsFromDuration(dur1[i]);
          playNoteFreq(note, ms);
          vTaskDelay(pdMS_TO_TICKS(20));
          if (currentSong != song) break;
        }
      }
      xSemaphoreGive(buzzerMutex);
    }
    // jeda antara pengulangan; lebih pendek kalau lagu diganti
    vTaskDelay(pdMS_TO_TICKS(300));
  }
}

void inputTask(void *pvParameters) {
  Serial.printf("Input task running on core %d\n", xPortGetCoreID());
  Serial.println("Ketik '0' atau 'twinkle' untuk lagu 0, '1' atau 'happy' untuk lagu 1, 'status' untuk cek.");
  String line;
  for (;;) {
    if (Serial.available()) {
      line = Serial.readStringUntil('\n');
      line.trim();
      if (line.length() == 0) {
        vTaskDelay(pdMS_TO_TICKS(50));
        continue;
      }
      String cmd = line;
      cmd.toLowerCase();
      if (cmd == "0" || cmd == "twinkle") {
        currentSong = 0;
        Serial.println("currentSong -> 0 (Twinkle)");
      } else if (cmd == "1" || cmd == "happy") {
        currentSong = 1;
        Serial.println("currentSong -> 1 (Happy Birthday)");
      } else if (cmd == "status") {
        Serial.printf("Status: currentSong = %d\n", currentSong);
      } else {
        Serial.printf("Unknown command: '%s'\n", line.c_str());
        Serial.println("Gunakan: 0 / twinkle | 1 / happy | status");
      }
    } else {
      vTaskDelay(pdMS_TO_TICKS(50));
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(50);

  ledcAttach(buzzerPin, baseFreq, ledcResolution);
  ledcWriteTone(buzzerPin, 0);

  buzzerMutex = xSemaphoreCreateMutex();

  // Player di core 0
  xTaskCreatePinnedToCore(
    playerTask,
    "PlayerTask",
    4096,
    NULL,
    1,
    NULL,
    0
  );

  // Input di core 1
  xTaskCreatePinnedToCore(
    inputTask,
    "InputTask",
    4096,
    NULL,
    1,
    NULL,
    1
  );

  Serial.println("Dual-core single-buzzer demo started. Default: Twinkle (song 0).");
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}