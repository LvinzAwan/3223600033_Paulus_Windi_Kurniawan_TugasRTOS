# Sistem Kendali Kipas & Ventilasi Pendingin Industri Berbasis RTOS (ESP32-S3)

**Mini Project RTOS: Penerapan Semaphore, Mutex, Queue, Interrupt, Multicore, Deadlock, dan Starvation.**

![Platform](https://img.shields.io/badge/Platform-ESP32--S3-blue)
![OS](https://img.shields.io/badge/OS-FreeRTOS-orange)
![Status](https://img.shields.io/badge/Status-Completed-success)

## 👥 Tim Pengembang (Kelompok 2)

* **Paulus Windi Kurniawan** (NRP: 3223600033)
* **Ihsanta Zaki Sanjaya** (NRP: 3223600058)

---

## 🎥 Video Demo Project

Berikut adalah demonstrasi lengkap sistem.

https://github.com/user-attachments/assets/05737ac2-beea-43f2-b422-5eb2723c5427

---

## 📖 Latar Belakang & Masalah

Proyek ini dikembangkan untuk mengatasi permasalahan sistem pendingin konvensional di lingkungan industri bersuhu tinggi.

1.  **Permasalahan Lingkungan:** Suhu area produksi yang tinggi dapat menurunkan performa mesin, mempercepat kerusakan perangkat elektronik, dan meningkatkan risiko *overheating* yang menyebabkan *downtime*.
2.  **Keterbatasan Sistem Konvensional:**
    * **Non-Adaptif:** Kipas hanya bekerja dalam mode ON/OFF, tidak menyesuaikan dengan perubahan suhu.
    * **Ventilasi Statis:** Bukaan ventilasi tidak disesuaikan dengan kebutuhan panas.
    * **Tidak Ada Mode Manual:** Tidak tersedia intervensi operator saat dibutuhkan.
    * **Keamanan Rendah:** Tidak memiliki mekanisme *Emergency Stop* instan.
    * **Monitoring Terbatas:** Tidak *real-time* dan respons sistem lambat karena tidak mendukung *multitasking*.

---

## 🚀 Fitur & Potensi Sistem

Sistem ini menggunakan ESP32-S3 dengan FreeRTOS untuk menghadirkan fitur-fitur berikut:

* **Auto Mode:** Mengatur kecepatan kipas (Stepper) dan sudut ventilasi (Servo) secara otomatis berdasarkan data sensor DHT22.
* **Manual Mode:** Operator dapat mengontrol sistem secara langsung menggunakan Potentiometer dan Rotary Encoder.
* **Emergency Stop:** Tombol fisik berbasis *Hardware Interrupt* (ISR) untuk menghentikan sistem seketika dalam keadaan darurat.
* **Dashboard OLED:** Menampilkan suhu, mode, kecepatan kipas, sudut ventilasi, dan status alarm secara *real-time*.
* **Dual Core Processing:** Pembagian beban kerja antara Core 0 (Sensor/UI) dan Core 1 (Aktuator) untuk performa maksimal.
* **Sistem Alarm:** Indikator visual (LED Merah/Kuning/Hijau) dan audio (Buzzer) untuk status Normal, Warning, dan Danger.

---

## 🛠️ Perangkat Keras & Desain Sistem

### Blok Diagram
Sistem terdiri dari Input (Sensor, Encoder, Potensio, Tombol) yang memicu Proses (ESP32-S3 + RTOS) untuk mengendalikan Output (Motor, Display, Alarm).

### Konfigurasi Pin Input

| Komponen | Pin | Jenis Pin | Fungsi |
| :--- | :--- | :--- | :--- |
| **DHT22** | 4 | Digital Input | Pembacaan suhu & kelembapan |
| **Encoder CLK** | 10 | Digital Input | Deteksi putaran encoder |
| **Encoder DT** | 11 | Digital Input | Deteksi arah putaran |
| **Encoder SW** | 2 | Digital Input | Tombol toggle mode |
| **Potentiometer** | 1 (ADC1) | Analog Input | Kontrol fan manual |
| **Emergency Button** | 6 | Interrupt (FALLING) | Memicu mode darurat |

### Konfigurasi Pin Output

| Komponen | Pin | Jenis Pin | Fungsi |
| :--- | :--- | :--- | :--- |
| **Buzzer** | 7 | PWM (LEDC) | Alarm suara |
| **Servo Ventilasi** | 18 | PWM Servo (50Hz) | Mengatur sudut ventilasi |
| **Stepper STEP** | 13 | Digital Output | Pulsa langkah motor |
| **Stepper DIR** | 14 | Digital Output | Arah putaran motor |
| **OLED SDA** | 8 | I2C (SDA) | Data Display |
| **OLED SCL** | 9 | I2C (SCL) | Clock Display |
| **LED Indikator** | 17 (G), 16 (Y), 15 (R) | Digital Output | Status Normal, Warning, Danger |

> **Catatan Teknis:** Servo dijalankan pada frekuensi 50 Hz dan buzzer dipisahkan pada channel LEDC tersendiri untuk mencegah konflik sinyal PWM.

---

## 🧠 Desain Task RTOS (Manajemen Core & Prioritas)

Sistem membagi tugas ke dalam *Task* yang berjalan secara paralel di dua core prosesor:

| Task | Fungsi | Core | Prioritas | Alasan Desain |
| :--- | :--- | :--- | :--- | :--- |
| **TaskSensor** | Membaca suhu DHT22 | 0 | 3 | Memisahkan beban dari aktuator; prioritas sedang agar pembacaan stabil. |
| **TaskInputUser** | Membaca Encoder/Tombol | 0 | 3 | Input UI dipisah dari kontrol motor agar responsif. |
| **TaskControl** | Logika Utama (Otak Sistem) | 1 | 4 | Pusat pengambilan keputusan; membutuhkan performa tinggi (Core 1). |
| **TaskFanMotor** | Menggerakkan Stepper | 1 | 5 | **Prioritas Tertinggi.** Meminimalkan latensi gerakan motor pendingin. |
| **TaskVentControl** | Menggerakkan Servo | 1 | 4 | Disinkronkan dengan motor; prioritas tinggi agar gerakan halus. |
| **TaskTempDisp** | Menampilkan Suhu | 0 | 2 | Prioritas rendah karena update layar tidak kritis (hanya visual). |
| **TaskStatusDisp** | Menampilkan Status Mode | 0 | 2 | Prioritas rendah, update info status sekunder. |
| **TaskAlarm** | Mengatur LED & Buzzer | 0 | 4 | Prioritas tinggi di Core 0 agar peringatan bahaya muncul segera. |

---

## 🔄 Alur Kerja Sistem

1.  **Inisialisasi:** Pembuatan Queue, Semaphore, Mutex, dan Task.
2.  **Sensor:** `TaskSensor` membaca suhu secara periodik dan mengirim ke Queue.
3.  **Input:** `TaskInputUser` membaca Encoder/Potensio. Event ganti mode dikirim via Queue.
4.  **Kendali (Control):** `TaskControl` menerima data, menghitung logika (Auto/Manual), dan mengupdate variabel global `SharedState` menggunakan **Mutex**.
5.  **Aktuator:** `TaskFanMotor` dan `TaskVentControl` membaca state dan menggerakkan hardware.
6.  **Tampilan:** `TaskDisplay` mengambil token **Semaphore** sebelum mengakses OLED.
7.  **Alarm:** `TaskAlarm` memantau suhu untuk menyalakan LED/Buzzer.
8.  **Emergency:** Tombol ditekan -> **ISR** aktif -> `TaskControl` mematikan semua output.

---

## 🧪 Analisis Skenario Pengujian (Good vs Bad Practice)

Proyek ini menyertakan menu simulasi (via Serial Monitor) untuk membuktikan pentingnya konsep RTOS. Berikut adalah detail analisis beserta video demo perbandingan:

### 1. Queue (Antrean Data)
Mengirim data antar task secara asinkron dan aman.
* **Good:** Menggunakan `xQueueSend`. Komunikasi teratur, data mode tersimpan rapi.
* **Bad (No Queue):** *Direct Variable Access*. Data tumpang tindih secara paksa, menyebabkan inkonsistensi.

| Good Implementation (With Queue) | Bad Implementation (No Queue) |
| :---: | :---: |
| <video src="https://github.com/user-attachments/assets/ddd02861-8a75-4da6-8d68-17f05ea51fa9" controls width="300"></video> | <video src="https://github.com/user-attachments/assets/0f2c57a7-5bc5-4208-a6aa-0e775d78b598" controls width="300"></video> |

### 2. Mutex (Mutual Exclusion)
Melindungi variabel global dari akses ganda.
* **Good:** Menggunakan `xSemaphoreTake`. Akses data aman, gerakan Servo stabil.
* **Bad (No Mutex):** *Race Condition*. Nilai ventilasi tertimpa.

| Good Implementation (With Mutex) | Bad Implementation (Race Condition) |
| :---: | :---: |
| <video src="https://github.com/user-attachments/assets/e7bb046e-685f-4ff2-8d5c-27d8dc4bb898" controls width="300"></video> | <video src="https://github.com/user-attachments/assets/8d9f0e91-9832-46bb-8477-dff7a3b1b27e" controls width="300"></video> |

### 3. Semaphore (Manajemen Sumber Daya)
Mengatur giliran akses ke layar OLED (I2C).
* **Good:** Akses OLED bergiliran. Tampilan stabil.
* **Bad (No Semaphore):** Dua task menulis bersamaan. Layar *glitch* dan teks berkedip.

| Good Implementation (With Semaphore) | Bad Implementation (Display Glitch) |
| :---: | :---: |
| <video src="https://github.com/user-attachments/assets/d31d435c-2e34-4e2b-b21a-50904f7a71cf" controls width="300"></video> | <video src="https://github.com/user-attachments/assets/fbd59a50-b13e-4d91-8bc5-827611f19aa6" controls width="300"></video> |

### 4. Multicore (Pemrosesan Paralel)
Membagi beban kerja ke Core 0 dan Core 1.
* **Good:** Motor (Core 1) dan UI (Core 0) berjalan paralel. Sistem mulus.
* **Bad (No Multicore):** Semua antre di Core 0. Motor tersendat (*lag*) saat layar refresh.

| Good Implementation (Dual Core) | Bad Implementation (Single Core) |
| :---: | :---: |
| <video src="https://github.com/user-attachments/assets/c4db877e-24d8-4a07-a514-02dd5cf0e54d" controls width="300"></video> | <video src="https://github.com/user-attachments/assets/dd710d8e-55a2-41f1-b897-50da83bffad8" controls width="300"></video> |

### 5. Interrupt (ISR)
Menangani Emergency Stop dengan prioritas tertinggi.
* **Good:** ISR hanya mengubah flag (cepat). Respons tombol instan.
* **Bad (Bad ISR):** Ada komputasi/delay di ISR. Sistem macet sesaat, menahan CPU.

| Good Implementation (Fast ISR) | Bad Implementation (Blocking ISR) |
| :---: | :---: |
| <video src="https://github.com/user-attachments/assets/41075df4-1839-41d8-9193-dda0954007d3" controls width="300"></video> | <video src="https://github.com/user-attachments/assets/67157216-3e69-4880-a119-c8149b0e8854" controls width="300"></video> |

### 6. Starvation (Kelaparan Task)
Manajemen prioritas agar semua task kebagian jatah CPU.
* **Good (Fair):** Prioritas seimbang. Motor dan Layar berjalan bersamaan.
* **Bad (Starvation):** Task Layar prioritas terlalu tinggi. Aktuator menjadi lambat yang ditunjukkan dengan gerakan motor yang lebih lambat

| Good Implementation (Fair Prio) | Bad Implementation (Starvation) |
| :---: | :---: |
| <video src="https://github.com/user-attachments/assets/23cce8bf-220f-4488-a71c-9f3d9ebc8fe4" controls width="300"></video> | <video src="https://github.com/user-attachments/assets/c8ab5297-e045-43f0-8694-cfc0a9ad71b4" controls width="300"></video> |

### 7. Deadlock (Jalan Buntu)
Mencegah sistem saling tunggu resource selamanya.
* **Normal:** Sistem berjalan lancar.
* **Deadlock:** Mutex diambil tapi tidak dikembalikan. Sistem *Freeze* total.

| Normal Operation | Deadlock Simulation |
| :---: | :---: |
| <video src="https://github.com/user-attachments/assets/dfa47091-ccf8-4211-885b-77ff5276ac01" controls width="300"></video> | <video src="https://github.com/user-attachments/assets/f201c25f-7298-41e4-923d-a3c875b34d82" controls width="300"></video> |

---

## 📥 Instalasi & Penggunaan

1.  **Persiapan Library:**
    * `Adafruit GFX` & `Adafruit SSD1306`
    * `ESP32Servo`
    * `AccelStepper`
    * `DHT sensor library`
2.  **Upload:**
    * Buka file `.ino` di Arduino IDE.
    * Pilih board **ESP32S3 Dev Module**.
    * Upload program.
3.  **Jalankan Demo:**
    * Buka **Serial Monitor** (Baudrate 115200).
    * Ketik angka **0-7** untuk memilih skenario demo:
        * `0`: Normal (Good)
        * `1`: No Queue
        * `2`: No Mutex
        * `3`: No Semaphore
        * `4`: Bad ISR
        * `5`: No Multicore
        * `6`: Deadlock
        * `7`: Starvation

---
