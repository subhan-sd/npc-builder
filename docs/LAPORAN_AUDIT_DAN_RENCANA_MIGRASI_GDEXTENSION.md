# Laporan Audit Repositori dan Rencana Implementasi Migrasi GDExtension

Tanggal: 2026-10-24 · Dokumen Sumber: `docs/BRD_NPC_Builder_GDExtension_dilanjutkan.md` · Basis Commit: `b929007`

## 1. Ringkasan Eksekutif

Audit dilakukan untuk memvalidasi kondisi repositori `npc-builder` terhadap fakta-fakta pada dokumen kebutuhan bisnis `BRD_NPC_Builder_GDExtension_dilanjutkan.md`. Seluruh fakta kondisi awal terbukti valid:
- Skrip pratinjau Godot saat ini menjalankan implementasi generator dan rasterizer duplikat 343 baris di `godot/scripts/npc_bridge.gd`.
- Jembatan C (`core/src/c_api.cpp`) kehilangan data (warna mata, bentuk wajah) dan menghasilkan piksel berbeda dibanding Inti C++.
- Serializer (`core/include/npc/serializer.hpp`) kehilangan warna mata saat deserialisasi dan tidak memvalidasi rentang/format.
- Target tes saat ini menggunakan `assert` standar yang hilang saat build Release (`-DNDEBUG`).
- Versi editor Godot yang aktif di host Windows adalah Godot 4.7.2 (`Godot_v4.7.2-stable_win64.exe`).

## 2. Tabel Pelacakan Kebutuhan

| Pengenal | Kategori / Judul | Status Awal | Catatan Verifikasi |
|---|---|---|---|
| Fungsional-001 | Pengujian terdaftar (CTest) | Sedang dikerjakan | `CMakeLists.txt` belum memiliki `enable_testing()` atau `add_test()`. |
| Fungsional-002 | Pemeriksaan tahan Release | Sedang dikerjakan | Biner tes saat ini memakai `cassert` yang hilang saat `NDEBUG`. |
| Fungsional-003 | Pesan gagal informatif | Sedang dikerjakan | Membutuhkan helper pengujian assert mandiri yang mencetak nilai. |
| Fungsional-004 | Pembuat hasil acuan (Golden) | Belum diperiksa | Matriks acuan 7.360 kombinasi perlu diintegrasikan ke target build. |
| Fungsional-005 | Tes hasil acuan | Belum diperiksa | Pengujian sidik digital FNV-1a 64-bit atas penyangga RGBA. |
| Fungsional-006 | Pembaruan acuan yang disengaja | Belum diperiksa | Prosedur dan dokumentasi pembaruan berkas acuan. |
| Fungsional-007 | Pembangunan lintas kompiler & visibilitas | Sedang dikerjakan | `CMAKE_CXX_VISIBILITY_PRESET hidden` dan peringatan MSVC `/W4`. |
| Fungsional-008 | Berkas yang diabaikan (.gitignore) | Belum diperiksa | Menambahkan `*.dll`, `*.dylib`, `*.exe`, `build-*/`. |
| Fungsional-009 | Integrasi berkelanjutan (CI) | Belum diperiksa | GitHub Actions workflow Linux, Windows, macOS. |
| Fungsional-010 | Lencana status nyata | Belum diperiksa | Menghubungkan README badge ke alur CI GitHub Actions. |
| Fungsional-011 | Kesetaraan jalur Inti dan Jembatan C | Belum diperiksa | Menyelaraskan field penampilan dan parameter render di C API. |
| Fungsional-012 | Kontrak struktur ber-versi | Belum diperiksa | Menambahkan `api_version` dan `struct_size` pada `CNpcAppearance`. |
| Fungsional-013 | Pelengkapan data penampilan C API | Belum diperiksa | Menambahkan warna mata, bentuk mata/hidung/mulut pada struct C. |
| Fungsional-014 | Parameter render lengkap C API | Belum diperiksa | Mendukung cedera (physical conditions) selain amputasi. |
| Fungsional-015 | Validasi enum dan angka C API | Belum diperiksa | Penolakan nilai di luar batas tanpa crash. |
| Fungsional-016 | Status galat yang konsisten C API | Belum diperiksa | Kode galat terdokumentasi (0 = sukses, negatif = kode galat). |
| Fungsional-017 | Kepemilikan memori eksplisit | Belum diperiksa | Penyangga keluaran dialokasikan pemanggil. |
| Fungsional-018 | Ukuran struktur yang aman | Belum diperiksa | Validasi `struct_size` di API C. |
| Fungsional-019 | API generator dan renderer terpisah | Belum diperiksa | Pembangkitan penampilan dan render piksel terpisah. |
| Fungsional-020 | Kompatibilitas simbol publik | Belum diperiksa | Hanya simbol `npc_*` yang diekspor dari pustaka dinamis. |
| Fungsional-021 | Proyek GDExtension terpisah | Belum diperiksa | Folder `extension/` terisolasi dari core. |
| Fungsional-022 | Integrasi godot-cpp yang dipatok | Belum diperiksa | Menggunakan binding kompatibel dengan Godot 4.7.2. |
| Fungsional-023 | Berkas registrasi `.gdextension` | Belum diperiksa | Konfigurasi manifest `npc_builder.gdextension`. |
| Fungsional-024 | Registrasi kelas native | Belum diperiksa | Registrasi `NPCGenerator` ke `ClassDB`. |
| Fungsional-025 | API pembangkitan penampilan native | Belum diperiksa | Return `Dictionary` penampilan lengkap dari seed. |
| Fungsional-026 | API render RGBA native | Belum diperiksa | Render piksel deterministik sesuai C++ core. |
| Fungsional-027 | Konversi ke Image/ImageTexture | Belum diperiksa | Menghasilkan `godot::Image` format RGBA8. |
| Fungsional-028 | Dukungan semua parameter render native | Belum diperiksa | 6 animasi, 4 arah, amputasi 10 bagian tubuh, 6 jenis cedera. |
| Fungsional-029 | Validasi argumen Godot | Belum diperiksa | Penolakan tipe atau nilai salah dari GDScript. |
| Fungsional-030 | Galat pemuatan yang dapat ditindaklanjuti | Belum diperiksa | Pesan diagnosis saat biner native tidak ditemukan. |
| Fungsional-031 | Ekspor platform yang dideklarasikan | Belum diperiksa | Platform aktual Linux dan Windows x86_64. |
| Fungsional-032 | Satu sumber kebenaran | Belum diperiksa | GDScript duplikat dipangkas setelah GDExtension aktif. |
| Fungsional-033 | Inisialisasi dan pelepasan sumber daya | Belum diperiksa | Siklus hidup GDExtension module initialization level scene. |
| Fungsional-034 | API native yang sederhana | Belum diperiksa | Godot script berinteraksi dengan API tingkat domain. |
| Fungsional-035 | Pemisahan fungsi render | Belum diperiksa | Refaktor internal `render()` tanpa mengubah keluaran visual. |
| Fungsional-036 | Kesetaraan serializer | Belum diperiksa | Penyimpanan dan pemulihan warna mata dan seluruh field. |
| Fungsional-037 | Validasi deserialisasi | Belum diperiksa | Penolakan data korup, teks terpotong, atau versi tidak sah. |
| Fungsional-038 | Versi format serialisasi | Belum diperiksa | Penanda versi format eksplisit pada data serialisasi. |
| Fungsional-039 | Stabilitas benih | Belum diperiksa | Determinisme output antargenerasi. |
| Fungsional-040 | Kebijakan perubahan visual | Belum diperiksa | Perubahan visual wajib tercatat di tabel log perubahan. |
| Fungsional-041 | Semantik PixelBuffer | Belum diperiksa | Batas dimensi 32x48 dan saluran warna terisolasi. |
| Fungsional-042 | Uji kondisi tubuh | Belum diperiksa | Amputasi dan cedera tidak merusak memori di luar kanvas. |
| Fungsional-043 | Dokumentasi generator version | Belum diperiksa | Penjelasan kebijakan versi generator pada dokumentasi. |
| Nonfungsional-001 s.d. 012 | Kebutuhan Kualitas & Batas | Belum diperiksa | Determinisme, isolasi dependensi, performa, dan keamanan memori. |

## 3. Rencana Perubahan Berkas dan Urutan Implementasi

1. **Tahap 1 — Fondasi Pembangunan dan Pengujian**:
   - `CMakeLists.txt`: Aktifkan `enable_testing()`, flags peringatan lintas compiler, `CMAKE_VISIBILITY_INLINES_HIDDEN`, `POSITION_INDEPENDENT_CODE`.
   - `tests/test_framework.hpp`: Framework uji ringan tanpa dependensi eksternal (macro `TEST_ASSERT`, `TEST_ASSERT_EQ`) yang tetap aktif pada Release `-DNDEBUG`.
   - `tests/test_generator.cpp`: Migrasikan semua `assert()` ke helper pengujian baru.
   - `tests/generate_fixtures.cpp`: Pembuat hasil acuan 7.360 kombinasi (FNV-1a 64-bit RGBA hash) disimpan di `tests/golden/matriks-acuan.txt`.
   - `tests/test_golden.cpp`: Verifikasi pengujian hasil acuan di CTest.
   - `.gitignore`: Tambahkan artefak kompilasi lintas OS (`*.dll`, `*.dylib`, `*.exe`, `*.pdb`, `build-*/`).
   - `.github/workflows/ci.yml`: Alur kerja otomatisasi build & CTest (Ubuntu GCC/Clang, Windows MSVC, macOS AppleClang).
   - Gerbang Verifikasi: Sengaja buat satu nilai harapan gagal di build Release, pastikan CTest gagal, lalu kembalikan.

2. **Tahap 2 — Penyempurnaan Jembatan C dan Serializer**:
   - `core/include/npc/c_api.h` & `core/src/c_api.cpp`: Sempurnakan struct `CNpcAppearance` (tambahkan warna mata, parameter wajah, `api_version`, `struct_size`). Tambahkan validasi batas, kode galat, dan parameter cedera.
   - `core/include/npc/serializer.hpp`: Simpan `eye_color`, validasi rentang nilai saat deserialisasi, validasi versi format.
   - `tests/test_c_api.cpp` & `tests/test_serializer.cpp`: Daftarkan ke CTest. Uji kesetaraan 100% byte RGBA antara C API dan Inti C++.

3. **Tahap 3 — Implementasi GDExtension**:
   - `extension/CMakeLists.txt` & submodul / vendor `godot-cpp`: Konfigurasi target pustaka dinamis `npc_builder_gdextension`.
   - `extension/src/npc_generator.hpp` & `extension/src/npc_generator.cpp`: Adapter native `NPCGenerator` mengekspos `generate_appearance()` dan `render_image()`.
   - `godot/bin/npc_builder.gdextension`: Konfigurasi manifest target library Linux dan Windows.
   - Verifikasi pemuatan biner GDExtension di Godot 4.7.2.

4. **Tahap 4 — Migrasi Pratinjau Godot**:
   - Hubungkan `godot/scripts/preview_ui.gd` ke kelas `NPCGenerator` dari GDExtension.
   - Pangkas algoritma rasterizer dan generator duplikat di `godot/scripts/npc_bridge.gd`.
   - Verifikasi kesetaraan visual 1:1.

5. **Tahap 5 — Perapian Dokumentasi & Penerimaan Akhir**:
   - Perbarui `README.md` dengan status build CI nyata dan panduan GDExtension.
   - Dokumentasikan hasil pengukuran performa render.
