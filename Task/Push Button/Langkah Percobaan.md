# README – Simulasi Push Button dengan Dual-Core FreeRTOS pada ESP32-S3 (Wokwi)

**Judul:** Simulasi Push Button dengan Dual-Core FreeRTOS pada ESP32-S3

---

## Tujuan

* Menguji pembacaan dua pushbutton secara terpisah pada ESP32-S3.  
* Menerapkan multitasking dengan **Dual-Core FreeRTOS**: satu task per core.  
* Mengamati lewat Serial Monitor event `PRESSED` dan `RELEASED` masing-masing button serta core yang mengeksekusi task.

---

## Alat & Bahan (Simulasi)

* **Platform:** Wokwi Simulator — [Buka project Wokwi](https://wokwi.com/projects/447226073014238209)
* **Komponen:**
  * ESP32-S3 DevKitC-1
  * 2 × Pushbutton (virtual di Wokwi)

---

## Koneksi (sesuai simulasi)

| Pushbutton | Pin ESP32-S3 |
| ----------- | ------------- |
| btn1.1.l    | GPIO 20       |
| btn1.2.l    | GND           |
| btn2.1.l    | GPIO 21       |
| btn2.2.l    | GND           |

> Pastikan di kode GPIO 20 dan 21 dikonfigurasi sebagai input (biasanya dengan internal pull-up atau pull-down sesuai wiring) agar pembacaan stabil.

---

## Langkah Percobaan

1. Buka link proyek Wokwi: [https://wokwi.com/projects/447226073014238209](https://wokwi.com/projects/447226073014238209)
2. Klik **Run Simulation**.
3. Buka **Serial Monitor** (baud **115200**).
4. Jalankan program yang sudah meng-implementasikan dua task FreeRTOS:
   * Task `btn1_task` di-pin ke **core 0**, membaca GPIO20 (btn1).
   * Task `btn2_task` di-pin ke **core 1**, membaca GPIO21 (btn2).
5. Uji dengan **menekan (click)** dan **melepas (release)** pushbutton virtual pada Wokwi:
   * Tekan/lepaskan `btn1` (GPIO20) beberapa kali; amati output di serial.
   * Tekan/lepaskan `btn2` (GPIO21) beberapa kali; amati output di serial.
6. Catat timestamp dan pesan `PRESSED` / `RELEASED` yang muncul di Serial Monitor untuk masing-masing core.


---

## Tabel Hasil Pengamatan

| No | Aksi (yang dilakukan)               | Output Serial contoh                                                                 | Keterangan |
|----|-------------------------------------|--------------------------------------------------------------------------------------|-------------|
| 1  | Tekan btn1 (GPIO20)                 | `[Core 0] Button 1 PRESSED (pin 20) at 2651 ms`                                     | Task btn1 dijalankan di core0 |
| 2  | Lepas btn1                          | `[Core 0] Button 1 RELEASED (pin 20) at 2751 ms`                                    | Release terbaca di core0 |
| 3  | Tekan btn1 lagi                     | `[Core 0] Button 1 PRESSED (pin 20) at 3181 ms`                                     | — |
| 4  | Lepas btn1                          | `[Core 0] Button 1 RELEASED (pin 20) at 3261 ms`                                    | — |
| 5  | Tekan btn2 (GPIO21)                 | `[Core 1] Button 2 PRESSED (pin 21) at 3931 ms`                                     | Task btn2 dijalankan di core1 |
| 6  | Lepas btn2                          | `[Core 1] Button 2 RELEASED (pin 21) at 4911 ms`                                    | — |
| 7  | Tekan btn2 lagi                     | `[Core 1] Button 2 PRESSED (pin 21) at 7431 ms`                                     | — |
| 8  | Lepas btn2                          | `[Core 1] Button 2 RELEASED (pin 21) at 7951 ms`                                    | — |

---

## Gambar Demo
<img width="1920" height="1080" alt="Gambar Percobaan" src="https://github.com/user-attachments/assets/66556065-493c-49e1-906f-3b4dec74ac3b" />

## Video Demo
https://github.com/user-attachments/assets/dba4cc20-97e7-4129-bad6-7e2b9bb36b55

