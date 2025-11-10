# README – Simulasi Dual-Core FreeRTOS dengan OLED Display pada ESP32-S3 (Wokwi)

**Judul:** Simulasi Dual-Core FreeRTOS pada ESP32-S3 dengan tampilan teks dinamis di OLED (Core 0 & Core 1)

---

## Tujuan

* Mengimplementasikan **Dual-Core FreeRTOS** pada ESP32-S3 dengan dua task OLED yang berjalan di core berbeda.  
* Menampilkan teks berbeda pada OLED tergantung core yang aktif (`RTOS Core0` atau `RTOS Core1`).  
* Mengontrol core aktif secara **interaktif** melalui input Serial.

---

## Alat & Bahan (Simulasi)

* **Platform:** Wokwi Simulator — [Buka project di sini](https://wokwi.com/projects/447255677154399233)  
* **Komponen:**
  * ESP32-S3 DevKitC  
  * OLED Display SSD1306 (I²C)  

---

## Koneksi (sesuai simulasi)

| Komponen | Pin ESP32-S3 |
|-----------|---------------|
| OLED SDA  | GPIO 36 |
| OLED SCL  | GPIO 35 |
| VCC       | 3.3V |
| GND       | GND |

> Pastikan alamat I²C OLED diatur ke **0x3C**, dan pin SDA/SCL sesuai seperti di atas.

---

## Langkah Percobaan

1. Buka proyek di Wokwi: [https://wokwi.com/projects/447255677154399233](https://wokwi.com/projects/447255677154399233)  
2. Klik **Run Simulation**.  
3. Buka **Serial Monitor** (baud **115200**).  
4. Setelah start, akan muncul pesan: "Core yang aktif = 0. Ketik '0' atau '1' di Serial."
5. Lihat tampilan OLED: akan menampilkan teks **“RTOS Core0”** secara bertahap.  
6. Di **Serial Monitor**, ketik:  
* `1` → OLED menampilkan teks **“RTOS Core1”**  
* `0` → OLED menampilkan teks **“RTOS Core0”** kembali  
7. Ulangi beberapa kali dan amati peralihan tampilan OLED antar core.

---

## Tabel Hasil Pengamatan

| No | Aksi (Serial Input) | Output Serial | Tampilan OLED | Core yang aktif |
|----|----------------------|----------------|----------------|-----------------|
| 1  | – (awal simulasi) | `Core yang aktif = 0. Ketik '0' atau '1' di Serial.` | Mengetik “RTOS Core0” | Core 0 |
| 2  | Ketik `1` | `Core yang aktif = 1` | Mengetik “RTOS Core1” | Core 1 |
| 3  | Ketik `0` | `Core yang aktif = 0` | Mengetik “RTOS Core0” | Core 0 |
| 4  | Ketik selain `0`/`1` | `Gunakan '0' atau '1'` | Tidak berubah | – |

---

## Hasil Akhir

Simulasi berhasil menunjukkan pembagian tugas **Dual-Core FreeRTOS** pada ESP32-S3, di mana:
* **Core 0** dan **Core 1** menjalankan task OLED terpisah.  
* Hanya task dari core yang **aktif** yang boleh menulis ke OLED (sinkronisasi via **mutex**).  
* Pergantian core dikendalikan secara real-time lewat input serial.

## Gambar Demo
<img width="1920" height="1080" alt="Gambar Percobaan" src="https://github.com/user-attachments/assets/49ae6cbb-ba62-41ee-994f-d4e4801b4a67" />

## Video Demo
https://github.com/user-attachments/assets/9fad0777-ae8f-4e7c-8bf2-a9fbd6636601




