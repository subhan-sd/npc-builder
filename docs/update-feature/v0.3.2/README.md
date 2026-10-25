# Laporan Pembaruan Fitur — v0.3.2 (Character Profile Validation, Serialization & CI Reliability)

**Tanggal:** 2026-10-25  
**Versi:** `v0.3.2`  
**Fitur Utama:** Pengerasan Validasi Profil Karakter, Integritas Serialisasi, Keandalan CI/CD & Godot Smoke Test  
**Cabang:** `v0.3.2-validation-serialization-ci-reliability`  
**Dokumen Kebutuhan Terkait:** `docs/V0.3/BRD_Character_Profile_Validation_Serialization_AND_CI_Reliability_V0.3.2.md`

---

## 1. Ringkasan Perubahan

Pembaruan versi `v0.3.2` berfokus pada keandalan sistem (*reliability*), pengerasan validasi (*validation hardening*), kepastian integritas serialisasi (*serialization round-trip integrity*), dan keandalan pipa integrasi berkelanjutan (*CI/CD pipeline*).

Sebelumnya, pengujian CI gagal mengonfigurasi CMake saat submodule `godot-cpp` belum diinisialisasi secara rekursif, validasi enum bentuk wajah dan aturan konsistensi amputasi belum ditegakkan penuh, fungsi perenderan C-API (`npc_profile_render`) belum menolak profil cacat sebelum proses render, dan pengujian Godot smoke test masih membiarkan binary usang lolos tanpa kegagalan eksplisit. Seluruh celah tersebut ditutup secara tuntas pada rilis ini dengan mempertahankan C++ Core sebagai **single source of truth**.

---

## 2. Rincian Pembaruan

### A. Keandalan CI & Konfigurasi Multi-Platform (`.github/workflows/ci.yml`, `CMakeLists.txt`)
- **Checkout Rekursif**: Menambahkan `submodules: recursive` pada tahap checkout workflow GitHub Actions untuk memastikan direktori `extension/godot-cpp` tersedia di seluruh platform runner.
- **Pengaman Konfigurasi CMake**: Menambahkan syarat ganda ketersediaan `extension/CMakeLists.txt` dan `extension/godot-cpp/CMakeLists.txt` sebelum memanggil `add_subdirectory(extension)`.
- **Target Multi-Konfigurasi**: Menetapkan properti `RUNTIME_OUTPUT_DIRECTORY_DEBUG` dan `RUNTIME_OUTPUT_DIRECTORY_RELEASE` agar binary GDExtension (`.dll`/`.so`) selalu diletakkan tepat di direktori `godot/bin/`.
- **Dukungan macOS**: Mendaftarkan binary `.dylib` pada berkas konfigurasi `godot/bin/npc_builder.gdextension`.
- **Godot Smoke Test Otomatis di CI**: Memasang runner resmi Godot Engine 4.4 Linux headless pada job Linux Release untuk mengeksekusi smoke test native secara otomatis.

### B. Pengerasan Validasi Profil Karakter (`ProfileValidator`)
- **Validasi Domain Bentuk Wajah**:
  - `hair_style`: [0, 3] (`0: Bald, 1: Short, 2: Medium, 3: Long`).
  - `eye_shape`: [0, 2].
  - `eye_spacing`: [1, 2].
  - `nose_shape`: [0, 1].
  - `mouth_shape`: [0, 1].
  - `expression`: [0, 5] (`Neutral` hingga `Surprised`).
- **Konsistensi Fisik & Amputasi**:
  - Kepala (`Head`) dan Badan (`Torso`) dilarang keras diamputasi.
  - Penegakan konsistensi kaskade amputasi: amputasi lengan atas mewajibkan amputasi telapak tangan; amputasi paha/kaki mewajibkan amputasi telapak kaki.
  - Larangan cedera pada anggota badan yang teramputasi: cedera yang ditempatkan pada bagian tubuh yang hilang otomatis ditolak sebagai profil tidak konsisten.
- **Normalisasi Adaptif**: Memperbaiki fungsi `normalize()` untuk mereparasi kaskade amputasi yang terputus, mengembalikan amputasi kepala/badan yang tidak sah, menghapus luka pada anggota tubuh yang hilang, serta menjepit (*clamp*) enum bentuk wajah.

### C. Konsistensi Validasi Pra-Render & C-API (`core/src/c_api.cpp`, `extension/src/npc_generator.cpp`)
- **Penolakan Profil Cacat Sebelum Render**: Fungsi `npc_profile_render()` dan `NPCGenerator::render_profile()` kini memvalidasi profil menggunakan `ProfileValidator::validate()` sebelum memanggil generator rendering.
- **Keamanan Penyangga (*Buffer Safety*)**: Jika profil tidak valid, fungsi C-API langsung mengembalikan kode galat `NPC_ERR_VALIDATION_FAIL` tanpa mengubah isi memori penyangga keluaran (*buffer*) dan tanpa menimbulkan *crash*.
- **Validasi Pointer & Argumen**: Menolak panggilan render dengan `injury_count > 0` saat pointer `injuries == nullptr` dengan kode galat `NPC_ERR_INVALID_ARGUMENT`.

### D. Integritas Serialisasi Round-Trip (`core/include/npc/serializer.hpp`)
- **Format V3 Lossless**: Mempertahankan seluruh metadata generasi, anatomi pecahan presisi tinggi, 5 bentuk wajah, ekspresi, 5 palet warna RGBA, 10 status amputasi, serta seluruh atribut luka (*condition_id*, *body_part*, *type*, *severity*, *visual_seed*, *active*, *healed*).
- **Validasi Aliran Deserialisasi**: Menolak string biner/teks serialisasi yang memuat nilai flag amputasi atau status luka di luar [0, 1], serta memvalidasi kembali batas domain bentuk wajah pada format migrasi V2.

### E. Pengetatan Godot Smoke Test (`godot/scripts/smoke_test_profile.gd`)
- Menghapus logika pengecualian (*bypass*) yang sebelumnya membiarkan binary versi lama lolos.
- Pengujian langsung gagal (`push_error` dan `quit(1)`) jika kelas native `NPCGenerator` atau metode `create_default_profile` tidak ditemukan.
- Menambahkan skenario uji penolakan profil cacat melalui Godot API, memastikan fungsi render native mengembalikan `null` saat menerima data tidak valid.

---

## 3. Matriks Dampak & Pengurangan Risiko

| Aspek | Kondisi Sebelum v0.3.2 | Kondisi Sesudah v0.3.2 | Dampak / Mitigasi |
| :--- | :--- | :--- | :--- |
| **Pipa CI/CD** | Gagal konfigurasi CMake akibat submodule kosong. | Submodule recursive aktif + uji smoke Godot otomatis. | CI memberikan bukti kualitas nyata di setiap commit dan pull request. |
| **Input C-API** | Profil invalid dapat lolos langsung ke fungsi perenderan. | Validasi ketat sebelum render; kode galat konsisten. | Mencegah kerusakan memori, *undefined behavior*, dan output rendering palsu. |
| **Konsistensi Anatomi** | Kepala/badan bisa diamputasi; luka bisa ada di kaki buntung. | Aturan konsistensi anatomi ditegakkan oleh validator. | Menghilangkan anomali render fisik yang mustahil secara biologis/logis. |
| **Smoke Test Godot** | Mengabaikan ketiadaan metode baru jika binary lama terpasang. | Wajib memakai binary baru; gagal jika terjadi regresi. | Mencegah klaim sukses palsu (*false positive*) pada pengujian integrasi engine. |

---

## 4. Hasil Verifikasi Pengujian

Seluruh rangkaian pengujian lokal dan headless engine terverifikasi **100% Lulus**:
- `CoreUnitTests`: **Passed** (RNG, siluet anatomi, dan deformasi postur).
- `GoldenFixtureTest`: **Passed** (7.360 matriks kombinasi hash piksel identik tanpa regresi visual).
- `CApiParityTest`: **Passed** (100% kesetaraan byte RGBA C-API vs C++ Core pada 100 seed).
- `SerializerVerificationTest`: **Passed** (Round-trip serialisasi Format V2 & penolakan data korup).
- `CharacterProfileTest`: **Passed** (Validasi enum wajah, konsistensi amputasi & cedera, serialisasi V3 round-trip 100 seed, migrasi V2, C-API profile render, serta penolakan profil cacat pra-render).
- `Godot Engine Smoke Test`: **Passed** (Instansiasi native GDExtension, validasi profil, serialisasi, rendering 32x48, dan penolakan profil tidak valid).
