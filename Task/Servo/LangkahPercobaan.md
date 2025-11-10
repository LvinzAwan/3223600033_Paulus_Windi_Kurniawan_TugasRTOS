### Percobaan Servo Dual Core ESP32 (Simulasi Wokwi)

**Judul:** Simulasi Pengendalian Servo Menggunakan Dual-Core FreeRTOS pada ESP32

---

#### Tujuan

* Menguji pengendalian servo dengan sistem multitasking dua core.
* Mempelajari cara memindahkan task servo antar core secara dinamis.
* Mengamati respon servo terhadap perintah sudut melalui Serial Monitor.

---

#### Alat & Simulasi

**Platform:** [Wokwi Simulator – Servo Dual Core ESP32](https://wokwi.com/projects/447234800508152833)
**Komponen:**

* ESP32-S3
* Servo motor
* Kabel penghubung

**Koneksi:**

| Komponen    | ESP32 Pin | Keterangan               |
| ----------- | --------- | ------------------------ |
| Servo (PWM) | GPIO 5    | Pin sinyal kontrol servo |
| VCC Servo   | 5V        | Daya servo               |
| GND Servo   | GND       | Ground bersama           |

---

#### Langkah Percobaan

1. Buka link simulasi:
   👉 [https://wokwi.com/projects/447234800508152833](https://wokwi.com/projects/447234800508152833)
2. Tekan **Run Simulation** untuk memulai simulasi.
3. Buka **Serial Monitor** (baud rate **115200**).
4. Masukkan perintah berikut melalui Serial Monitor:

   * `0–180` → mengatur sudut servo.
   * `core0` → pindahkan task servo ke core 0.
   * `core1` → pindahkan task servo ke core 1.
5. Amati pergerakan servo dan pesan di Serial Monitor yang menunjukkan core aktif serta posisi servo.

---

#### Hasil Pengamatan

| Perintah       | Respon di Serial Monitor            |
| -------------- | ----------------------------------- |
| `90`           | Servo ke 90°, selesai di 90°        |
| `core0`        | Servo task dipindah ke core 0       |
| `core1`        | Servo task dipindah ke core 1       |
| Nilai berulang | Servo bergerak halus ke posisi baru |

#### Gambar Demo 
<img width="1918" height="868" alt="Gambar Servo Dual Core" src="https://github.com/user-attachments/assets/ecf50e12-a167-4958-a1f7-ac2ff607114e" />

#### Video Demo 


https://github.com/user-attachments/assets/4093a184-f399-4850-8d72-7ed71e9f8132




---
