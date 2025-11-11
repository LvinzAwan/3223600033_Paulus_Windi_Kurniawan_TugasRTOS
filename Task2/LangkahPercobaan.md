#**Percobaan Pengaruh Priority dan Stack Size pada Multi-Tasking FreeRTOS**

---

## 🎯 **1. Tujuan Percobaan**
Percobaan ini bertujuan untuk **menganalisis pengaruh pengaturan prioritas (priority) dan ukuran stack (stack size)** terhadap kestabilan dan performa sistem operasi waktu nyata (**FreeRTOS**) pada mikrokontroler **ESP32-S3** yang memiliki dua inti prosesor (dual-core).

Melalui percobaan ini, setiap peripheral dijalankan dalam **task terpisah**, kemudian diuji pada tiga skenario berbeda untuk melihat bagaimana pengaturan priority dan stack memengaruhi:
- Kelancaran eksekusi task,
- Distribusi beban antar core,
- Potensi terjadinya *task starvation* dan *blocking*,
- Konsistensi pembaruan data pada perangkat keras (servo, OLED, stepper, dll).

---

## ⚙️ **2. Alat dan Bahan**
- **Mikrokontroler:** ESP32-S3 DevKitC-1  
- **Software:** Arduino IDE / Wokwi Simulator  
- **Komponen:**
  - 3 LED + resistor 220Ω  
  - 2 push button  
  - Buzzer  
  - Potensiometer  
  - Rotary encoder  
  - OLED Display SSD1306 (I2C)  
  - Servo SG90  
  - Stepper Motor NEMA17 + Driver A4988  
  - Breadboard dan kabel jumper  

---

## 🔌 **3. Deskripsi Rangkaian**
Rangkaian dibuat sesuai konfigurasi di bawah ini:

| Komponen | Pin ESP32-S3 | Keterangan |
|-----------|--------------|------------|
| LED1 | GPIO15 | Nyala-mati setiap 200 ms |
| LED2 | GPIO16 | Nyala-mati setiap 300 ms |
| LED3 | GPIO17 | Nyala-mati setiap 400 ms |
| Tombol 1 | GPIO6 | Input digital (aktif LOW) |
| Tombol 2 | GPIO5 | Input digital (aktif LOW) |
| Buzzer | GPIO7 | Bunyi periodik dengan frekuensi 1.2 kHz |
| Potensiometer | GPIO3 | Input analog |
| Rotary Encoder | CLK=GPIO10, DT=GPIO11 | Deteksi arah dan jumlah putaran |
| OLED | SDA=GPIO8, SCL=GPIO9 | Menampilkan teks dan status sistem |
| Servo | GPIO18 | PWM 50Hz, bergerak 0°–180° |
| Stepper (A4988) | STEP=GPIO13, DIR=GPIO14 | Bergerak maju-mundur |
| Power | 5V dan GND | Suplai untuk servo, OLED, dan driver |

> **Catatan:** Servo dan buzzer diatur agar tidak memakai LEDC secara bersamaan untuk mencegah konflik PWM.

---

## 🧩 **4. Pembagian Task dan Core**
Masing-masing peripheral dikendalikan oleh satu task independen yang dijalankan secara paralel melalui FreeRTOS.

| Task | Fungsi | Core | Prioritas |
|------|--------|-------|------------|
| **TaskLED** | Menyalakan LED bergantian | Core 0 | 1 |
| **TaskButton** | Membaca tombol input | Core 0 | 2 |
| **TaskOLED** | Menampilkan teks pada OLED | Core 0 | 1 |
| **TaskServo** | Menggerakkan servo 0°–180° | Core 1 | 3 |
| **TaskStepper** | Menggerakkan stepper bolak-balik | Core 1 | 3 |
| **TaskEncoder** | Membaca rotary encoder | Core 1 | 4 |
| **TaskPot** | Membaca potensiometer | Core 1 | 1 |
| **TaskBuzzer** | Mengeluarkan bunyi beep periodik | Core 1 | 1 |

---

## 🧪 **5. Langkah Percobaan**
1. Buka proyek pada **Arduino IDE** atau **Wokwi**.  
2. Sambungkan seluruh komponen sesuai dengan tabel pin di atas.  
3. Upload program ke board ESP32-S3.  
4. Buka **Serial Monitor** dengan baud rate `115200`.  
5. Jalankan percobaan untuk tiga skenario berikut dengan mengubah variabel:
   ```cpp
   int scenario = 1; // ubah ke 2 atau 3 untuk mencoba skenario lain
   ```
6. Amati hasil pada Serial Monitor, OLED, dan pergerakan servo/stepper.  
7. Catat perbedaan performa antar skenario.  
8. Sertakan tiga video dokumentasi hasil percobaan (Scenario 1, 2, dan 3).

---
## 📹 **6. Dokumentasi**
Tiga video hasil percobaan disertakan sebagai bukti:
1. **Video 1 – Scenario 1:** Semua peripheral berjalan normal dan sinkron.  


https://github.com/user-attachments/assets/331d62dc-0e2f-426a-990c-91e56662a946


2. **Video 2 – Scenario 2:** Delay muncul akibat prioritas tidak seimbang.  


https://github.com/user-attachments/assets/ebd666ce-2cc0-476e-a866-fa5729a3fbf2


3. **Video 3 – Scenario 3:** Sistem tidak stabil akibat stack terlalu kecil.

   
https://github.com/user-attachments/assets/454c719c-ae25-42fe-8330-31dea1147b90

---

## 📊 **7. Hasil Percobaan**

### 🔹 **Scenario 1 — Prioritas & Stack Ideal**
- Semua task berjalan lancar dan stabil.  
- Servo bergerak halus dari 0° ke 180°.  
- Stepper berputar maju-mundur tanpa delay.  
- OLED menampilkan teks dengan refresh stabil.  
- Buzzer beep periodik tanpa gangguan.  
- Encoder dan potensiometer terbaca akurat.  

**Kesimpulan:** Pengaturan prioritas dan stack seimbang → tidak terjadi *blocking* antar task.

---

### 🔹 **Scenario 2 — Prioritas Campur** 
- Servo kadang tersendat saat buzzer aktif.  

**Kesimpulan:** Task berprioritas tinggi mendominasi waktu CPU → muncul *task preemption*.

---

### 🔹 **Scenario 3 — Stack Kecil & Prioritas Rendah**
- OLED tidak tampil (task gagal inisialisasi).  
- Servo dan stepper tidak bergerak.  
- Terjadi Stack Overflow yang mengakibatkan crash.  

**Kesimpulan:** Stack kecil dan prioritas rendah menyebabkan *task starvation* dan *crash*.  

---

## 📘 **8. Analisis**
- Core 0 cocok untuk task ringan (I/O, tampilan, LED).  
- Core 1 ideal untuk task presisi tinggi (servo, stepper, encoder).  
- Stack size yang cukup penting agar tidak terjadi *overflow* saat task menulis ke buffer.  
- Prioritas harus ditentukan berdasarkan urgensi real-time task, bukan jumlah operasi.

---


## 🧾 **9. Kesimpulan Akhir**
Percobaan ini menunjukkan bahwa:
- Pengaturan **priority** dan **stack** sangat berpengaruh terhadap kinerja multitasking.  
- Kombinasi dual-core pada ESP32-S3 memberikan performa optimal bila pembagian beban antar core seimbang.  
- FreeRTOS memungkinkan kontrol multi-peripheral secara simultan dengan tingkat stabilitas tinggi, selama resource tidak tumpang tindih.  

---
