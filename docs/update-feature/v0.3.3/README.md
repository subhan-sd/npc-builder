# Laporan Pembaruan Fitur — v0.3.3 (Body Geometry Refactor, Injury-Amputation Parity & Transition Matrix)

**Tanggal:** 2026-10-25  
**Versi:** `v0.3.3`  
**Fitur Utama:** Ekstraksi Geometri Tubuh (`BodyGeometry`), Perbaikan Cedera pada Amputasi, Batas Cedera Ekstremitas, Kenaikan Versi Lintas Lapisan & Perluasan Matriks Acuan  
**Cabang:** `v0.3.3-body-geometry-refactor`  
**Dokumen Kebutuhan Terkait:** `docs/V0.3/V0.3.3/BRD_Refaktor_Geometri_Tubuh_V0.3.3.md`

---

## 1. Ringkasan Perubahan

Pembaruan versi `v0.3.3` mengimplementasikan refaktor arsitektur geometri tubuh (`BodyGeometry`) yang sebelumnya tersebar di dalam loop perenderan `generator.cpp`. Kini, nilai geometri dasar dihitung sekali dan menjadi sumber data bersama lintas tahap perenderan dan cedera. Selain itu, bug visual cedera pada anggota badan yang diamputasi diperbaiki secara tuntas, koordinat cedera tangan/kaki ditempatkan pada batas anggota badan sebenarnya, nomor versi lintas lapisan dinaikkan secara terkoordinasi dengan dukungan migrasi mundur tanpa celah, dan matriks acuan diperluas dari 7.360 menjadi 7.884 kombinasi uji.

---

## 2. Rincian Pembaruan

### A. Ekstraksi Geometri Tubuh Bersama (`BodyGeometry` & `compute_geometry()`)
- Berkas baru `core/include/npc/body_geometry.hpp` mendefinisikan struktur `BodyGeometry` yang menyimpan 7 nilai dasar bilangan bulat: `shoulder_w`, `waist_w`, `hips_w`, `thigh_thick`, `neck_w`, `arm_base_thick`, dan `leg_gap`.
- Fungsi murni `compute_geometry(mass, musc)` mengekstrak seluruh rumus proporsi tubuh kanonikal sehingga menjadi satu-satunya tempat peritungan dimensi fisik karakter.

### B. Penyimpanan Geometri ke Profil & Kenaikan Versi Terkoordinasi
- **Skema Profil (`PROFILE_SCHEMA_VERSION = 2`)**: `CharacterProfile` kini menyimpan `BodyGeometry` sebagai domain mandiri.
- **Serialisasi (`PROFILE_FORMAT_VERSION = 4`)**: Serializer menyimpan 7 nilai geometri. Deserializer mendukung migrasi mundur Format V3 (skema 1) dan Format V2 (legacy) dengan penghitungan otomatis nilai geometri dari anatomi.
- **Antarmuka C (`NPC_API_VERSION = 4`)**: Struktur `CNpcProfile` diperluas dengan 7 field `int16_t` untuk geometri tubuh. Pengecekan ukuran `struct_size` ketat menolak struktur biner lama dengan kode galat `NPC_ERR_VERSION_MISMATCH`.
- **Perluasan Godot (GDExtension)**: Fungsi `profile_to_dict` dan `dict_to_profile` menyertakan kamus `body_geometry`.

### C. Perbaikan Logika Cedera & Konsistensi Amputasi
- **Pengabaian Cedera pada Amputasi**: Loop cedera di `CharacterGenerator::render()` memverifikasi `cond.is_amputated(inj.body_part)`. Cedera pada anggota badan yang hilang dilewati sepenuhnya tanpa menggambar efek visual anomali.
- **Batas Geometri Tangan & Kaki**: Cedera pada `LeftHand`, `RightHand`, `LeftFoot`, dan `RightFoot` kini memiliki kotak pembatas koordinat sendiri di pergelangan tangan/kaki, tidak lagi jatuh ke badan (Torso).

### D. Perluasan Matriks Hasil Acuan & Pengujian Baru
- **Matriks Acuan Diperluas**: Berkas `tests/golden/matriks-acuan.txt` diregenerasi menjadi 7.884 baris (menambahkan 400 kasus cedera seluruh 10 bagian tubuh dalam 4 arah, 100 kasus kombinasi amputasi + cedera, dan 24 kasus profil batas ekstrem).
- **Pengujian Baru**:
  - `test_geometry.cpp`: Memverifikasi 100 benih menghasilkan geometri yang cocok dengan rumus kanonikal.
  - `test_transition.cpp`: Memverifikasi 12 ambang batas diskret dan tabel kemonotonan parameter.

---

## 3. Matriks Dampak & Pengurangan Risiko

| Aspek | Kondisi Sebelum v0.3.3 | Kondisi Sesudah v0.3.3 | Dampak / Mitigasi |
| :--- | :--- | :--- | :--- |
| **Geometri Tubuh** | Tersebar sebagai variabel lokal di fungsi render. | Terstruktur dalam `BodyGeometry` yang dihitung terpusat. | Mempermudah penambahan anatomi wanita, pakaian dinamis, dan armor modular. |
| **Cedera & Amputasi** | Cedera digambar di limb yang sudah diamputasi; tangan/kaki jatuh ke torso. | Cedera di limb amputasi dilewati; tangan/kaki di posisi fisik asli. | Menghilangkan artefak grafis anomali. Ditandai sebagai **perubahan visual**. |
| **Kontrak Antarmuka** | Versi API 3, Skema 1, Format V3. | Versi API 4, Skema 2, Format V4. | Pencegahan ketidakcocokan memori biner dengan fallback migrasi otomatis data lama. |
| **Cakupan Hasil Acuan** | 7.360 kombinasi (terbatas pada cedera torso & arah selatan). | 7.884 kombinasi (10 anggota badan x 4 arah, kombinasi, batas). | Menjamin 100% proteksi regresi visual lintas platform (Linux, Windows, macOS). |

---

## 4. Hasil Verifikasi Pengujian

Seluruh rangkaian pengujian terverifikasi **100% Lulus**:
- `CoreUnitTests`: **Passed**
- `GoldenFixtureTest`: **Passed** (7.884 matriks kombinasi hash piksel identik)
- `CApiParityTest`: **Passed** (100% kesetaraan byte RGBA C-API vs C++ Core pada API v4)
- `SerializerVerificationTest`: **Passed** (Format V2 round-trip & penolakan data korup)
- `CharacterProfileTest`: **Passed** (Profil default, validasi enum & amputasi, round-trip V4, migrasi V3 & V2, dan C-API v4)
- `BodyGeometryTest`: **Passed** (100 benih validasi kesetaraan formula geometri kanonikal)
- `TransitionTest`: **Passed** (12 ambang batas diskret & 5 pasangan kemonotonan parameter)
- `Godot Headless Smoke Test`: **Passed** (Instansiasi native, pembuatan profil dengan `body_geometry`, serialisasi, validasi, dan rendering)
