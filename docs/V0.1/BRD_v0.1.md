# BRD — NPC Builder
## Procedural Human Character Generation, Physical Conditions & Animation

**Status:** Draft v0.1 — Technical Review  
**Project:** Living World  
**Module:** NPC Builder  
**Race Scope:** Human  
**Engine:** Godot 4  
**Simulation & Character Data:** C++20  
**Integration:** GDExtension / C-ABI Shared Library  
**Primary Goal:** Menghasilkan NPC manusia dengan variasi tubuh, wajah, anggota tubuh, kondisi fisik, dan animasi secara prosedural, konsisten, serta deterministik.

---

# 1. Executive Summary

NPC Builder adalah modul yang bertanggung jawab membentuk dan menampilkan karakter manusia dalam game Living World.

Sistem harus mampu menghasilkan karakter manusia dengan penampilan berbeda berdasarkan parameter tubuh, fitur wajah, anggota tubuh, kondisi fisik, serta seed yang menentukan variasi penampilan.

Karakter tidak boleh sekadar menjadi kumpulan bentuk yang berubah secara acak. Setiap NPC harus mempunyai identitas visual yang relatif konsisten selama permainan, termasuk ketika berjalan, berlari, menyerang, jatuh, berenang, atau mengalami cedera.

NPC Builder dirancang sebagai fondasi bagi sistem kehidupan dan simulasi Living World. Data karakter nantinya dapat dihubungkan dengan usia, genetika, pekerjaan, keluarga, konflik, pertempuran, dan sistem sosial.

## 1.1 Tujuan utama

1. Menghasilkan karakter manusia tanpa mewajibkan sprite individual buatan manual.
2. Mendukung variasi bentuk tubuh, wajah, rambut, dan proporsi anggota tubuh.
3. Merepresentasikan amputasi, luka, darah, perban, dan bekas cedera.
4. Menyediakan animasi dasar untuk aktivitas NPC.
5. Menjamin konsistensi identitas visual karakter di antara berbagai animasi dan kondisi.
6. Memungkinkan karakter diregenerasi menggunakan parameter dan seed yang sama.
7. Menyediakan antarmuka yang dapat dipakai modul simulasi C++ dan renderer Godot.

## 1.2 Prinsip desain

- **Procedural first:** bentuk dasar dihasilkan melalui algoritma, geometri, dan palet warna.
- **Deterministic generation:** seed yang sama dengan parameter dan versi generator yang sama menghasilkan hasil visual yang sama.
- **Data-driven:** penampilan karakter ditentukan oleh data, bukan hardcoded untuk setiap NPC.
- **Simulation-rendering separation:** logika dan data karakter dimiliki C++, sementara Godot menampilkan hasilnya.
- **Composable appearance:** tubuh, wajah, rambut, pakaian, cedera, dan animasi merupakan lapisan yang dapat dikombinasikan.
- **Graceful degradation:** kondisi yang tidak didukung tidak boleh membuat karakter gagal dirender atau menyebabkan crash.
- **Extensible:** arsitektur harus memungkinkan penambahan ras lain di masa mendatang tanpa mendesain ulang seluruh sistem.

---

# 2. Ruang Lingkup

## 2.1 In scope

### A. Bentuk tubuh
- Kurus.
- Gemuk.
- Tinggi.
- Pendek.
- Berotot.
- Bungkuk.
- Kombinasi beberapa karakteristik tubuh.

### B. Wajah
- Mata.
- Hidung.
- Mulut.
- Rambut.
- Ekspresi.

### C. Anggota tubuh
- Kepala.
- Torso atau badan.
- Lengan kiri dan kanan.
- Tangan.
- Kaki kiri dan kanan.
- Kaki atau telapak kaki sebagai bagian visual.
- Status amputasi anggota tubuh.

### D. Kondisi fisik
- Luka.
- Darah.
- Perban.
- Bekas cedera.
- Dampak visual kondisi fisik terhadap postur dan animasi.

### E. Animasi
- Idle.
- Berjalan.
- Berlari.
- Menyerang.
- Jatuh.
- Berenang.

### F. Variasi NPC
- Seed individual.
- Parameter pembentuk penampilan.
- Variasi proporsi dan fitur.
- Konsistensi visual antar-frame dan antar-state.
- Penyimpanan serta pemulihan data penampilan.

## 2.2 Out of scope untuk versi awal

Fitur berikut tidak menjadi persyaratan implementasi awal:

- Ras nonmanusia seperti elf, orc, giant, atau dwarf.
- Sistem genetika antargenerasi yang menentukan penampilan keturunan.
- Simulasi anatomi medis atau kerusakan organ.
- Simulasi fisika ragdoll kompleks.
- Facial animation tingkat lanjut, lip-sync, dan ekspresi dialog.
- Sistem equipment lengkap, armor, senjata, serta pergantian pakaian kompleks.
- Generator sprite sheet artistik dengan kualitas setara produksi game komersial.
- Sistem multiplayer dan sinkronisasi karakter lintas jaringan.

---

# 3. Persyaratan Teknis dan Arsitektur

## 3.1 Pembagian tanggung jawab

### C++20 — Character Data & Generation

C++ bertanggung jawab atas:
- Definisi data karakter.
- Parameter bentuk tubuh dan wajah.
- Seed dan determinisme generator.
- Status anggota tubuh.
- Data luka, perban, dan bekas cedera.
- Pemilihan state animasi berdasarkan perintah dari simulasi.
- Validasi data karakter.
- Penyediaan data penampilan untuk renderer.
- Serialisasi data karakter yang diperlukan untuk save/load.

### Godot 4 — Rendering & Visual Presentation

Godot bertanggung jawab atas:
- Menampilkan karakter.
- Menggambar bentuk dan lapisan visual.
- Transformasi, posisi, rotasi, dan skala visual.
- Playback animasi.
- Transisi antar-state animasi.
- Kamera dan zoom.
- UI untuk preview dan debugging karakter.
- Visualisasi parameter karakter.

## 3.2 Strategi rendering

Procedural pixel rendering 2D (baseline canvas 32x48) menggunakan nearest-neighbor filter, palet warna kulit, pakaian, rambut, dan physical conditions.

---

# 4. Functional Requirements

- **NPC-01 (Body Shape):** Parameter `height`, `body_mass`, `muscularity`, `posture`, `head_scale`, `torso_ratio`, `arm_ratio`, `leg_ratio`.
- **NPC-02 (Face):** Mata, hidung, mulut, model rambut (bald, short, medium, long), ekspresi (neutral, happy, sad, angry, in_pain, surprised).
- **NPC-03 (Body Parts & Amputation):** 10 bagian tubuh diskrit (`Head`, `Torso`, `LeftArm`, `RightArm`, `LeftHand`, `RightHand`, `LeftLeg`, `RightLeg`, `LeftFoot`, `RightFoot`). Amputasi lengan atau kaki otomatis menghapus anak ranting visualnya secara permanen selama state.
- **NPC-04 (Physical Condition & Injury):** Luka (cut, bruise, burn, scar, bandage, blood stain) terikat pada bagian tubuh spesifik secara prosedural deterministik.
- **NPC-05 (Animation System):** 6 state (`Idle`, `Walk`, `Run`, `Attack`, `Fall`, `Swim`) x 4 arah pandang (`South`, `North`, `East`, `West`).
- **NPC-06 (Deterministic NPC Variation):** Generator SplitMix64 PRNG per `appearance_seed`. Serializer save/load berbasis teks tanpa kehilangan presisi floating point.
