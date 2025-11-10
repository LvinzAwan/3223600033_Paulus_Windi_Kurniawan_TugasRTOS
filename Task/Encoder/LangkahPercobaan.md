
### Percobaan Rotary Encoder Dual Core ESP32-S3 (Simulasi Wokwi)

**Judul:** Simulasi Pembacaan Rotary Encoder dengan Dual-Core FreeRTOS pada ESP32-S3

---

#### Tujuan

* Menguji pembacaan sinyal rotary encoder pada ESP32-S3.
* Menerapkan konsep multitasking dengan dua core (FreeRTOS).
* Mengamati hasil pemindahan task antar core secara real-time lewat serial monitor.

---

#### Alat & Bahan (Simulasi)

* **Platform:** [Wokwi Simulator](https://wokwi.com/projects/447236106676493313)
* **Komponen:**

  * ESP32-S3
  * Rotary Encoder KY-040

**Koneksi:**

| Rotary Encoder | ESP32-S3 |
| -------------- | -------- |
| CLK            | GPIO 18  |
| DT             | GPIO 19  |
| +              | 3.3V     |
| GND            | GND      |

---

#### Langkah Percobaan

1. Buka link simulasi Wokwi:
   👉 [https://wokwi.com/projects/447236106676493313](https://wokwi.com/projects/447236106676493313)
2. Klik **Run Simulation**.
3. Buka **Serial Monitor** (baud rate 115200).
4. Putar encoder virtual:

   * Putaran **searah jarum jam (CW)** → nilai encoder naik.
   * Putaran **berlawanan arah (CCW)** → nilai encoder turun.
5. Coba kirim perintah lewat Serial Monitor:

   * Ketik `reset` → nilai encoder kembali ke **0**.
   * Ketik `core0` → pindahkan proses pembacaan encoder ke **core 0**.
   * Ketik `core1` → pindahkan kembali ke **core 1**.
6. Amati perubahan pada Serial Monitor setiap kali task berpindah core atau encoder diputar.

---

#### Hasil Pengamatan

| Aksi                    | Output di Serial Monitor            |
| ----------------------- | ----------------------------------- |
| Putar encoder CW        | Encoder: 1, 2, 3, ...               |
| Putar encoder CCW       | Encoder: -1, -2, -3, ...            |
| Ketik `reset`           | Encoder direset ke 0                |
| Ketik `core0` / `core1` | Encoder task dipindah ke core 0 / 1 |

#### Gambar Demo
<img width="1918" height="863" alt="Gambar Encoder Dual Core" src="https://github.com/user-attachments/assets/9eded6dc-0ca2-473a-9949-6bb594abbe01" />

#### Video Demo


https://github.com/user-attachments/assets/9cec8128-de9d-40b3-8c25-8e1b554954eb


---
