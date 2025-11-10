# RTOS: Dual Core ESP32-S3 dengan Multi-Peripheral

## Deskripsi
Proyek ini menunjukkan penerapan **sistem RTOS berbasis dual-core** pada mikrokontroler **ESP32-S3**, di mana setiap core menjalankan beberapa task secara paralel.  
Core 0 mengatur LED, tombol, buzzer, OLED, servo, dan input serial, sedangkan Core 1 mengelola potensiometer, rotary encoder, dan motor stepper.  
Sinkronisasi antar-task dilakukan menggunakan **mutex semaphore**, dan pembagian beban kerja antar-core diatur menggunakan `xTaskCreatePinnedToCore()` agar sistem tetap responsif dan stabil.

---

## Komponen yang Digunakan
- ESP32-S3 DevKitC-1  
- OLED SSD1306 (I2C Address: 0x3C)  
- Potensiometer  
- Rotary Encoder KY-040  
- Servo Motor  
- Stepper Motor (400 step/rev) + Driver A4988  
- LED dan Resistor 220 Ω  
- Push Button  
- Buzzer aktif  
- Breadboard dan kabel jumper  

---

## Langkah Percobaan

### 1. Rangkai Komponen
| Komponen | Pin ESP32-S3 | Keterangan |
|-----------|---------------|------------|
| LED | GPIO 5 | Output indikator |
| Push Button | GPIO 6 | Input digital (pull-up internal) |
| Buzzer | GPIO 7 | Output aktif |
| Potensiometer | GPIO 3 | Input analog |
| OLED SDA | GPIO 8 | Komunikasi I2C |
| OLED SCL | GPIO 9 | Komunikasi I2C |
| Rotary Encoder CLK | GPIO 10 | Input A |
| Rotary Encoder DT | GPIO 11 | Input B |
| Servo | GPIO 12 | PWM output |
| Stepper STEP | GPIO 13 | Kontrol langkah |
| Stepper DIR | GPIO 14 | Arah putaran |

---

### 2. Upload Program
1. Buka **Arduino IDE** atau **Wokwi Simulator**.  
2. Masukkan kode dari file skecth.ino.  
3. Atur baud rate Serial Monitor ke **115200**.  
4. Jalankan simulasi untuk melihat hasilnya.

---

### 3. Jalankan Simulasi
**Link Wokwi Simulasi:**  
👉 [Klik di sini untuk membuka proyek](https://wokwi.com/projects/447250257472870401)

---

### 4. Uji Tiap Peripheral

#### a. LED & Core Test  
LED berkedip setiap 300 ms menandakan task berjalan pada **Core 0**.

#### b. Tombol dan Buzzer  
- Tekan tombol → buzzer berbunyi 2 kHz selama 100 ms.  
- Status tombol (ON/OFF) tampil di Serial Monitor dan OLED.

#### c. OLED Display  
OLED menampilkan status sistem secara real-time:
ESP32-S3 DualCore
Core0:83 Core1:78
POT:456 ENC:-10
BTN:OFF

#### d. Potensiometer dan Encoder  
- Putar potensiometer → nilai analog berubah di Serial Monitor & OLED.  
- Putar rotary encoder → nilai bertambah atau berkurang sesuai arah.

#### e. Servo dan Stepper Motor  
Masukkan perintah di Serial Monitor:
- `servo90` → Menggerakkan servo ke 90°.  
- `step200` → Stepper maju 200 langkah.  
- `step-200` → Stepper mundur 200 langkah.
  
---

## Pembagian Core dan Task

| Core | Task | Fungsi |
|------|------|--------|
| **Core 0** | TaskLED | Menyalakan dan mematikan LED |
| **Core 0** | TaskButton | Membaca input tombol dan mengaktifkan buzzer |
| **Core 0** | TaskOLED | Menampilkan data sistem ke OLED |
| **Core 0** | TaskServo | Mengatur posisi servo sesuai perintah |
| **Core 0** | TaskInput | Membaca input teks dari Serial |
| **Core 1** | TaskEncoder | Membaca putaran rotary encoder |
| **Core 1** | TaskStepper | Menggerakkan motor stepper |
| **Core 1** | TaskPot | Membaca nilai analog dari potensiometer |

---

## Output di Serial Monitor
Contoh hasil saat simulasi berjalan:
[SETUP DONE] Dual-core system started.
[MONITOR] Core0:85 Core1:80 | POT:512 | ENC:-8 | BTN:OFF
[INPUT] Servo set to 90°
[SERVO] Moved to 90°
[INPUT] Stepper move -200 steps
[STEPPER] Move to position: -200
[BUTTON] Pressed, buzzer: ON

---

## Dokumentasi

**Tampilan Simulasi Wokwi:**  
<img width="1918" height="868" alt="Gambar All Pheripheral Dual Core" src="https://github.com/user-attachments/assets/36c75339-4008-498a-99b9-59e4c4bf1469" />

**Video Demonstrasi:**  


https://github.com/user-attachments/assets/668683c1-3d02-4cb0-b9f9-3235d16ff615


---
