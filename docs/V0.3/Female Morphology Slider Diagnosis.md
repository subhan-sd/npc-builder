# Laporan Diagnosa: Slider Morfologi Wanita (Chest Curve & Torso Ratio) Tidak Berdampak Visual

**Tanggal:** 2026-10-25  
**Repositori:** `subhan-sd/npc-builder`  
**Baseline Cabang:** `v0.3.4.1`  
**HEAD Commit:** `2617e2b8faf11dca3fad056054361c886a2658cb`  
**Kesimpulan Singkat:** `MULTIPLE CAUSES` (Stale Binary GDExtension + Caching Mutasi Objek + Limitasi Representasi Visual)

---

## 1. Langkah Reproduksi Masalah

1. Jalankan Godot Engine (`godot --path godot` atau melalui host UI).
2. Di panel kontrol kiri:
   - Pilih **Gender:** `Female`.
   - Ubah slider **Chest Curve** dari `0.0` ke `4.0`.
   - Ubah slider **Torso Ratio** dari `0.0` ke `1.0`.
3. **Observasi:**
   - Siluet karakter di layar tengah (`TextureRect`) tidak mengalami perubahan bentuk tubuh, proporsi tinggi torso/kaki tidak bergeser, dan tidak muncul siluet feminin maupun kontur dada.

---

## 2. Pelacakan Alur Data (Trace Data Flow)

### 2.1 Alur Slider UI (`preview_ui.gd`)
1. User menggeser `chest_slider` atau `torso_ratio_slider`.
2. Sinyal `value_changed` memanggil `_update_params()` (`preview_ui.gd:128-146`).
3. Nilai disimpan ke dictionary lokal `current_profile`:
   ```gdscript
   a["torso_ratio"] = torso_ratio_slider.value
   current_profile["geometry_override"]["chest_curve"] = int(chest_slider.value)
   ```
4. `_update_params()` memanggil `_update_display()` (`preview_ui.gd:148-171`).
5. Memanggil jembatan GDScript:
   ```gdscript
   var img = NpcBridge.render_profile_image(current_profile, anim_state, direction, anim_time)
   ```

### 2.2 Alur Jembatan GDScript (`npc_bridge.gd`)
- `NpcBridge.render_profile_image()` (`npc_bridge.gd:76-84`) mendelegasikan ke objek native:
  ```gdscript
  return _generator.render_profile(profile, anim_state, direction, anim_time)
  ```

### 2.3 Alur Konversi GDExtension (`extension/src/npc_generator.cpp`)
1. `NPCGenerator::render_profile()` memanggil `dict_to_profile(profile)`:
   - Baris 123-131: Membaca `anatomy["torso_ratio"]`.
   - **CACAT 1:** Baris 213-226 membaca `geometry_override`, tetapi **TIDAK** membaca field `chest_curve` dari kamus `geometry_override`:
     ```cpp
     // Pada commit 2617e2b, field chest_curve TIDAK diekstrak:
     p.geometry_override.shoulder_w = ovr.get("shoulder_w", 0);
     ...
     p.geometry_override.arm_len = ovr.get("arm_len", 0);
     // chest_curve HILANG! p.geometry_override.chest_curve tetap 0.
     ```
   - **CACAT 2:** Baris 228-243 mengutamakan kamus `body_geometry` lama yang dicache dari `generate_profile()`:
     ```cpp
     if (dict.has("body_geometry")) {
         // Membaca nilai statis lama (torso_h, leg_len, dll.) yang dihitung saat start!
     } else {
         p.body_geometry = p.derived_geometry();
     }
     ```
     Karena `dict` sudah memiliki `body_geometry` sejak inisialisasi awal, `derived_geometry()` **TIDAK PERNAH DIPANGGIL ULANG** saat slider anatomi bergeser! Nilai `torso_ratio` yang baru diabaikan sepenuhnya oleh cache `body_geometry`.

### 2.4 Biner GDExtension Kadaluwarsa (Stale Binary)
- Berkas pustaka bersama aktif di `godot/bin/libnpc_builder_extension.so` memiliki timestamp:
  `2026-10-25 19:32:10` (dibuat pada commit `86a50ca`, yaitu era v0.3.3.1 sebelum v0.3.4 diimplementasikan).
- Biner tersebut dikompilasi sebelum penambahan `torso_ratio`, `arm_ratio`, maupun `chest_curve`.
- Godot memuat biner lama ini saat dijalankan, sehingga kode C++ Core terbaru belum dieksekusi sama sekali oleh runtime Godot.

---

## 3. Perbandingan Nilai Numerik Geometri (Min vs Default vs Max)

Evaluasi formula `compute_full_geometry(0.5f, 0.5f, 0.5f, 0.5f, torso_ratio, arm_ratio, ovr)`:

### A. Slider `Torso Ratio` (Tinggi Total = 39 px, `height = 0.5`)
| Nilai Slider | `torso_adj` | `raw_leg_len` | `leg_len` | `raw_torso_h` | `torso_h` | `arm_len` | Perubahan Region |
|---|---|---|---|---|---|---|---|
| `0.0` (Min: Torso Pendek / Kaki Panjang) | `+1` | `13 + 2 + 1 = 16` | **16** | `39 - 10 - 16 = 13` | **13** | **15** | Kaki memanjang 1px, torso memendek 1px |
| `0.5` (Default: Proporsi Kanonikal) | `0` | `13 + 2 + 0 = 15` | **15** | `39 - 10 - 15 = 14` | **14** | **16** | Baseline acuan (Bit-exact v0.3.3.1) |
| `1.0` (Max: Torso Panjang / Kaki Pendek) | `-1` | `13 + 2 - 1 = 14` | **14** | `39 - 10 - 14 = 15` | **15** | **17** | Kaki memendek 1px, torso memanjang 1px |

*Analisis:* Secara numerik di Core C++, variasi `torso_ratio` menggeser batas `torso_h` sebesar $\pm 1$ piksel dan `leg_len` sebesar $\mp 1$ piksel.

### B. Slider `Chest Curve` (Kontur Dada)
| Nilai Slider | `geo.chest_curve` | Baris Shading Torso (`y`) | Efek Visual di Render (`generator.cpp`) |
|---|---|---|---|
| `0` (Flat / Pria) | `0` | `torso_h * 0.38` (jika `musc > 0.45`) | Titik shadow pecs pria (2 titik gelap). |
| `1` (Small) | `1` | `y == 5` (`torso_h * 0.38`) | 2 highlight terang di tengah, 2 shadow gelap di samping. |
| `2` (Moderate / Female Default) | `2` | `y == 5` dan `y == 6` | Highlight dada atas + garis shadow bawah dada. |
| `4` (Max) | `4` | `y == 5` dan `y == 6` | Sama dengan level 2 (kontur 2 baris piksel). |

---

## 4. Akar Masalah (Root Causes)

Ditemukan **3 faktor penyebab** yang saling berkelindan:

### Akar Masalah 1: `extension/src/npc_generator.cpp` Mengabaikan Nilai Baru
1. `dict_to_profile()` mengekstrak `geometry_override`, tetapi lupa mengekstrak `chest_curve`:
   `p.geometry_override.chest_curve` selalu bernilai `0`.
2. `dict_to_profile()` mendahulukan pembacaan kamus cache `body_geometry` daripada menghitung ulang melalui `derived_geometry()`. Akibatnya, perubahan `height`, `torso_ratio`, dan `geometry_override` pada kamus profil Godot **terkunci pada cache lama**.

### Akar Masalah 2: Biner GDExtension Tidak Terkompilasi Ulang
- Berkas `godot/bin/libnpc_builder_extension.so` masih berasal dari rilis commit `86a50ca` (pukul 19:32).
- Pembangunan `build-test` hanya mengompilasi unit test C++ Core, bukan target GDExtension library Godot. Runtime Godot mengeksekusi biner lama yang belum mengenal simbol atau logika morfologi v0.3.4.

### Akar Masalah 3: Keterbatasan Representasi Visual Morfologi Wanita pada Kanvas Piksel
- Pada kanvas karakter 32 × 48 piksel (dengan tinggi torso 14 piksel dan lebar pinggang 7–8 piksel), anatomi wanita **tidak bisa hanya diwakili oleh 2 titik shading dada**.
- Bentuk siluet wanita memerlukan perbedaan kontur geometris menyeluruh:
  1. **Lebar Bahu (`shoulder_w`):** Wanita umumnya memiliki rasio bahu terhadap torso lebih ramping dibanding pria berotot.
  2. **Lengkungan Pinggang ke Pinggul (Hourglass Silhouette):** Pinggang (`waist_w`) lebih ramping dengan pinggul (`hips_w`) lebih lebar.
  3. **Tinggi Torso vs Kaki:** Rentang $\pm 1$ piksel saat ini terlalu halus sehingga mata manusia sulit membedakannya tanpa perubahan lebar panggul.

---

## 5. Rencana Perbaikan Minimal (Minimal Corrective Plan)

### Prioritas 1: Perbaiki Data Flow GDExtension (`extension/src/npc_generator.cpp`)
1. Tambahkan ekstraksi `chest_curve` di `dict_to_profile()`:
   ```cpp
   p.geometry_override.chest_curve = ovr.get("chest_curve", 0);
   ```
2. Pastikan `p.body_geometry` **selalu dihitung ulang secara kanonikal** dari parameter anatomi dan override:
   ```cpp
   p.body_geometry = p.derived_geometry();
   ```
   Menghapus pembacaan bypass dari kamus cache `dict["body_geometry"]`.

### Prioritas 2: Kompilasi Ulang Biner GDExtension ke `godot/bin/`
- Kompilasi ulang `npc_builder_extension` via CMake sehingga `godot/bin/libnpc_builder_extension.so` memuat kode v0.3.4.

### Prioritas 3: Otomatisasi Morfologi Wanita di UI Godot (`preview_ui.gd`)
- Ketika Gender `Female` dipilih, setel proporsi tubuh wanita yang jelas:
  - `chest_curve = 2` (kontur dada terlihat).
  - `hips_w = +1` atau `+2` (siluet panggul melebar).
  - `waist_w = -1` (lekukan pinggang feminin).
  - `shoulder_w = -1` (bahu lebih ramping dibanding pria).

---

## 6. Verifikasi yang Dibutuhkan

1. **Unit Test C++ Core:**
   - Memastikan `dict_to_profile()` memetakan `chest_curve` dan menghitung ulang `body_geometry` tanpa regresi.
2. **Headless Godot Smoke Test:**
   - Eksekusi `godot --headless --path godot --quit` memastikan ekstensi termuat tanpa galat simbol atau script parse error.
3. **Visual Verification:**
   - South view: Membandingkan perbedaan piksel Male vs Female pada seed 12345 (muncul lekukan pinggul dan highlight dada).
   - East/West view: Memverifikasi profil dada lateral menonjol 1 piksel di torso depan.

---

## 7. Kesimpulan

**`MULTIPLE CAUSES`**
1. **Bug Alur Data GDExtension:** `dict_to_profile()` tidak memetakan `chest_curve` dan memblokir rekalkulasi geometri karena prioritas pembacaan cache `body_geometry` lama.
2. **Biner Kadaluwarsa:** Pustaka `.so` di `godot/bin/` belum dikompilasi ulang ke versi v0.3.4.
3. **Limitasi Representasi:** Siluet wanita belum mengombinasikan pelebaran panggul dan pengecilan pinggang bersamaan dengan kontur dada.
