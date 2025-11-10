
### Percobaan LED Dual Core ESP32 (Simulasi Wokwi)

**Judul:** Simulasi Pengendalian Tiga LED Menggunakan Dual-Core FreeRTOS pada ESP32-S3

---

#### Tujuan

* Mengetahui cara menjalankan beberapa task secara paralel menggunakan FreeRTOS pada ESP32-S3.
* Mengamati kerja multitasking antara dua core melalui pengendalian tiga LED dengan jeda waktu berbeda.

---

#### Alat & Simulasi

**Platform:** [Wokwi Simulator – LED Dual Core ESP32](https://wokwi.com/) *(ganti dengan link proyekmu setelah disimpan)*
**Komponen:**

* ESP32-S3 
* 3 LED merah
* Kabel jumper

---

#### Koneksi

| Komponen       | ESP32 Pin | Keterangan                          |
| -------------- | --------- | ----------------------------------- |
| LED 1 (atas)   | GPIO 4    | Dinyalakan dari **Task 1 (core 0)** |
| LED 2 (tengah) | GPIO 17   | Dinyalakan dari **Task 2 (core 1)** |
| LED 3 (bawah)  | GPIO 10   | Dinyalakan dari **Task 3 (core 0)** |
| Katoda LED     | GND       | Ground bersama                      |

> **Catatan:** Pada simulasi Wokwi, resistor tidak wajib digunakan karena LED tidak rusak akibat arus berlebih.
> Namun, di rangkaian nyata, sebaiknya setiap LED dipasang resistor pembatas arus 220Ω–330Ω.

---

#### Langkah Percobaan

1. Buka simulasi Wokwi sesuai program.
2. Jalankan simulasi dengan klik **Run Simulation**.
3. Buka **Serial Monitor** (baud rate **115200**).
4. Amati hasil tampilan:

   ```
   Task1 running on core 0  
   Task3 running on core 0  
   Task2 running on core 1  
   ```
5. Perhatikan LED pada board:

   * **LED1 (GPIO 4)** berkedip setiap 1 detik.
   * **LED2 (GPIO 17)** berkedip setiap 0.7 detik.
   * **LED3 (GPIO 10)** berkedip setiap 0.5 detik.

---

#### Hasil Pengamatan

| Task  | Core | Interval Kedip | Pin     |
| ----- | ---- | -------------- | ------- |
| Task1 | 0    | 1 detik        | GPIO 4  |
| Task2 | 1    | 0.7 detik      | GPIO 17 |
| Task3 | 0    | 0.5 detik      | GPIO 10 |

#### Gambar Demo
<img width="1918" height="870" alt="Gambar Led Dual Core" src="https://github.com/user-attachments/assets/8ce5f7b4-4057-49a5-885b-87a2227a9bdc" />

#### Video Demo


https://github.com/user-attachments/assets/439d91d9-0271-41b4-85ad-a2ebad857fa3


---
