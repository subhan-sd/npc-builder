# Laporan Pembaruan Fitur — v0.3.1 (Character Profile System)

**Tanggal:** 2026-10-24  
**Versi:** `v0.3.1`  
**Fitur Utama:** Sistem Profil Karakter (*Character Profile System*) & Pemisahan Domain Konfigurasi  
**Cabang:** `v0.3.1-character-profile-system`  
**Dokumen Kebutuhan Terkait:** `docs/V0.3/BRD_CHARACER_PROFIL_SYSTEM_V0.3.1.md`

---

## 1. Ringkasan Perubahan

Sebelum versi `v0.3.1`, konfigurasi karakter tersimpan dalam satu struktur datar (`HumanAppearance`). Pendekatan ini menyulitkan penambahan variasi tubuh baru, modular pakaian, atau kondisi fisik tanpa mengubah alur perenderan dan serialisasi sekaligus.

Pada versi `v0.3.1`, konfigurasi distandarisasi ke dalam model kanonikal **`CharacterProfile`** berbasis pemisahan domain tanggung jawab (*domain separation*), dilengkapi validator mandiri, serialisasi format V3, jembatan C-API v3, dan modul GDExtension native.

---

## 2. Rincian Fitur Baru

### A. Model Profil Karakter Berbasis Domain (`core/include/npc/profile.hpp`)
Konfigurasi dibagi menjadi 4 domain independen:
1. **`AnatomyProfile`**: Struktur fisik dan proporsi tubuh (`height`, `body_mass`, `muscularity`, `posture`, `head_scale`, `torso_ratio`, `arm_ratio`, `leg_ratio`).
2. **`AppearanceProfile`**: Fitur visual independen dan warna (`eye_shape`, `eye_spacing`, `nose_shape`, `mouth_shape`, `hair_style`, ekspresi, serta warna RGBA untuk kulit, mata, rambut, baju, dan celana).
3. **`PhysicalConditionProfile`**: Kondisi fisik tubuh, mencakup status amputasi bertingkat 10 anggota badan (`amputations`) dan daftar luka fisik (`injuries`).
4. **`GenerationProfile`**: Parameter identitas dan reproduksi deterministik (`npc_id`, `seed`, `generator_version`).

### B. Validasi & Normalisasi Aman (`ProfileValidator`)
- **Validasi Ketat**: Memeriksa versi skema (`schema_version`), memastikan tidak ada nilai non-finit (`NaN` atau `Inf`), memeriksa rentang angka valid `[0.0, 1.0]`, dan memverifikasi batas nilai `enum`.
- **Normalisasi Adaptif**: Fungsi `ProfileValidator::normalize()` melakukan penjepitan (*clamping*) nilai pecahan secara deterministik jika sedikit meleset tanpa memicu kegagalan fatal (*crash*).

### C. Serialisasi Format V3 & Kompatibilitas Mundur (`core/include/npc/serializer.hpp`)
- **Format Profil V3**: Menyimpan seluruh domain profil, termasuk seluruh status amputasi dan daftar luka secara deterministik.
- **Dukungan Format Lama (V2)**: Fungsi deserializer otomatis mengenali format V2 dan memigrasikannya menjadi `CharacterProfile` valid tanpa data hilang.

### D. Eksposur Lintas Lapisan (C-API v3 & GDExtension)
- **C-API v3 (`core/include/npc/c_api.h`, `core/src/c_api.cpp`)**:
  - `CNpcProfile`: Struktur biner stabil untuk konfigurasi profil.
  - `npc_profile_create_default()`, `npc_profile_validate()`, dan `npc_profile_render()`.
- **GDExtension Adapter (`extension/src/npc_generator.cpp`)**:
  - Mengekspos metode native: `create_default_profile`, `generate_profile`, `validate_profile`, `serialize_profile`, `deserialize_profile`, dan `render_profile`.
- **Godot Bridge (`godot/scripts/npc_bridge.gd`)**:
  - Skrip antarmuka tipis memanggil metode native GDExtension tanpa duplikasi logika generator di GDScript.

---

## 3. Dampak Arsitektur & Risiko (Impact & Risks)

| Aspek | Kondisi Sebelum v0.3.1 | Kondisi Sesudah v0.3.1 | Dampak / Mitigasi |
| :--- | :--- | :--- | :--- |
| **Pemisahan Data** | Bercampur dalam satu struktur datar `HumanAppearance`. | Terbagi rapi ke 4 domain mandiri. | Mengubah proporsi tubuh tidak lagi berisiko menghapus data luka atau warna. |
| **Regresi Visual** | Rentan pergeseran piksel saat penambahan field baru. | Teruji deterministik dengan 7.360 acuan. | **0% regresi visual**; semua golden fixtures lama tetap lulus 100%. |
| **Kompabilitas Simpanan** | Format serialisasi flat V2. | Format V3 terstruktur + migrasi V2 otomatis. | Berkas simpanan lama pemain/dunia tetap dapat dimuat dengan aman. |
| **Ekstensi Mendatang** | Memerlukan modifikasi renderer untuk setiap parameter baru. | Cukup memperluas domain profil terkait. | Membuka jalan mulus bagi variasi wanita (`v0.3.2`), pakaian modular, dan ras fantasi. |

---

## 4. Hasil Verifikasi Pengujian

Seluruh rangkaian pengujian terverifikasi lulus:
- `CoreUnitTests`: **Passed** (RNG, siluet anatomi, dan deformasi postur).
- `GoldenFixtureTest`: **Passed** (7.360 matriks kombinasi hash piksel identik).
- `CApiParityTest`: **Passed** (100% kesetaraan byte RGBA C-API vs C++ Core).
- `SerializerVerificationTest`: **Passed** (Round-trip serialisasi dan penolakan data korup).
- `CharacterProfileTest`: **Passed** (Uji konstruksi default, validasi domain, serialisasi V3 round-trip 100 seed, migrasi V2, dan C-API profile render).
- `Godot Smoke Test`: **Passed** (`smoke_test_profile.gd` terverifikasi via Godot Engine headless runtime).
