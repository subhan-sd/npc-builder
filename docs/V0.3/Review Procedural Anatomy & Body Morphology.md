# Laporan Audit dan Rencana Implementasi: Procedural Anatomy & Body Morphology

**Tanggal:** 2026-10-25  
**Dokumen Sumber BRD:** `docs/V0.3/V0.3.4/BRD-Procedural-Anatomy-Body-Morphology.md`  
**Target:** v0.3.4 (Procedural Anatomy & Body Morphology)  
**Status Evaluasi:** Audit Only (Tidak ada perubahan kode produksi pada fase ini)

---

## 1. Repository Baseline

### 1.1 Status Git
- **Branch Aktif:** `master`
- **HEAD Commit:** `363875fd5017331fdf5d3e5ba477251f3b70756d`
- **Working Tree State:** Perapian berkas dokumentasi (staged/modified dokumen BRD lama ke dalam direktori versi `docs/V0.1`, `docs/V0.2`, `docs/V0.3`, `docs/migrasi-gdextension`), tidak ada modifikasi kode C++/Godot.

### 1.2 Verifikasi Pengujian Awal (Release Build)
Eksekusi pengujian suite via CTest pada `build-test` menghasilkan:
```text
Test project /home/subhan/Project/npc-builder/build-test
    Start 1: CoreUnitTests
1/7 Test #1: CoreUnitTests ....................   Passed    0.01 sec
    Start 2: GoldenFixtureTest
2/7 Test #2: GoldenFixtureTest ................   Passed    0.18 sec
    Start 3: CApiParityTest
3/7 Test #3: CApiParityTest ...................   Passed    0.01 sec
    Start 4: SerializerVerificationTest
4/7 Test #4: SerializerVerificationTest .......   Passed    0.01 sec
    Start 5: CharacterProfileTest
5/7 Test #5: CharacterProfileTest .............   Passed    0.01 sec
    Start 6: BodyGeometryTest
6/7 Test #6: BodyGeometryTest .................   Passed    0.40 sec
    Start 7: TransitionTest
7/7 Test #7: TransitionTest ...................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 7 (Total time: 0.63s, Exit code: 0)
```
- **Golden Fixture Matrix:** 7.884 kombinasi hashing FNV-1a piksel identik (0 diff pada `tests/golden/matriks-acuan.txt`).

---

## 2. Requirement Matrix (FR-01 s/d FR-10 & NFR)

Klasifikasi:
- `EXISTS`: Sudah diimplementasikan dan diverifikasi dalam kode serta tes.
- `PARTIAL`: Sebagian fondasi ada, namun kriteria penerimaan BRD belum terpenuhi.
- `MISSING`: Belum ada implementasi sama sekali.
- `BLOCKED`: Bergantung pada keputusan arsitektur atau dependensi yang belum selesai.

| Pengenal | Kebutuhan | Status | Berkas & Simbol Relevan | Catatan Verifikasi |
|---|---|---|---|---|
| **FR-01** | Variasi tinggi tubuh | `PARTIAL` | `core/include/npc/body_geometry.hpp`<br>`compute_full_geometry()` | `height` memengaruhi `total_height` (36..42) dan `torso_h`, namun `torso_ratio` di `AnatomyProfile` belum dipakai dalam rumus (masih fixed formula). |
| **FR-02** | Variasi lebar tubuh | `EXISTS` | `core/include/npc/body_geometry.hpp`<br>`BodyGeometry`, `GeometryBounds` | Lebar bahu (`shoulder_w`), pinggang (`waist_w`), pinggul (`hips_w`), dan ketebalan paha/lengan dihitung independen dan memiliki batas kanonikal. |
| **FR-03** | Variasi massa dan muscularity | `PARTIAL` | `core/include/npc/body_geometry.hpp`<br>`generator.cpp:detail::render_internal` | Parameter `body_mass` dan `muscularity` independen dan menghasilkan V-taper / lateral swell, tetapi 4 preset (Kurus, Rata-rata, Berisi, Berotot) belum didefinisikan sebagai fungsi/preset eksplisit. |
| **FR-04** | Proporsi bagian tubuh | `PARTIAL` | `core/include/npc/profile.hpp`<br>`AnatomyProfile` | Field `torso_ratio`, `arm_ratio`, `head_scale` ada di `AnatomyProfile` (0.0..1.0), tetapi diabaikan oleh `compute_full_geometry()` (hanya `height`, `mass`, `musc`, `leg_ratio` yang dihitung). |
| **FR-05** | Anatomi wanita (dada & panggul) | `MISSING` | `core/src/generator.cpp:280-335`<br>`core/include/npc/profile.hpp` | Belum ada parameter `chest_volume` / `pelvis_width` / gender di `AnatomyProfile`. Renderer hanya memiliki pectorals pria (`musc > 0.45f`). |
| **FR-06** | Transisi proporsi terkontrol | `PARTIAL` | `core/include/npc/body_geometry.hpp`<br>`tests/test_transition.cpp` | Formula bersifat non-uniform dan deterministik. Uji transisi diskret ada di `test_transition.cpp`, tetapi belum mencakup parameter proporsi baru. |
| **FR-07** | Kompatibilitas body parts | `EXISTS` | `core/include/npc/body_geometry.hpp`<br>`compute_posed_placement()` | Hubungan hirarkis 10 body parts kanonikal (Head..RightFoot) terjaga dan terverifikasi 100% pada 19.008 kombinasi pose. |
| **FR-08** | Kompatibilitas pose, injury, dan amputasi | `EXISTS` | `core/src/generator.cpp:486-538`<br>`core/include/npc/profile.hpp` | Cedera pada limb amputasi dilewati; cascade amputation diverifikasi; bounding box injury sinkron dengan pose placement. |
| **FR-09** | Determinisme | `EXISTS` | `core/include/npc/generator.hpp`<br>`DeterministicRNG`, `tests/test_golden.cpp` | Determinisme input+seed diuji melalui 7.884 golden fixture di Linux, Windows, macOS. |
| **FR-10** | Validasi dan normalisasi | `EXISTS` | `core/include/npc/profile.hpp`<br>`ProfileValidator::validate/normalize` | Menjamin clamping [0.0, 1.0], rekalkulasi `body_geometry`, integritas cascade amputasi. |
| **NFR-01** | Determinisme lintas platform | `EXISTS` | `tests/golden/matriks-acuan.txt` | Hashing FNV-1a 64-bit lulus uji lintas platform. |
| **NFR-02** | Regression safety | `EXISTS` | `tests/test_golden.cpp` | Memblokir perubahan piksel visual yang tidak terjustifikasi. |
| **NFR-03** | Performa kalkulasi ringan | `EXISTS` | `compute_full_geometry()` | Kalkulasi berbasis aritmatika integer/clamping cepat O(1), di-cache di `body_geometry`. |
| **NFR-04** | Maintainability & Single source of truth | `PARTIAL` | `extension/src/npc_generator.cpp:204` | C++ Core adalah authoritative calculator, namun `dict_to_profile()` di extension masih memakai fallback legacy `compute_geometry(mass, musc)`. |
| **NFR-05** | Extensibility | `EXISTS` | `GeometryOverride`, `PosedPlacement` | Struktur modular siap menyambut variasi anatomi baru. |

---

## 3. Existing Architecture and Source-of-Truth Map

```
                ┌───────────────────────────────────┐
                │      AnatomyProfile (Input)       │
                │  - height, body_mass, muscularity │
                │  - posture, leg_ratio             │
                │  - [UNUSEDJ: torso/arm/head_scale │
                └─────────────────┬─────────────────┘
                                  │
                                  ▼
                ┌───────────────────────────────────┐
                │     GeometryOverride (Input)      │
                │   - 11 integer pixel offsets      │
                └─────────────────┬─────────────────┘
                                  │
                                  ▼
                     compute_full_geometry()
                  (Canonical Math - C++20 Core)
                                  │
                                  ▼
                ┌───────────────────────────────────┐
                │       BodyGeometry (Output)       │
                │  - 7 widths/thicknesses           │
                │  - 4 vertical heights/lengths     │
                │  (Output cache, NOT input data)   │
                └────────┬──────────────────┬───────┘
                         │                  │
                         ▼                  ▼
              compute_posed_placement()  render_internal()
              (Pose Bounding Boxes)      (Rasterizer & Overlays)
```

### Peta Sumber Kebenaran Saat Ini:
1. **Parameter Input Anatomi:** `CharacterProfile.anatomy` (`AnatomyProfile`).
2. **Kalkulator Kanonikal:** `compute_full_geometry()` di `core/include/npc/body_geometry.hpp`.
3. **Penyimpan Cache Hasil:** `CharacterProfile.body_geometry` (`BodyGeometry`). Ditandai secara eksplisit dalam kode sebagai output cache murni yang selalu ditimpa ulang oleh `ProfileValidator::normalize()`.
4. **Konsumen Geometri:**
   - `CharacterGenerator::render_profile()` di `core/src/generator.cpp`.
   - `compute_posed_placement()` di `core/include/npc/body_geometry.hpp`.
   - C API (`CNpcProfile` mengekspor `geo_*` sebagai read-only output).
   - GDExtension (`NPCGenerator::profile_to_dict` mengekspor kamus `body_geometry`).

---

## 4. Analisis Kesenjangan (Gap Analysis)

### Gap 1: Parameter Proporsi yang Belum Dimanfaatkan (`torso_ratio`, `arm_ratio`, `head_scale`)
- **Kondisi Saat Ini:** `AnatomyProfile` memiliki field `torso_ratio`, `arm_ratio`, dan `head_scale`. Namun di `compute_full_geometry()`:
  - `head_h` di-hardcode ke `8` px.
  - `torso_h` dihitung murni dari: `std::max(11, total_height - 8 - 2 - leg_len)`.
  - `arm_len` di-hardcode ke `torso_h + 2`.
- **Dampak Kesenjangan:** Karakter dengan tinggi yang sama belum bisa memvariasikan panjang torso vs panjang kaki secara mandiri melalui `torso_ratio`. Variasi rasio lengan (`arm_ratio`) tidak berdampak visual.
- **Perubahan Minimal:** Integrasikan `torso_ratio` dan `arm_ratio` ke dalam formula kanonikal `compute_full_geometry()` dengan tetap mempertahankan batas validasi `GeometryBounds`.

### Gap 2: Karakteristik Anatomi Wanita (FR-05)
- **Kondisi Saat Ini:**
  - `AnatomyProfile` tidak memiliki penanda gender, tipe anatomi, ukuran dada, maupun kontur panggul khusus wanita.
  - Di `generator.cpp`, kontur torso baris 288–302 hanya menghasilkan siluet maskulin (V-taper atau lurus) dengan aksen pectoral garis horizontal jika `musc > 0.45f`.
  - Hips (`hips_w`) didorong terutama oleh massa tubuh (`7 + round(mass * 3.5)`), bukan lebar panggul skeletal wanita.
- **Kesenjangan Kemampuan Render:**
  - Kanvas karakter adalah 32 × 48 piksel. Tinggi torso hanya 11–19 piksel, dan lebar dada 8–14 piksel.
  - Tidak ada shading/highlight khusus kontur payudara lateral atau pembesaran panggul bawah independen dari massa tubuh.
- **Risiko Kompatibilitas:** Menambah kontur dada/panggul akan mengubah matriks piksel visual (Golden Fixtures).

### Gap 3: Preset Morfologi Eksplisit (FR-03)
- **Kondisi Saat Ini:** Presets visual (Kurus, Rata-rata, Berisi, Berotot) hanya ada sebagai konsep dokumentasi / konfigurasi uji manual di `test_generator.cpp`. Belum ada helper kanonikal `AnatomyProfile::create_preset(MorphologyPreset)`.
- **Perubahan Minimal:** Tambahkan enum `MorphologyPreset` dan fungsi pembuat preset murni di `core/include/npc/profile.hpp` tanpa mengubah skema biner biner C API.

### Gap 4: Sinkronisasi GDExtension Dictionary Fallback
- **Kondisi Saat Ini:** Berkas `extension/src/npc_generator.cpp:204` memanggil `compute_geometry(p.anatomy.body_mass, p.anatomy.muscularity)` jika kamus Godot tidak memiliki entri `body_geometry`. Fungsi ini mengabaikan `height`, `leg_ratio`, dan `geometry_override`.
- **Perubahan Minimal:** Perbarui fallback ke `p.derived_geometry()`.

---

## 5. Risiko dan Kekhawatiran Kompatibilitas

| Aspek | Tingkat Risiko | Deskripsi Risiko | Strategi Mitigasi |
|---|---|---|---|
| **Golden Fixture Breakage** | **TINGGI** | Perubahan rumus proporsi torso/kaki atau penambahan kontur dada akan mengubah sidik digital (hash) 7.884 baris di `matriks-acuan.txt`. | Default parameter baru harus menghasilkan nilai default yang **100% identik secara biner** dengan karakter v0.3.3 saat ini (misal `torso_ratio = 0.5f` menghasilkan formula identik). |
| **Serialisasi Profil** | **SEDANG** | Menambahkan field baru ke `AnatomyProfile` dapat merusak format serialisasi `PROFILE_FORMAT_VERSION = 5`. | Gunakan format versi baru (`Format 6`) dengan jalur migrasi mundur untuk Format 2, 3, 4, 5, atau pertahankan `AnatomyProfile` tetap 8 float jika menggunakan parameter yang sudah ada. |
| **C API ABI Breakage** | **TINGGI** | Menambah field ke `CNpcProfile` mengubah ukuran struct. | C API v4 telah menerapkan guard `struct_size >= NPC_PROFILE_MIN_SIZE_V4`. Penambahan field baru harus diiringi kenaikan `NPC_API_VERSION = 5` atau diletakkan di akhir struct dengan guard offset. |
| **Integritas Pose & Amputasi** | **RENDAH** | Perubahan dimensi torso atau lengan berpotensi menggeser attach point anggota badan. | Gunakan fungsi derivasi terpadu `compute_posed_placement()` yang otomatis membaca dari `BodyGeometry`. |

---

## 6. Proposed Minimal Architecture

Prinsip: **Zero Schema Bloat**. Manfaatkan parameter yang sudah ada di `AnatomyProfile` sebelum menambah parameter baru.

```cpp
// 1. Manfaatkan field yang sudah ada di AnatomyProfile:
struct AnatomyProfile {
    float height{0.5f};       // Total height scale
    float body_mass{0.5f};    // Volume & thickness
    float muscularity{0.5f};  // Muscle tone, V-taper, shoulder width
    float posture{0.0f};      // Lean & head drop
    float head_scale{0.5f};   // Digunakan untuk head_h/head_w scaling
    float torso_ratio{0.5f};  // Distribusi torso vs leg height
    float arm_ratio{0.5f};    // Proporsi panjang lengan
    float leg_ratio{0.5f};    // Rasio segmen paha/betis
    
    // Opsi ekstensi masa depan (Tahap C):
    // float chest_volume{0.0f};
    // float pelvis_width{0.5f};
};
```

1. **Formula Torso-Leg Ratio Terpadu:**
   ```cpp
   // Menggunakan torso_ratio yang sudah ada:
   int raw_total_h = 36 + std::round(height * 6.0f);
   // torso_ratio 0.5f mempertahankan proporsi kanonikal v0.3.3.1 persis
   int available_h = raw_total_h - 8 - 2; // total minus head & neck
   int raw_leg_len = 13 + std::round(leg_ratio * 4.0f) + std::round((0.5f - torso_ratio) * 4.0f);
   int raw_torso_h = available_h - raw_leg_len;
   ```
2. **Kanonikal Presets Helper:**
   Fungsi statis murni yang mengembalikan `AnatomyProfile` untuk 4 preset dasar tanpa mengubah biner serializer:
   - `AnatomyProfile::preset_slim()`
   - `AnatomyProfile::preset_average()`
   - `AnatomyProfile::preset_heavy()`
   - `AnatomyProfile::preset_muscular()`

---

## 7. Implementation Milestones

### Milestone 1: Stabilisasi GDExtension & Kanonikal Presets Morfologi (Tanpa Ubah Visual)
- **Cakupan:**
  - Perbaiki fallback `dict_to_profile()` di `extension/src/npc_generator.cpp` agar memanggil `derived_geometry()`.
  - Tambahkan fungsi preset morfologi kanonikal (`preset_slim`, `preset_average`, dll.) di `core/include/npc/profile.hpp`.
- **Berkas Terdampak:** `extension/src/npc_generator.cpp`, `core/include/npc/profile.hpp`.
- **Pengujian:** Unit test preset di `tests/test_profile.cpp`.
- **Kriteria Penerimaan:** Golden fixtures 100% identik (0 diff), seluruh 7 tes CTest lulus.

### Milestone 2: Integrasi Proporsi Anatomi Dasar (`torso_ratio`, `arm_ratio`)
- **Cakupan:**
  - Aktifkan pengaruh `torso_ratio` dan `arm_ratio` pada `compute_full_geometry()`.
  - Pastikan nilai default `0.5f` menghasilkan geometri yang persis sama dengan v0.3.3.1 (zero-regression baseline).
  - Tambahkan batas di `GeometryBounds` jika diperlukan.
- **Berkas Terdampak:** `core/include/npc/body_geometry.hpp`.
- **Pengujian:** `tests/test_geometry.cpp`, `tests/test_transition.cpp`.
- **Kriteria Penerimaan:** Pada `torso_ratio = 0.5f`, tidak ada perubahan hash piksel. Pada variasi `0.0f` dan `1.0f`, siluet torso dan kaki berubah terukur tanpa memicu clipping/anomali.

### Milestone 3: Model Morfologi Wanita (Parameter & Shading Kontur Torso)
- **Cakupan:**
  - Tentukan integrasi data: evaluasi penambahan `chest_volume` dan `pelvis_width` vs representasi via `GeometryOverride`.
  - Implementasikan kontur shading dada dan panggul pada `generator.cpp:render_internal()`.
  - Pastikan pose, injury overlay, dan cascade amputation tetap presisi.
- **Berkas Terdampak:** `core/include/npc/profile.hpp`, `core/src/generator.cpp`, `core/include/npc/body_geometry.hpp`.
- **Pengujian:** Unit tests anatomi wanita, tes penempatan injury dada/panggul.
- **Kriteria Penerimaan:** Menghasilkan siluet panggul/dada terukur pada arah South dan East/West.

### Milestone 4: Kontrak Serializer, C API, dan Regenerasi Hasil Acuan (Golden Matrix)
- **Cakupan:**
  - Jika ada field baru pada `AnatomyProfile`, naikkan `PROFILE_FORMAT_VERSION` ke 6 dan sediakan migrasi mundur Format 5.
  - Naikkan `NPC_API_VERSION` jika struct C API diperluas.
  - Regenerasi matriks acuan `tests/golden/matriks-acuan.txt` secara terencana dengan justifikasi review formal.
- **Berkas Terdampak:** `core/include/npc/serializer.hpp`, `core/include/npc/c_api.h`, `core/src/c_api.cpp`, `tests/golden/matriks-acuan.txt`.
- **Pengujian:** `SerializerVerificationTest`, `CApiParityTest`, `GoldenFixtureTest`.
- **Kriteria Penerimaan:** Format lama dapat dibaca tanpa data hilang; seluruh platform lulus verifikasi hash.

---

## 8. Unresolved Decisions (Keputusan yang Perlu Dikunci)

1. **Strategi Parameter Anatomi Wanita:**
   - *Opsi A:* Tambahkan field eksplisit `gender` / `chest_volume` / `pelvis_width` di `AnatomyProfile`. Konsekuensi: Schema version naik, format serializer naik ke Format 6, C API struct membesar.
   - *Opsi B:* Manfaatkan `GeometryOverride` (misal override `hips_w` dan penambahan marker pakaian/dada) untuk variasi siluet wanita tanpa mengubah skema dasar `AnatomyProfile`.
   - *Rekomendasi:* Opsi A jika menginginkan procedural generation murni dari seed; Opsi B jika ingin menunda perubahan ABI hingga v0.4.
2. **Representasi Visual Dada pada Kanvas 32 × 48:**
   - Pada resolusi 32 × 48 dengan tinggi torso ~14 px, payudara wanita hanya dapat diwakili oleh 2–4 piksel shading (highlight atas, shadow bawah).
   - Apakah perluasan ini cukup berbasis pixel-art shading organik seperti pectoral pria saat ini, atau memerlukan modul pakaian/layer terpisah?
3. **Kebijakan Perubahan Golden Matrix:**
   - Apakah perubahan visual minor akibat proporsi torso baru diizinkan mengubah fixture acuan saat ini, atau harus dijaga agar default `0.5f` menghasilkan bit-exact golden match?
   - *Rekomendasi:* Default `0.5f` **harus bit-exact**, variasi hanya terjadi ketika parameter non-default disetel.

---

## 9. Rekomendasi Go / No-Go

- **Keputusan:** **GO dengan Kondisi Bertahap (Phased Go)**.
- **Rencana Tindakan:**
  1. Mulai dengan **Milestone 1** (perapian GDExtension fallback & morfologi presets murni di C++) tanpa mengubah struktur data publik maupun golden fixtures.
  2. Implementasikan **Milestone 2** dengan constraint zero-regression pada nilai default `0.5f`.
  3. Kunci keputusan arsitektur Anatomi Wanita (Unresolved Decision #1 & #2) sebelum memulai Milestone 3.
- **Larangan Ketat:** Jangan meregenerasi golden fixture sebelum Milestone 2 tervalidasi bit-exact pada baseline.
