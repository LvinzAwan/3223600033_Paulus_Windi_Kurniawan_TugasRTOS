# README – Dual-Core Single-Buzzer (Twinkle / Happy Birthday) pada ESP32-S3 (Wokwi)

**Judul:** Simulasi Dual-Core FreeRTOS dengan 1 buzzer pada ESP32-S3 — pilih lagu lewat Serial

---

## Tujuan

* Menjalankan dua task FreeRTOS di dua core berbeda: **playerTask** (core 0) dan **inputTask** (core 1).  
* Memainkan **satu buzzer** (shared) yang digunakan untuk memutar dua lagu berbeda (Twinkle / Happy Birthday).  
* Mengendalikan lagu lewat input serial (`0`/`twinkle`, `1`/`happy`, `status`) dan mengamati output serial serta core yang mengeksekusi.

---

## Alat & Bahan (Simulasi)

* **Platform:** Wokwi Simulator (ESP32-S3)  
* **Board:** ESP32-S3 DevKitC  
* **Komponen virtual:** 1 × Buzzer  
* **Bahasa pemrograman:** Arduino (FreeRTOS + LEDC tone)

---

## Koneksi

| Komponen | Pin ESP32-S3 |
|-----------|---------------|
| Buzzer (+) | GPIO 14 |
| Buzzer (-) | GND |

> Pada simulasi, pastikan pin buzzer dihubungkan ke GPIO 14 sesuai variabel `buzzerPin` pada kode.

---

## Perintah Serial

| Perintah | Fungsi |
|-----------|--------|
| `0` atau `twinkle` | Memutar lagu **Twinkle Twinkle Little Star** |
| `1` atau `happy` | Memutar lagu **Happy Birthday** |
| `status` | Menampilkan lagu yang sedang aktif |

> Input diketik di Serial Monitor dengan baud rate **115200**.

---

## Langkah Percobaan

1. Buka proyek ESP32-S3 di **Wokwi**.  https://wokwi.com/projects/447246803300109313
2. Hubungkan buzzer ke **GPIO14** dan **GND**.  
3. Jalankan simulasi dengan klik **Run Simulation**.  
4. Buka **Serial Monitor** dengan baud rate **115200**.  
5. Amati pesan awal:
   * `Player task running on core 0`
   * `Input task running on core 1`
   * `Dual-core single-buzzer demo started. Default: Twinkle (song 0).`
6. Masukkan perintah berikut di Serial Monitor:
   * `0` atau `twinkle` → mainkan Twinkle Twinkle  
   * `1` atau `happy` → mainkan Happy Birthday  
   * `status` → periksa lagu yang sedang aktif  
7. Amati perubahan suara buzzer dan output log di Serial Monitor.

---

## Tabel Hasil Pengamatan

| No | Aksi | Output Serial Contoh | Keterangan |
|----|------|----------------------|-------------|
| 1 | Jalankan simulasi | `Player task running on core 0` / `Input task running on core 1` | Task berjalan di dua core berbeda |
| 2 | Default play | `[Player] Playing Twinkle (song 0)` | Lagu Twinkle dimainkan di core 0 |
| 3 | Ketik `1` di Serial | `currentSong -> 1 (Happy Birthday)` | Lagu diganti melalui task input di core 1 |
| 4 | Lagu berubah | `[Player] Playing Happy Birthday (song 1)` | Player task memutar Happy Birthday |
| 5 | Ketik `0` di Serial | `currentSong -> 0 (Twinkle)` | Lagu kembali ke Twinkle |
| 6 | Lagu berganti lagi | `[Player] Playing Twinkle (song 0)` | Pergantian lagu berhasil |

---

## Hasil Akhir

Simulasi berhasil menampilkan multitasking **dual-core FreeRTOS** pada ESP32-S3 dengan satu buzzer yang dapat berganti lagu secara dinamis berdasarkan input serial.

## Gambar Demo
<img width="1920" height="1080" alt="Gambar Percobaan" src="https://github.com/user-attachments/assets/27b3b7cc-a678-43e8-99bf-d59ad50e674b" />

## Video Demo
https://github.com/user-attachments/assets/cb44ce13-724c-481a-827c-22afa30d6ede

