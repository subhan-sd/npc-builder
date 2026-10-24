# Living World — Procedural NPC Builder

![Living World NPC Builder Banner](docs/references/npc_anatomy_v021_profiles.png)

[![C++20](https://img.shields.io/badge/Language-C%2B%2B20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![Engine](https://img.shields.io/badge/Engine-Godot%204.x-478cbf.svg)](https://godotengine.org/)
[![CI](https://github.com/subhan-sd/npc-builder/actions/workflows/ci.yml/badge.svg)](https://github.com/subhan-sd/npc-builder/actions/workflows/ci.yml)
[![Version](https://img.shields.io/badge/Version-v0.3.1-orange.svg)]()

> **Living World NPC Builder** adalah sistem generator karakter piksel prosedural deterministik berbasis **C++20** untuk simulasi dunia dinamis, dilengkapi preview studio interaktif di **Godot 4.x** melalui **GDExtension**.

---

## 📌 Status & Progress Saat Ini

**Versi Terkini:** `v0.3.1` — *Character Profile System & Domain Separation*  
**Fase Saat Ini:** Menghadirkan model konfigurasi kanonikal `CharacterProfile` dengan pemisahan domain anatomi, tampilan visual, dan kondisi fisik. C++ Core bertindak sebagai satu-satunya sumber kebenaran (*single source of truth*) yang terintegrasi ke C-API v3, Serializer V3 (dengan backward compatibility V2), dan adapter GDExtension `NPCGenerator`. Seluruh 5 target pengujian CTest lulus 100%.

### 🖼️ Studio Preview (Godot 4.x GDExtension Runtime)

![Godot Preview Window GDExtension](docs/references/godot_preview_gdextension_running.png)

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

### 6. C++ Core Single Source of Truth & GDExtension Architecture
- Algoritma C++20 murni dengan alokasi memori terkontrol di `core/src/generator.cpp` sebagai satu-satunya sumber kebenaran (*single source of truth*).
- Integrasi GDExtension native (`extension/` berbasis `godot-cpp`) mendaftarkan kelas `NPCGenerator` ke `ClassDB`.
- Menghilangkan duplikasi logika generator dan rasterizer di GDScript.
- Waktu render C++ inti rata-rata ~1.2 mikrodetik per karakter dengan determinisme 100% terverifikasi melalui matriks 7.360 golden fixture hash FNV-1a.

### 7. Character Profile System & Domain Separation (v0.3.1)
- **Model Konfigurasi Terstruktur (`CharacterProfile`)**:
  - `AnatomyProfile`: Proporsi tubuh, skala massa, tinggi, rasio anggota tubuh, dan deformasi postur.
  - `AppearanceProfile`: Pilihan bentuk mata, hidung, mulut, gaya rambut, ekspresi, dan warna RGBA.
  - `PhysicalConditionProfile`: Kondisi luka (`injuries`) dan amputasi bertingkat (`amputations`).
  - `GenerationProfile`: Pengenal NPC, benih acak (*seed*), dan versi generator.
- **Validasi & Normalisasi Otomatis**: Mendeteksi nilai NaN/Inf, rentang [0.0, 1.0], dan enum tidak sah melalui `ProfileValidator`.
- **Serialisasi Format V3 & Kompatibilitas**: Mendukung format profil V3 serta membaca format V2 lama tanpa kehilangan data.
- **Eksposur Lintas Lapisan**: Tersedia melalui C-API v3 (`CNpcProfile`) dan GDExtension (`create_default_profile`, `generate_profile`, `validate_profile`, `serialize_profile`, `deserialize_profile`, `render_profile`).

---

## 🗺️ Roadmap Proyek

- [x] **v0.1**: Baseline flat-rectangle procedural renderer & Godot preview.
- [x] **v0.2**: 5-tone palette ramp, 10 body parts, cascade amputation, physical condition injuries.
- [x] **v0.2.1**: Natural human anatomy, curvilinear torso, articulate limbs, multi-part posture deformation, unclothed preview.
- [x] **v0.2.3**: GDExtension native architecture migration, C++ core single source of truth, C-API v2 with round-trip serializer, Release-proof CTest suite & 7,360 golden fixture matrix.
- [x] **v0.3.1**: Character Profile System, domain separation (anatomy, appearance, conditions), ProfileValidator, Serializer V3, C-API v3, GDExtension profile bridge.
- [ ] **v0.3.2+**:
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

### 1. Build C++ Core, Tests, & GDExtension
```bash
# Konfigurasi dan compile (Release)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# Jalankan seluruh rangkaian test otomatis CTest
ctest --test-dir build --output-on-failure

# Jalankan benchmark render mikro
./build/benchmark_render
```

### 2. Menjalankan Godot Preview Studio (GDExtension)
1. Pastikan ekstensi `godot/bin/libnpc_builder_extension.so` (Linux) atau `.dll` (Windows) telah terkompilasi melalui CMake.
2. Buka Godot Engine 4.x (res:// project di folder `godot/`).
3. Jalankan scene utama `res://scenes/preview.tscn` (tekan `F5` atau `F6`). Godot otomatis memuat kelas native `NPCGenerator`.

---

## 📂 Struktur Repositori

```text
├── core/
│   ├── include/npc/       # Header C++ (types, appearance, conditions, profile.hpp, pixel_buffer, c_api.h)
│   └── src/               # Implementasi C++ authoritative generator, serializer, & C API v3
├── extension/             # Modul native GDExtension (adapter NPCGenerator & register_types)
├── godot/
│   ├── bin/               # Manifest .gdextension & compiled dynamic libraries (.so / .dll)
│   ├── scenes/            # Scene UI Godot (preview.tscn)
│   ├── scripts/           # Script UI & thin GDExtension bridge (npc_bridge.gd)
│   └── project.godot      # Konfigurasi Godot 4.x project
├── tests/
│   ├── test_generator.cpp # Unit test C++ (RNG, posture, silhouettes, amputasi)
│   ├── test_profile.cpp   # Unit test kanonikal CharacterProfile v0.3.1
│   ├── test_c_api.cpp     # Test kesetaraan 100% byte RGBA C API vs C++ Core
│   ├── test_serializer.cpp# Test round-trip serializer & validasi data rusak
│   ├── test_golden.cpp    # Test regresi 7.360 golden fixture hashes FNV-1a
│   ├── generate_fixtures.cpp # Tool generator fixture acuan
│   └── benchmark_render.cpp  # Microbenchmark kecepatan render (~1.2 us/frame)
├── docs/
│   ├── BRD_NPC_BUILDER_V0.2.1.md
│   ├── BRD_NPC_Builder_GDExtension_dilanjutkan.md
│   ├── LAPORAN_AUDIT_DAN_RENCANA_MIGRASI_GDEXTENSION.md
│   ├── README_v0.2.1.md   # Laporan rilis visual v0.2.1
│   ├── V0.3/              # Dokumen kebutuhan bisnis v0.3.1 (Character Profile System)
│   └── references/        # Fixture gambar & screenshot hasil pengujian
└── README.md              # Dokumentasi utama proyek
```

---

## 📄 Lisensi
Bagian dari proyek **Living World Simulation Engine**.
