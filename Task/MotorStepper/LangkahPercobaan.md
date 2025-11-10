### Percobaan Motor Stepper Dual Core ESP32 (Simulasi Wokwi)

**Judul:** Simulasi Pengendalian Motor Stepper dengan Dual-Core FreeRTOS pada ESP32

---

#### Tujuan

* Menguji pengendalian motor stepper melalui driver A4988 menggunakan ESP32-S3.
* Menerapkan sistem multitasking dua core untuk memisahkan tugas input dan kontrol motor.
* Mengamati cara berpindahnya task motor antar core secara real-time lewat Serial Monitor.

---

#### Alat & Simulasi

**Platform:** [Wokwi Simulator – Motor Stepper Dual Core ESP32](https://wokwi.com/projects/447222884263304193)
**Komponen:**

* ESP32-S3
* Driver A4988
* Motor Stepper 100 steps
* Kabel jumper

---

#### Koneksi

| Komponen       | ESP32 Pin      | Keterangan               |
| -------------- | -------------- | ------------------------ |
| STEP           | GPIO 13        | Sinyal langkah stepper   |
| DIR            | GPIO 12        | Arah putaran             |
| ENABLE         | GND            | Aktifkan driver          |
| SLEEP          | 3.3V           | Mengaktifkan mode kerja  |
| RESET          | 3.3V           | Reset aktif tinggi       |
| VDD            | 3.3V           | Daya logika A4988        |
| VMOT           | 5V             | Daya motor               |
| GND            | GND ESP32      | Ground bersama           |
| A4988 ke motor | 1A, 1B, 2A, 2B | Sesuai urutan coil motor |

---

#### Langkah Percobaan

1. Buka link simulasi:
   👉 [https://wokwi.com/projects/447222884263304193](https://wokwi.com/projects/447222884263304193)
2. Tekan **Run Simulation** untuk menjalankan simulasi.
3. Buka **Serial Monitor** dengan baud rate **115200**.
4. Ketik perintah pada Serial Monitor:

   * Angka **positif** (mis. `200`) → motor berputar searah jarum jam.
   * Angka **negatif** (mis. `-200`) → motor berputar berlawanan arah jarum jam.
   * `core0` → memindahkan task motor ke core 0.
   * `core1` → memindahkan task motor ke core 1.
5. Amati pergerakan motor stepper dan output yang muncul di Serial Monitor.

---

#### Hasil Pengamatan

| Perintah | Output di Serial Monitor                  |
| -------- | ----------------------------------------- |
| `250`    | Motor bergerak 50 langkah ke posisi 250   |
| `-300`   | Motor bergerak -300 langkah ke posisi -50 |
| `core0`  | Motor task dipindah ke core 0             |
| `core1`  | Motor task dipindah ke core 1             |

#### Gambar Demo
<img width="1918" height="863" alt="Gambar Motor Stepper Dual Core" src="https://github.com/user-attachments/assets/54d057d8-8061-4739-b06b-118c43fc3fd4" />

#### Video Demo


https://github.com/user-attachments/assets/6beda4ad-59db-40a0-996d-247f1e0859f5


---
