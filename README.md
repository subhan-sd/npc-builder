# Living World — Procedural NPC Builder

![Living World NPC Builder Banner](docs/references/npc_anatomy_v021_profiles.png)

[![C++20](https://img.shields.io/badge/Language-C%2B%2B20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![Engine](https://img.shields.io/badge/Engine-Godot%204.x-478cbf.svg)](https://godotengine.org/)
[![Build Status](https://img.shields.io/badge/Tests-8%2F8%20Passing-brightgreen.svg)]()
[![Version](https://img.shields.io/badge/Version-v0.2.1-orange.svg)]()

> **Living World NPC Builder** adalah sistem generator karakter piksel prosedural deterministik berbasis **C++20** untuk simulasi dunia dinamis, dilengkapi preview studio interaktif di **Godot 4.x**.

---

## 📌 Status & Progress Saat Ini

**Versi Terkini:** `v0.2.1` — *Visual Anatomy Refinement & Procedural Human Body System*  
**Fase Saat Ini:** Fondasi anatomi manusia natural tanpa pakaian selesai terverifikasi sebelum masuk ke variasi gender dan pakaian.

### 🖼️ Studio Preview (Godot 4.x Runtime)

![Godot Preview Window](docs/references/godot_preview_running_window.png)

---

## 🚀 Fitur yang Sudah Selesai (Completed)

### 1. Anatomi Piksel Prosedural & Profil Tubuh (v0.2.1)
- **Model Studi Anatomi Netral (Unclothed)**: Menghilangkan render pakaian kaku agar proporsi tubuh dapat diinspeksi secara objektif.
- **Profil Siluet Terkalibrasi (Referensi Klasik)**:
  - **Slim Jim**: Frame ramping, anggota tubuh lebih ramping, celah kaki natural.
  - **Trim Jim**: Proporsi baseline atletis seimbang.
  - **Gym Jim**: Bahu lebar (*broad deltoid*), siluet *V-taper*, kontur dada (*pectoral crease*), dan aksen abdominal.
  - **Chin Jim**: Pinggang dan pinggul lebar, perut membulat (*paunch*).
- **Artikulasi Organik**: Transisi trapezius melandai, takik siku, otot lengan bawah, ibu jari tangan, celah lutut, lengkungan betis lateral, dan tumpuan kaki stabil.

| Slim Jim | Trim Jim | Gym Jim | Chin Jim | Side (Tegak) | Side (Bungkuk) |
| :---: | :---: | :---: | :---: | :---: | :---: |
| ![Slim Jim](docs/references/fixture_slim_south.png) | ![Trim Jim](docs/references/fixture_trim_south.png) | ![Gym Jim](docs/references/fixture_gym_south.png) | ![Chin Jim](docs/references/fixture_chin_south.png) | ![Upright](docs/references/fixture_posture_upright_east.png) | ![Hunched](docs/references/fixture_posture_hunched_east.png) |

### 2. Deformasi Posture Terpadu (Multi-part Coordinate Shift)
- Slider **Posture** (0.0 - 1.0) menghasilkan deformasi tulang yang selaras:
  - Kemiringan kepala maju (hingga 3px).
  - Kemiringan torso atas maju (hingga 1.5px).
  - Penyesuaian leher dan pergeseran bahu tampak samping (*East/West*).
  - Kompresi trapezius pada tampak depan (*South*).

### 3. Palet Warna Harmonik 5-Tone Ramp
Setiap warna kulit diturunkan menjadi 5 tingkatan kontras:
- `Highlight` -> `Base Tone` -> `Shadow Tone` -> `Deep Crease` -> `Deep Outline`.

### 4. Animasi & Arah Pandang
- **6 State Animasi**: `Idle`, `Walk`, `Run`, `Attack`, `Fall`, `Swim`.
- **4 Arah Pandang**: `South (Front)`, `North (Back)`, `East (Right)`, `West (Left)`.

### 5. Kondisi Fisik & Simulasi Kerusakan
- **Amputasi Cascading**: 10 bagian tubuh (`Head`, `Torso`, `Left/Right Arm`, `Left/Right Hand`, `Left/Right Leg`, `Left/Right Foot`). Jika lengan diamputasi, tangan otomatis tidak dirender.
- **Visual Injuries**: `Cut` / `Blood`, `Bandage` (balutan perban melintang), `Scar` (bekas luka), dan `Bruise` (memar). Semua luka terikat pada koordinat relatif anggota tubuh dan ikut berayun saat animasi.

### 6. C++ Core Authoritative & Cross-Platform Bridge
- Algoritma C++20 murni dengan alokasi memori terkontrol di `core/src/generator.cpp`.
- GDScript bridge di `godot/scripts/npc_bridge.gd` mensinkronkan logika 1:1 sehingga preview dapat berjalan native di Windows/Linux tanpa ketergantungan DLL eksternal.

---

## 🗺️ Roadmap Proyek

- [x] **v0.1**: Baseline flat-rectangle procedural renderer & Godot preview.
- [x] **v0.2**: 5-tone palette ramp, 10 body parts, cascade amputation, physical condition injuries.
- [x] **v0.2.1**: Natural human anatomy, curvilinear torso, articulate limbs, multi-part posture deformation, unclothed preview.
- [ ] **v0.3**:
  - Profil Anatomi Wanita (*Female body proportions, pelvic/chest variation*).
  - Sistem Layering Pakaian Dinamis (*Modular clothing, armor, and equipment*).
  - Variasi Wajah Lanjutan & Gaya Rambut Modular.
- [ ] **v0.4**:
  - Sistem Penuaan Prosedural (*Aging, wrinkles, hair graying*).
  - Integrasi API C ke Engine C++ untuk simulasi massal (ribuan NPC simultan).

---

## 🛠️ Cara Build & Menjalankan

### Persyaratan
- C++20 Compiler (GCC 13+, Clang 16+, atau MSVC 2022)
- CMake 3.20+
- Godot Engine 4.x (Direkomendasikan 4.3 atau 4.7+)

### 1. Build C++ Core & Test Suite
```bash
# Konfigurasi dan compile
cmake -B build -S .
cmake --build build

# Jalankan unit test
./build/npc_test
```

### 2. Menjalankan Godot Preview Studio
1. Buka Godot Engine 4.x.
2. Import project yang berada di folder `godot/`.
3. Jalankan scene utama `res://scenes/preview.tscn` (tekan `F5` atau `F6`).

---

## 📂 Struktur Repositori

```text
├── core/
│   ├── include/npc/       # Header C++ (types, appearance, conditions, pixel_buffer)
│   └── src/               # Implementasi C++ authoritative generator & serializer
├── godot/
│   ├── scenes/            # Scene UI Godot (preview.tscn)
│   ├── scripts/           # Script UI & GDScript sync bridge (npc_bridge.gd)
│   └── project.godot      # Konfigurasi Godot 4.x project
├── tests/
│   ├── test_generator.cpp # Unit test C++ (RNG, posture, silhouettes, amputasi)
│   └── generate_fixtures.cpp # Tool generator fixture perbandingan gambar
├── docs/
│   ├── BRD_NPC_BUILDER_V0.2.1.md
│   ├── README_v0.2.1.md   # Laporan rilis visual v0.2.1
│   └── references/        # Fixture gambar & screenshot hasil pengujian
└── README.md              # Dokumentasi utama proyek
```

---

## 📄 Lisensi
Bagian dari proyek **Living World Simulation Engine**.
