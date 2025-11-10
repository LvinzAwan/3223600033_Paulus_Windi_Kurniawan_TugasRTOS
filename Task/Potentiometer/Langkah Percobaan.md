# README – Dual-Core Potensiometer pada ESP32-S3 (Wokwi)

**Judul:** Membaca Potensiometer dengan Dual-Core FreeRTOS pada ESP32-S3

---

## Tujuan
- Menjalankan dua task FreeRTOS di dua core berbeda:  
  - `readPotTask` → membaca nilai ADC potensiometer (**Core 0**)  
  - `displayPotTask` → menampilkan nilai ke Serial (**Core 1**)  
- Menunjukkan pembagian kerja antar core pada ESP32-S3.

---

## Alat & Bahan
- **Platform:** Wokwi Simulator ([ESP32-S3 DevKitC-1](https://wokwi.com))  
- **Komponen:**
  - 1 × ESP32-S3  
  - 1 × Potensiometer 10k  
  - 3 × Kabel jumper

---

## Koneksi
| Pin Potensiometer | Koneksi ke ESP32-S3 |
|--------------------|--------------------|
| Kaki kiri          | 3.3V               |
| Kaki tengah (wiper)| GPIO 14 (POT_PIN)  |
| Kaki kanan         | GND                |

> Pastikan sinyal dari potensiometer **tidak melebihi 3.3V**.

---

## Langkah Percobaan
1. Jalankan proyek di **Wokwi** atau unggah ke ESP32-S3.  https://wokwi.com/projects/447256423170230273
2. Buka **Serial Monitor** (baud **115200**).  
3. Putar potensiometer perlahan.  
4. Amati perubahan nilai ADC dan persentasenya di Serial Monitor.  
5. Perhatikan juga core tempat masing-masing task dijalankan.

---

## Tabel Hasil Pengamatan
| No | Posisi Potensiometer | Nilai ADC (0–4095) | Persentase (%) | Core Task |
|----|----------------------|--------------------|----------------|-----------|
| 1  | Paling kiri          | 0                  | 0.0            | Core 0 & 1 |
| 2  | Tengah               | 2048               | 50.0           | Core 0 & 1 |
| 3  | Paling kanan         | 4095               | 100.0          | Core 0 & 1 |


---

**Kesimpulan:**  
Percobaan ini menunjukkan multitasking **Dual-Core FreeRTOS** pada ESP32-S3, di mana satu core membaca nilai potensiometer dan core lain menampilkan hasilnya secara paralel melalui Serial Monitor.

## Gambar Demo
<img width="1920" height="1080" alt="Gambar Percobaan" src="https://github.com/user-attachments/assets/805fd268-d61f-4992-8465-cf90ff1fe1dd" />

## Video Demo
https://github.com/user-attachments/assets/9639b40c-42d8-4adf-937e-066576840a2e
