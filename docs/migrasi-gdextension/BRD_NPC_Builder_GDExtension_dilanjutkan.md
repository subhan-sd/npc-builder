# Dokumen Kebutuhan Bisnis: Integrasi Pembuat Karakter dengan Godot melalui GDExtension dan Perbaikan Pendukung

Proyek: Living World · Modul: Pembuat Karakter (repositori `npc-builder`) · Versi dokumen: 1 · Tanggal: 2026-10-24 · Status: Rancangan untuk ditinjau

Dasar analisis: kode pada commit `365badd` (versi v0.2.1, cabang `master`). Penulisan mengikuti “Konvensi Penulisan Dokumen Kebutuhan Bisnis”. Fakta yang berasal dari pengukuran penyusun ditandai “pengukuran 2026-10-24”, dan cara mengulanginya ada di Lampiran A.

## 1. Ringkasan

Pratinjau Godot tidak memakai Inti C++. Pratinjau menjalankan salinan pembangkitan penampilan dan penggambaran yang ditulis ulang dalam GDScript (343 baris). Dua implementasi itu sudah berbeda: benih acak yang sama menghasilkan karakter berbeda, dan Jembatan C menghasilkan gambar yang berbeda dari Inti pada 100 dari 100 benih uji.

Dokumen ini menetapkan kebutuhan agar Godot memakai Inti C++ melalui GDExtension sebagai satu-satunya implementasi penggambaran. Pengamannya adalah pengujian hasil acuan, yaitu sidik digital (hash) penyangga piksel yang dijalankan di tiga sistem operasi. Pekerjaan dibagi menjadi empat tahap: fondasi pembangunan dan pengujian, penyempurnaan Jembatan C, Perluasan Godot, dan perapian Inti.

Hasil yang diharapkan: satu sumber kebenaran, determinisme yang terbukti, dan fondasi yang siap untuk pekerjaan berikutnya seperti genom, simulasi, dan pakaian.

## 2. Latar belakang

Visi Living World menetapkan inti simulasi C++20 sebagai sumber kebenaran, dengan Godot 4 sebagai penampil melalui GDExtension. Pembuat Karakter dibuat agar dunia berisi ratusan sampai ribuan karakter dengan variasi tubuh tanpa menggambar aset satu per satu.

Dokumen kebutuhan versi 0.1 (`docs/BRD_v0.1.md`) sudah menetapkan integrasi “GDExtension / C-ABI Shared Library” (baris 10) dan pemisahan simulasi dari penggambaran (baris 40). Dokumen ini karena itu memenuhi kebutuhan lama yang belum terpenuhi, bukan menambah kebutuhan baru.

Pekerjaan dilakukan sekarang karena versi 0.2.1 sudah menyelesaikan fondasi anatomi. Selama renderer GDScript dipertahankan, setiap fitur berikutnya (pakaian, variasi gender, genom) harus ditulis dua kali. Dokumen audit versi 0.2.1 (`docs/NPC_BUILDER_V021_AUDIT.md`, Bagian 3) bahkan menetapkan bahwa setiap perubahan pada Inti dicerminkan secara manual ke GDScript.

Dokumen lama berawalan `BRD_` ditulis dalam bahasa Inggris dan memakai singkatan. Dokumen-dokumen itu tidak diubah.

## 3. Tujuan dan bukan-tujuan

### 3.1 Tujuan

1. Godot memakai Inti C++ sebagai satu-satunya implementasi penggambaran dan pembangkitan penampilan.
2. Gambar yang dihasilkan Godot identik dengan gambar yang dihasilkan jalur uji C++, dibuktikan dengan sidik digital.
3. Determinisme terbukti identik pada Linux, Windows, dan macOS.
4. Pengujian tetap memeriksa pada build Release dan berjalan otomatis pada setiap perubahan.
5. Antarmuka Jembatan C lengkap, tervalidasi, dan bersetempel versi.
6. Inti dan serializer siap diperluas tanpa merusak data atau gambar yang sudah ada.

### 3.2 Bukan-tujuan

1. Pemisahan genom dari penampilan, yaitu penghitungan penampilan keturunan dari gen orang tua. Dikerjakan dalam dokumen lanjutan.
2. Cache sprite dan atlas tekstur untuk ribuan karakter. Fakta-014 menunjukkan penggambaran satu karakter sangat cepat, sehingga kebutuhan ini perlu diukur ulang setelah integrasi selesai.
3. Irisan vertikal berisi puluhan karakter yang bergerak di grid Godot.
4. Pakaian, variasi gender, dan ras fantasi.
5. Perbaikan visual: pencampuran alfa pada `PixelBuffer::set_pixel`, penempatan cedera pada tangan dan kaki yang kini jatuh ke area torso, dan polesan anatomi.
6. Inti simulasi Living World (grid, kebutuhan, genetika, keluarga, kelompok, permukiman). Dokumen ini tidak menyentuhnya.

## 4. Ruang lingkup

### 4.1 Termasuk

- Konfigurasi pembangunan (`CMakeLists.txt`, `.gitignore`) dan alur kerja integrasi berkelanjutan.
- Pengujian: pengganti `assert`, berkas hasil acuan, dan tes hasil acuan.
- Jembatan C: kelengkapan data, validasi, dan nomor versi.
- Perluasan Godot baru berbasis godot-cpp, beserta berkas `.gdextension`.
- Skrip Godot: penghapusan renderer GDScript dan penyesuaian skrip pratinjau.
- Inti: pemecahan fungsi `render()` dan pembaruan serializer.
- Dokumentasi: `README.md` dan Daftar perubahan visual dalam dokumen ini.

### 4.2 Tidak termasuk

Semua butir pada Bagian 3.2, dan perubahan apa pun pada dokumen lama berawalan `BRD_`.

## 5. Kondisi saat ini

Semua fakta berikut diverifikasi pada commit `365badd`. Fakta yang berasal dari program uji sementara penyusun ditandai dan dijelaskan di Lampiran A.

| Pengenal | Fakta | Sumber |
| --- | --- | --- |
| Fakta-001 | Skrip pratinjau hanya memanggil dua fungsi `NpcBridge` (didaftarkan sebagai autoload): `generate_appearance(seed_val, npc_id)` dan `render_texture(app, amputations, anim_state, direction, anim_time)`. | `godot/project.godot` bagian autoload; `godot/scripts/preview_ui.gd`; `godot/scripts/npc_bridge.gd` baris 40 dan 64 |
| Fakta-002 | `npc_bridge.gd` (343 baris) menulis ulang pembangkitan penampilan dan seluruh penggambaran, dan tidak memanggil pustaka C++ apa pun. Pencarian kata `gdextension`, `load(`, `npc_render`, `npc_generate`, dan `ClassDB` pada folder `godot/` tidak menemukan hasil. | `godot/scripts/npc_bridge.gd`; pencarian pada folder `godot/` |
| Fakta-003 | Folder `godot/` hanya berisi `project.godot`, `scenes/preview.tscn`, dan empat berkas skrip (dua `.gd`, dua `.uid`). Tidak ada berkas `.gdextension`. | Daftar berkas repositori |
| Fakta-004 | README menyatakan Inti C++ “authoritative” dan jembatan GDScript “mensinkronkan logika 1:1”. Dokumen audit menyatakan setiap peningkatan rasterizer C++ dicerminkan manual ke GDScript. | `README.md` baris 59 sampai 61; `docs/NPC_BUILDER_V021_AUDIT.md` Bagian 3 |
| Fakta-005 | Pembangkitan penampilan berbeda. GDScript memakai `RandomNumberGenerator` bawaan Godot dan warna kulit, rambut, mata, baju, dan celana yang tetap. Inti memakai pembangkit SplitMix64 dan memilih warna dari palet. Benih yang sama karena itu menghasilkan karakter yang berbeda. | fungsi `generate_appearance` di `npc_bridge.gd`; fungsi `generate_from_seed` di `core/src/generator.cpp`; `core/include/npc/rng.hpp` |
| Fakta-006 | Struktur `CNpcAppearance` pada Jembatan C tidak memuat warna mata, bentuk mata, jarak mata, bentuk hidung, dan bentuk mulut. Fungsi render hanya menerima topeng amputasi, bukan cedera. Fungsi itu mengunci `generator_version` pada 1 (pembangkit menulis 2) dan kecepatan animasi pada 1,0, serta tidak memvalidasi `anim_state`, `direction`, dan `expression`. Fungsi `npc_generate_appearance` mengembalikan struktur sebagai nilai. | `core/src/c_api.cpp` |
| Fakta-007 | Untuk benih 1 sampai 100 (Idle, South, waktu 0,0), gambar lewat Jembatan C berbeda dari gambar langsung Inti pada 100 dari 100 benih. | Pengukuran 2026-10-24 |
| Fakta-008 | Serializer tidak menyimpan warna mata: 100 dari 100 benih kehilangan warna mata setelah serialisasi dan deserialisasi. Deserialisasi menerima nilai di luar rentang tanpa galat (contoh: tinggi 9, ekspresi 99, dan warna 999 yang terpotong menjadi 231) dan tidak memeriksa nomor versi. Tes serialisasi hanya membandingkan pengenal, benih, dan empat nilai tubuh. | `core/include/npc/serializer.hpp`; `tests/test_generator.cpp` fungsi `test_serialization`; pengukuran 2026-10-24 |
| Fakta-009 | Build Release memakai `-O3 -DNDEBUG`. Biner tes Release tidak memuat satu pun rujukan `__assert_fail`, sehingga seluruh `assert` hilang dan program tetap melaporkan semua tes lulus. | `flags.make` hasil CMake; `nm` pada biner; pengukuran 2026-10-24 |
| Fakta-010 | `CMakeLists.txt` tidak memanggil `enable_testing()` atau `add_test()`. `tests/generate_fixtures.cpp` bukan target pembangunan. Opsi peringatan hanya dipasang pada `npc_core` dan bergaya g++ dan clang. Tidak ada pengaturan visibilitas simbol. | `CMakeLists.txt` |
| Fakta-011 | Di Linux (g++, CMake 4.4.4, Release) proyek berhasil dibangun apa adanya: `npc_core`, `npc_builder`, dan `npc_test` terbentuk. Pembangunan di Windows dan macOS belum diverifikasi. | Pengukuran 2026-10-24 |
| Fakta-012 | Pustaka bersama `npc_builder` mengekspor dua fungsi C (`npc_generate_appearance`, `npc_render_pixels`) dan dua simbol C++ (`CharacterGenerator::generate_from_seed`, `CharacterGenerator::render`). | `nm -D` pada `libnpc_builder.so`; pengukuran 2026-10-24 |
| Fakta-013 | Delapan tes lulus bila dikompilasi tanpa `NDEBUG` memakai `g++ -std=c++20 -Wall -Wextra -Wpedantic`, tanpa peringatan. | Pengukuran 2026-10-24 |
| Fakta-014 | Waktu satu penggambaran (kompiler `-O2`, tanpa cedera, 20.000 penggambaran, satu inti prosesor Intel Xeon 2,1 gigahertz): rata-rata 1,28 mikrodetik, median 1,24 mikrodetik, persentil ke-99 2,36 mikrodetik. Angka ini hanya indikasi, bukan angka dasar mesin pengembangan. | Pengukuran 2026-10-24 |
| Fakta-015 | `preview_ui.gd` memanggil `render_texture` pada setiap bingkai (fungsi `_process`), dan setiap panggilan mengembalikan nilai bertipe `ImageTexture`. | `godot/scripts/preview_ui.gd`; `npc_bridge.gd` baris 64 |
| Fakta-016 | Pratinjau hanya menyediakan dua dari sepuluh bagian tubuh untuk diamputasi (lengan kiri dan kaki kiri) dan tidak menyediakan cedera. | `godot/scripts/preview_ui.gd` |
| Fakta-017 | Konfigurasi proyek Godot menyebut versi fitur “4.7”. Versi godot-cpp belum ada di repositori. | `godot/project.godot` |
| Fakta-018 | README memuat lencana statis “Tests 8/8 Passing” yang tidak terhubung ke hasil tes. | `README.md` bagian atas |

## 6. Arsitektur sasaran

### 6.1 Lapisan dan tanggung jawab

| Lapisan | Lokasi | Tanggung jawab | Tidak boleh |
| --- | --- | --- | --- |
| Inti Pembuat Karakter | `core/` | Membangkitkan penampilan dari benih, menggambar penyangga piksel, menyerialisasi penampilan. Pemilik seluruh rumus. | Bergantung pada Godot atau pustaka grafis apa pun |
| Jembatan C | `core/src/c_api.cpp` dan header publik baru | Antarmuka C yang stabil untuk konsumen selain Godot dan untuk pengujian lintas bahasa | Memuat logika penggambaran |
| Perluasan Godot | Folder baru (usulan `extension/`) | Mengubah tipe Godot (Dictionary, Array, ImageTexture) menjadi tipe Inti dan sebaliknya, serta mendaftarkan kelas | Memuat rumus penggambaran |
| Skrip Godot | `godot/scripts/` | Antarmuka pratinjau: kontrol, tampilan, waktu animasi | Memuat rumus penggambaran atau pembangkitan penampilan |

### 6.2 Arah data

Diagram berikut menunjukkan siapa memanggil siapa. Panah menunjukkan arah panggilan.

```
Skrip Godot (Dictionary, Array, bilangan)
      │
      ▼
Perluasan Godot (godot-cpp) ─────────────► Inti Pembuat Karakter (core/)
                                                     ▲
Konsumen lain dan tes lintas bahasa ► Jembatan C ────┘
```

### 6.3 Prinsip

1. Ada satu implementasi penggambaran, yaitu di Inti.
2. Data lintas lapisan dikirim sebagai salinan nilai. Satu-satunya memori yang dimiliki lapisan lain adalah penyangga keluaran milik pemanggil.
3. Setiap antarmuka lintas lapisan membawa nomor versi.
4. Setiap lapisan diuji terhadap berkas hasil acuan yang sama.

### 6.4 Keputusan yang diusulkan

Perluasan Godot memanggil Inti langsung (C++ ke C++ melalui godot-cpp). Alasannya: tidak ada penyalinan struktur, seluruh field termasuk cedera tersedia, dan penyimpangan seperti Fakta-006 tidak terulang.

Jembatan C tetap dipertahankan karena sudah menjadi target `npc_builder` dan berguna bagi konsumen selain Godot. Keduanya diuji terhadap hasil acuan yang sama (Fungsional-011), sehingga tidak ada penyimpangan yang lolos tanpa terdeteksi.

Alternatifnya adalah Perluasan Godot yang hanya memakai Jembatan C. Jalurnya tunggal, tetapi struktur C harus selalu menyamai Inti. Keputusan resmi dicatat di Terbuka-001.

## 7. Kebutuhan fungsional

Kata kewajiban mengikuti konvensi: “harus” berarti wajib, “sebaiknya” berarti diinginkan, “boleh” berarti opsional. Setiap tabel adalah satu tahap.

### 7.1 Tahap 1: Fondasi pembangunan dan pengujian

| Pengenal dan judul | Pernyataan | Alasan | Cara verifikasi | Bergantung pada |
| --- | --- | --- | --- | --- |
| Fungsional-001 Pengujian terdaftar | `CMakeLists.txt` harus memanggil `enable_testing()` dan mendaftarkan setiap program tes dengan `add_test()`. | Saat ini tidak ada satu perintah untuk menjalankan semua tes (Fakta-010). | Perintah `ctest` menjalankan seluruh tes. Tes yang gagal membuat `ctest` berakhir dengan kode keluar bukan nol. | Tidak ada |
| Fungsional-002 Pemeriksaan tahan Release | Setiap pemeriksaan dalam tes harus tetap berjalan pada konfigurasi Debug dan Release. | Pada Release, `assert` hilang dan tes melaporkan lulus tanpa memeriksa (Fakta-009). | Ubah satu nilai harapan menjadi salah, bangun Release, jalankan `ctest`: hasilnya harus gagal. | Fungsional-001 |
| Fungsional-003 Pesan gagal informatif | Tes sebaiknya memakai kerangka pengujian yang menampilkan nilai harapan dan nilai aktual saat gagal. | `assert` hanya menampilkan ekspresi, bukan nilainya. | Satu tes yang sengaja gagal menampilkan kedua nilai di keluaran. | Fungsional-002 |
| Fungsional-004 Pembuat hasil acuan | Harus ada program, memakai target `generate_fixtures` yang sudah ada atau target baru, yang menghitung sidik digital penyangga piksel untuk matriks acuan (Bagian 7.1.1) dan menuliskannya ke `tests/golden/matriks-acuan.txt`, satu baris per kombinasi. | Hasil acuan menjadi pengaman semua perubahan berikutnya. | Dijalankan dua kali, berkas yang dihasilkan identik karakter demi karakter. | Fungsional-001 |
| Fungsional-005 Tes hasil acuan | Tes harus menghitung ulang sidik digital seluruh kombinasi matriks acuan dan gagal bila ada yang berbeda dari berkas acuan, dengan menyebut kombinasi pertama yang berbeda (benih, keadaan animasi, arah, waktu animasi). | Perubahan piksel sekecil apa pun harus terdeteksi. | Ubah satu warna konstanta di `generator.cpp`: tes gagal dan menyebut kombinasi yang berbeda. | Fungsional-004 |
| Fungsional-006 Pembaruan acuan yang disengaja | Harus ada perintah terdokumentasi untuk memperbarui berkas acuan, dan setiap pembaruan harus dicatat di Daftar perubahan visual (Bagian 10.3). | Pembaruan acuan tanpa catatan menghilangkan fungsi pengaman. | README memuat perintah itu. Setiap permintaan tarik yang mengubah `tests/golden/` memuat entri baru di Daftar perubahan visual. | Fungsional-005 |
| Fungsional-007 Pembangunan lintas kompiler | `CMakeLists.txt` harus memakai opsi peringatan sesuai kompiler (`-Wall -Wextra -Wpedantic` untuk g++ dan clang, `/W4` untuk kompiler Microsoft), menyembunyikan simbol secara bawaan, dan mengaktifkan `POSITION_INDEPENDENT_CODE` pada `npc_core`. | Fakta-010 dan Fakta-012. Kode yang tidak bergantung pada posisi membuat `npc_core` aman ditautkan ke pustaka dinamis pada semua kompiler, walau Linux dengan g++ sudah berhasil (Fakta-011). | `nm -D` pada pustaka bersama hanya menampilkan simbol C yang sengaja diekspor. Pembangunan dengan kompiler Microsoft selesai tanpa galat. | Fungsional-001 |
| Fungsional-008 Berkas yang diabaikan | `.gitignore` harus mencakup `*.dll`, `*.dylib`, `*.exe`, `*.pdb`, dan folder `build-*/`. | Saat ini hanya `build/`, `*.so`, `*.a`, `*.o`, dan `.godot/`. | Pembangunan di tiga sistem operasi tidak meninggalkan berkas tak terlacak pada keluaran `git status`. | Tidak ada |
| Fungsional-009 Integrasi berkelanjutan | Alur kerja GitHub Actions harus membangun dan menjalankan `ctest` pada Linux, Windows, dan macOS, masing-masing untuk Debug dan Release, pada setiap dorongan dan setiap permintaan tarik. | Determinisme lintas sistem operasi hanya terbukti bila dijalankan di ketiganya. | Enam kombinasi berstatus hijau. Kegagalan satu tes di satu kombinasi membuat alur kerja merah. | Fungsional-005 |
| Fungsional-010 Lencana status nyata | README harus menampilkan lencana status alur kerja dan menghapus lencana statis “Tests 8/8 Passing”. | Lencana statis tidak mencerminkan hasil tes sebenarnya (Fakta-018). | Lencana berubah merah ketika satu tes sengaja digagalkan pada cabang uji. | Fungsional-009 |

#### 7.1.1 Matriks acuan

Matriks acuan terdiri dari dua kelompok kombinasi.

- **Kelompok dasar**: benih 1 sampai 100, enam keadaan animasi, empat arah, dan tiga nilai waktu animasi (0,0; 0,25; dan 0,5 detik), tanpa kondisi fisik. Jumlahnya 7.200 kombinasi.
- **Kelompok kondisi**: benih 1 sampai 10, keadaan Idle, arah South, waktu 0,0 detik. Ada enam belas skenario per benih: amputasi masing-masing dari sepuluh bagian tubuh, dan satu cedera untuk tiap enam jenis cedera pada bagian tubuh Torso (benih visual 1, tingkat keparahan 0,5). Jumlahnya 160 kombinasi.

Totalnya 7.360 kombinasi. Sidik digital dihitung dengan algoritma FNV-1a 64 bit atas keempat saluran warna (merah, hijau, biru, alfa) setiap piksel, berurutan baris demi baris dari kiri atas, dan ditulis sebagai 16 karakter heksadesimal. Format baris: benih, animasi, arah, waktu, skenario, sidik digital.


### 7.2 Tahap 2: Penyempurnaan Jembatan C

Tahap ini memperbaiki dan menstabilkan kontrak C yang sudah ada. Perluasan Godot tetap akan memanggil Inti C++ secara langsung sesuai arsitektur sasaran, tetapi Jembatan C harus tetap benar untuk konsumen lain dan menjadi permukaan integrasi yang dapat diuji.

| Pengenal dan judul | Pernyataan | Alasan | Cara verifikasi | Bergantung pada |
| --- | --- | --- | --- | --- |
| Fungsional-011 Kesetaraan jalur Inti dan Jembatan C | Untuk masukan yang sama, `npc_render_pixels` harus menghasilkan penyangga piksel yang identik dengan pemanggilan `CharacterGenerator::render` secara langsung. | Fakta-007 menunjukkan seluruh 100 sampel yang diuji tidak cocok. | Tes hasil acuan menjalankan kedua jalur untuk matriks yang sama dan membandingkan setiap byte RGBA, bukan hanya hash. | Fungsional-005 |
| Fungsional-012 Kontrak struktur ber-versi | Struktur konfigurasi dan penampilan publik harus memiliki versi kontrak eksplisit. Perubahan tata letak ABI harus ditangani secara sadar, bukan diam-diam. | Mencegah konsumen lama membaca struktur baru dengan tata letak berbeda. | Tes memeriksa versi yang didukung; versi tidak dikenal menghasilkan status galat yang terdokumentasi. | Fungsional-007 |
| Fungsional-013 Pelengkapan data penampilan | Antarmuka publik harus dapat mengirim dan menerima seluruh field penampilan yang dimiliki Inti, termasuk warna mata, bentuk mata, jarak mata, bentuk hidung, bentuk mulut, ekspresi, serta versi generator. | Fakta-006 menunjukkan data hilang ketika melewati Jembatan C. | Buat penampilan dengan nilai berbeda pada setiap field, lakukan round-trip, lalu bandingkan seluruh field. | Fungsional-012 |
| Fungsional-014 Parameter render lengkap | Fungsi render harus menerima konfigurasi render lengkap: keadaan animasi, arah, waktu animasi, kecepatan animasi, amputasi, cedera, dan opsi relevan yang telah didefinisikan oleh model Inti. | Menghindari nilai diam-diam yang dipaksakan oleh bridge. | Tes setiap parameter dengan nilai nonbawaan dan pastikan hasil sesuai pemanggilan langsung Inti. | Fungsional-013 |
| Fungsional-015 Validasi enum dan angka | Fungsi publik harus memvalidasi enum, rentang angka, pointer, panjang buffer, dan versi sebelum membaca atau menulis data. Masukan tidak valid harus menghasilkan status galat yang jelas tanpa crash atau akses di luar batas. | Fakta-006 dan Fakta-008 menunjukkan masukan invalid belum divalidasi secara memadai. | Tes nilai enum tidak dikenal, nilai di luar rentang, pointer null, buffer terlalu kecil, dan versi tidak didukung. | Fungsional-012 |
| Fungsional-016 Status galat yang konsisten | Operasi C harus mengembalikan kode status yang terdokumentasi dan menyediakan cara mengambil pesan galat bila diperlukan. Tidak boleh memakai nilai hasil yang ambigu untuk menyatakan kegagalan. | Konsumen harus dapat membedakan hasil valid dari kegagalan. | Tes memicu setiap kategori galat dan memeriksa kode status serta pesan yang sesuai. | Fungsional-015 |
| Fungsional-017 Kepemilikan memori eksplisit | Header publik harus menjelaskan siapa yang mengalokasikan dan membebaskan setiap buffer. API harus menyediakan pasangan fungsi pelepas untuk memori yang dialokasikan oleh pustaka, atau memakai buffer keluaran milik pemanggil dengan kapasitas eksplisit. | Mencegah kebocoran dan pembebasan memori lintas batas pustaka yang tidak aman. | Tes berulang, pemeriksaan sanitizer bila tersedia, dan dokumentasi contoh penggunaan. | Fungsional-015 |
| Fungsional-018 Ukuran struktur yang aman | Bila struktur C publik digunakan sebagai parameter ABI, struktur tersebut harus menyertakan strategi kompatibilitas ukuran/versi, misalnya `struct_size` dan `api_version`, atau strategi setara yang didokumentasikan. | Memungkinkan penambahan field tanpa membaca memori yang tidak disediakan konsumen lama. | Tes struktur dengan ukuran/versi yang tidak didukung harus ditolak secara aman. | Fungsional-012 |
| Fungsional-019 API generator dan renderer terpisah | API harus memisahkan pembangkitan penampilan dari penggambaran. Pembangkitan menerima benih dan konfigurasi; render menerima penampilan yang sudah ada dan konfigurasi render. | Mendukung reproduksi karakter dan animasi tanpa membangkitkan ulang identitasnya setiap frame. | Tes menghasilkan penampilan sekali lalu merendernya pada beberapa arah/keadaan tanpa mengubah field penampilan. | Fungsional-014 |
| Fungsional-020 Kompatibilitas simbol publik | Hanya simbol yang memang bagian dari API C yang boleh diekspor dari pustaka bersama; kelas dan fungsi internal Inti tidak boleh menjadi ABI publik tanpa keputusan terpisah. | Mengurangi ketergantungan konsumen pada detail implementasi C++. | Pemeriksaan simbol ekspor di CI pada platform yang mendukung pemeriksaan tersebut; verifikasi setara di Windows/macOS. | Fungsional-007 |

#### 7.2.1 Ketentuan API C

Nama dan tanda tangan akhir fungsi boleh disesuaikan dengan gaya kode yang sudah ada, tetapi kemampuan berikut wajib tersedia:

- Membaca versi API.
- Membuat penampilan dari benih dan konfigurasi generator.
- Merender penampilan menjadi RGBA dengan ukuran buffer yang diketahui.
- Mengambil ukuran keluaran dan stride/urutan saluran secara eksplisit.
- Menyampaikan amputasi dan cedera tanpa kehilangan data.
- Melakukan serialisasi dan deserialisasi dengan validasi.
- Mengembalikan status galat yang stabil.
- Menyediakan fungsi penghancur/pelepas untuk objek atau buffer yang dimiliki pustaka, bila API memakai alokasi internal.

Jangan menambahkan logika anatomi atau algoritma penggambaran baru ke `c_api.cpp`. Berkas tersebut hanya menjadi lapisan adaptasi dan validasi.

### 7.3 Tahap 3: Perluasan Godot melalui GDExtension

Tahap ini merupakan fokus migrasi. Implementasi harus menggunakan `godot-cpp` yang versinya cocok dengan versi Godot yang dipakai proyek. Versi Godot yang tertulis di `project.godot` saat analisis adalah 4.7; versi dan tag `godot-cpp` yang benar harus dipastikan sebelum dependensi dikunci. Jangan mengandalkan cabang `master` yang bergerak tanpa versi yang tercatat.

| Pengenal dan judul | Pernyataan | Alasan | Cara verifikasi | Bergantung pada |
| --- | --- | --- | --- | --- |
| Fungsional-021 Proyek GDExtension terpisah | Harus ada direktori perluasan, usulan `extension/`, yang berisi kode adapter Godot, konfigurasi build, dan berkas registrasi. Direktori tersebut tidak boleh menaruh algoritma anatomi atau rasterizer baru. | Memisahkan integrasi engine dari Inti portabel. | Struktur proyek dan tinjauan kode menunjukkan adapter hanya memanggil API Inti. | Fungsional-007 |
| Fungsional-022 Integrasi godot-cpp yang dipatok | Dependensi `godot-cpp` harus ditetapkan ke tag/commit yang kompatibel dengan versi Godot proyek dan dicatat di dokumentasi serta build. | Mencegah build rusak akibat perubahan dependensi yang tidak disengaja. | Build bersih menggunakan versi dependensi yang terdokumentasi berhasil di Linux; CI memverifikasi platform sasaran. | Fungsional-021 |
| Fungsional-023 Berkas registrasi `.gdextension` | Harus tersedia berkas `.gdextension` dengan `entry_symbol` yang sesuai dan pemetaan pustaka per platform/arsitektur yang benar-benar didukung. | Godot perlu menemukan titik masuk dan biner native. | Proyek dapat memuat perluasan saat dibuka dan tidak mencatat galat pemuatan pada Output. | Fungsional-022 |
| Fungsional-024 Registrasi kelas native | Perluasan harus mendaftarkan minimal kelas generator yang dapat dipanggil dari GDScript, misalnya `NPCGenerator`, dan kelas/struktur adapter tambahan hanya bila diperlukan. Nama final harus konsisten dan terdokumentasi. | Memberikan API Godot yang jelas, bukan akses langsung ke fungsi internal. | Kelas terlihat melalui API Godot dan dapat dibuat/dipanggil dari skrip pratinjau. | Fungsional-023 |
| Fungsional-025 API pembangkitan penampilan | API native harus menerima benih dan konfigurasi pembangkitan, lalu mengembalikan data penampilan lengkap yang dapat digunakan kembali. API tidak boleh membuat karakter baru otomatis pada setiap render. | Memisahkan identitas/penampilan dari frame animasi. | Benih dan konfigurasi yang sama menghasilkan seluruh field penampilan yang sama pada build yang sama. | Fungsional-024 |
| Fungsional-026 API render RGBA | API native harus merender data penampilan beserta konfigurasi render menjadi hasil RGBA dengan dimensi yang eksplisit. Kontrak harus menetapkan urutan saluran, ukuran, stride, dan orientasi baris. | Godot harus mengubah hasil rasterizer menjadi `Image` tanpa interpretasi ambigu. | Bandingkan byte hasil GDExtension dengan hasil langsung Inti pada seluruh matriks acuan. | Fungsional-025 |
| Fungsional-027 Konversi ke Image/ImageTexture | Adapter harus menyediakan cara aman dan terdokumentasi untuk mengubah hasil RGBA menjadi `Image` dan `ImageTexture` Godot. Pengubahan ke tekstur dilakukan hanya saat gambar perlu diperbarui, bukan tanpa syarat setiap frame. | Menghindari biaya pembuatan objek tekstur yang tidak perlu. | Pengukuran menunjukkan render yang tidak berubah tidak membuat tekstur baru setiap frame; hasil piksel tetap identik. | Fungsional-026 |
| Fungsional-028 Dukungan semua parameter render | API native harus menerima enam keadaan animasi, empat arah, waktu animasi, kecepatan, amputasi, dan cedera yang didukung Inti. | Mencegah hilangnya kemampuan yang tersedia di core. | Tes API native untuk semua keadaan dan arah, serta semua jenis amputasi/cedera yang model datanya dukung. | Fungsional-026 |
| Fungsional-029 Validasi argumen Godot | Nilai dari GDScript harus divalidasi di batas native. Nilai salah harus menghasilkan galat yang dapat didiagnosis, bukan crash native atau perilaku tak terdefinisi. | Data skrip tidak boleh dipercaya sebagai masukan valid hanya karena berasal dari UI. | Tes argumen kosong, tipe salah, enum tidak dikenal, indeks di luar rentang, dan konfigurasi tidak lengkap. | Fungsional-024 |
| Fungsional-030 Galat pemuatan yang dapat ditindaklanjuti | Bila biner GDExtension hilang, tidak cocok dengan platform, atau gagal dimuat, dokumentasi dan pesan diagnosis harus menunjukkan nama artefak, konfigurasi build, dan langkah pemulihan yang relevan. | Masalah biner native sering kali tidak tampak seperti galat GDScript biasa. | Uji dengan memindahkan biner sementara atau memilih path yang salah; pesan harus membantu menemukan penyebab. | Fungsional-023 |
| Fungsional-031 Ekspor platform yang dideklarasikan | Konfigurasi harus mencantumkan platform/arsitektur yang benar-benar dibangun. Platform yang belum didukung tidak boleh dipetakan ke biner palsu atau dianggap sudah lulus. | Mencegah klaim kompatibilitas yang tidak dibuktikan. | Matriks build dan berkas `.gdextension` hanya memuat artefak yang tersedia. | Fungsional-022 |
| Fungsional-032 Satu sumber kebenaran | Setelah migrasi disetujui, pembangkitan penampilan dan rasterisasi di GDScript harus dihapus atau dipangkas menjadi adapter/UI saja. Tidak boleh ada algoritma kedua yang menghasilkan anatomi atau piksel. | Menghilangkan penyimpangan dua implementasi. | Pencarian kode dan tinjauan menunjukkan rumus generator/rasterizer hanya berada di `core/`. | Fungsional-026 |
| Fungsional-033 Inisialisasi dan pelepasan sumber daya | Perluasan harus mengikuti siklus hidup inisialisasi/terminasi Godot dan tidak menyimpan pointer ke data sementara setelah pemanggilan berakhir. | Menghindari akses use-after-free dan kerusakan saat menutup proyek. | Buka/tutup proyek dan jalankan render berulang tanpa crash atau kebocoran yang terdeteksi oleh alat yang tersedia. | Fungsional-024 |
| Fungsional-034 API native yang sederhana | API yang dilihat GDScript sebaiknya berbentuk operasi tingkat domain, bukan mengekspos detail `PixelBuffer`, pointer mentah, atau struktur internal C++. | Menjaga skrip UI mudah dipakai dan memberi ruang perubahan internal. | Skrip preview tidak perlu mengetahui detail alokasi atau layout internal buffer. | Fungsional-024 |

#### 7.3.1 Bentuk API sasaran (ilustratif, bukan nama wajib)

Contoh berikut menjelaskan bentuk penggunaan yang diharapkan. Ini bukan perintah untuk menyalin nama/metode secara buta; nama akhir harus mengikuti gaya kode dan kemampuan `godot-cpp` yang dipilih.

```gdscript
var generator := NPCGenerator.new()
var appearance = generator.generate_appearance(seed, config)
var image: Image = generator.render_image(
    appearance,
    animation_state,
    direction,
    animation_time,
    amputations,
    injuries
)
var texture := ImageTexture.create_from_image(image)
$CharacterPreview.texture = texture
```

Jika lebih bersih, fungsi render dapat mengembalikan `Image` dan skrip Godot membuat `ImageTexture`; atau adapter dapat menyediakan metode pembantu yang menghasilkan tekstur. Keputusan ini harus mempertimbangkan kepemilikan memori, frekuensi pembaruan, dan pengujian. Jangan memaksa pengembalian `ImageTexture` dari C++ jika hal itu membuat API Inti bergantung pada Godot.

### 7.4 Tahap 4: Perapian Inti dan serializer

| Pengenal dan judul | Pernyataan | Alasan | Cara verifikasi | Bergantung pada |
| --- | --- | --- | --- | --- |
| Fungsional-035 Pemisahan fungsi render | Fungsi `render()` yang terlalu besar sebaiknya dipecah menjadi fungsi privat yang memiliki tanggung jawab jelas, misalnya menggambar bagian tubuh, komposisi, animasi/pose, dan overlay kondisi. Nama final mengikuti struktur aktual. | Memudahkan pengujian dan perubahan tanpa membuat implementasi render kedua. | Tes hasil acuan tetap lulus setelah refaktor tanpa perubahan keluaran yang disengaja. | Fungsional-005 |
| Fungsional-036 Kesetaraan serializer | Serializer harus menyimpan dan memulihkan seluruh field penampilan yang termasuk kontrak serialisasi, termasuk warna mata dan field baru yang sudah ditetapkan. | Fakta-008 menunjukkan data hilang pada round-trip. | Tes round-trip membandingkan seluruh field, bukan subset. | Fungsional-013 |
| Fungsional-037 Validasi deserialisasi | Deserialisasi harus memvalidasi format, versi, rentang enum/numerik, nilai warna, jumlah elemen, dan kelengkapan field sebelum menerima data. | Data rusak tidak boleh diam-diam menjadi karakter valid dengan nilai terpotong. | Berkas valid diterima; setiap sampel rusak/versi tak didukung ditolak dengan galat yang jelas. | Fungsional-036 |
| Fungsional-038 Versi format serialisasi | Format simpan harus memiliki versi eksplisit. Kebijakan untuk versi lama harus ditetapkan: migrasi, penolakan dengan pesan jelas, atau dukungan kompatibilitas terbatas. | Perubahan struktur di masa depan perlu dapat dideteksi. | Tes format versi saat ini, versi lama yang didukung, dan versi yang tidak dikenal. | Fungsional-037 |
| Fungsional-039 Stabilitas benih | Untuk versi generator yang sama, benih dan konfigurasi yang sama harus menghasilkan penampilan identik. Perubahan algoritma yang disengaja harus menaikkan atau memperbarui versi generator sesuai kebijakan. | Menjamin reproduksi karakter yang tersimpan atau dibagikan. | Tes determinisme membandingkan seluruh field dan hasil piksel. | Fungsional-005 |
| Fungsional-040 Kebijakan perubahan visual | Perubahan hasil render yang disengaja harus memperbarui hasil acuan dan Daftar perubahan visual. Perubahan yang tidak disengaja harus menggagalkan CI. | Hash hanya mendeteksi perbedaan; peninjau tetap perlu mengetahui apakah perbedaan memang diharapkan. | PR yang mengubah hasil acuan memiliki alasan, bukti visual, dan entri perubahan. | Fungsional-006 |
| Fungsional-041 Semantik PixelBuffer | Kontrak `PixelBuffer` harus menetapkan ukuran, format RGBA, koordinat, batas, dan semantik alpha. Perbaikan blending alpha yang dikecualikan dari dokumen ini tidak boleh dilakukan diam-diam dalam migrasi. | Memisahkan perubahan integrasi dari perubahan visual. | Tes batas dan kontrak format; setiap perubahan blending ditangani pada pekerjaan terpisah atau perubahan yang disetujui. | Fungsional-005 |
| Fungsional-042 Uji kondisi tubuh | Tes harus memastikan amputasi dan cedera tidak mengubah field penampilan yang tidak berkaitan dan tidak mengakses area di luar buffer. | Kondisi adalah lapisan render yang terpisah dari identitas dasar. | Tes kombinasi kondisi, sanitizers jika tersedia, dan hasil acuan. | Fungsional-014 |
| Fungsional-043 Dokumentasi generator version | README dan API harus menjelaskan arti `generator_version`, kapan berubah, dan hubungannya dengan benih serta serializer. | Nomor versi generator tidak boleh sekadar angka dekoratif. | Dokumentasi memuat kebijakan dan tes membuktikan perilaku versi. | Fungsional-039 |

## 8. Kebutuhan nonfungsional

| Pengenal | Kebutuhan | Kriteria penerimaan |
| --- | --- | --- |
| Nonfungsional-001 Determinisme | Untuk versi generator dan input yang sama, hasil field dan byte piksel harus identik pada konfigurasi build/platform yang dinyatakan didukung. | Matriks acuan lulus pada setiap kombinasi CI. Jika ada ketergantungan platform yang terbukti, hal itu harus didokumentasikan dan target identik lintas platform ditinjau ulang secara eksplisit. |
| Nonfungsional-002 Portabilitas | Inti tetap C++20 dan tidak menyertakan header Godot. | Target `npc_core` dibangun tanpa `godot-cpp`. |
| Nonfungsional-003 Isolasi dependensi | Hanya target GDExtension yang bergantung pada `godot-cpp`; tes Inti tidak membutuhkan Godot untuk dijalankan. | Build dan `ctest` Inti dapat dijalankan di lingkungan tanpa editor Godot. |
| Nonfungsional-004 Keamanan memori | Tidak ada akses di luar batas buffer, pointer menggantung, atau pelepasan memori oleh allocator yang berbeda tanpa kontrak. | Tes batas dan sanitizer yang tersedia lulus; tidak ada galat yang diketahui pada skenario uji. |
| Nonfungsional-005 Diagnostik | Kegagalan build, pemuatan ekstensi, validasi input, dan tes harus menyertakan konteks yang dapat ditindaklanjuti. | Pesan menyebut tahap, konfigurasi, dan penyebab yang terdeteksi. |
| Nonfungsional-006 Pemeliharaan | Rumus pembangkitan penampilan dan penggambaran hanya ada satu kali. | Tidak ada salinan algoritma di GDScript atau adapter GDExtension. |
| Nonfungsional-007 Kinerja | Migrasi tidak boleh menurunkan kinerja secara material tanpa alasan. Pengukuran dilakukan sebelum dan sesudah pada mesin yang sama. | Catat median dan persentil ke-99 untuk render inti dan alur Godot; target angka final ditetapkan setelah baseline mesin pengembang diukur. |
| Nonfungsional-008 Beban frame | Preview tidak membuat `ImageTexture` baru pada setiap frame ketika parameter dan hasil gambar tidak berubah. | Uji profil atau instrumentasi menunjukkan pembaruan tekstur hanya saat diperlukan. |
| Nonfungsional-009 Kualitas kode | Kompiler berjalan dengan peringatan yang relevan; warning baru tidak diabaikan tanpa alasan. | Build CI tidak memiliki warning baru yang tidak disetujui. |
| Nonfungsional-010 Reproduksibilitas build | Versi compiler, CMake, Godot, `godot-cpp`, dan opsi build penting didokumentasikan. | Pengembang lain dapat mengikuti langkah README dari checkout bersih. |
| Nonfungsional-011 Kompatibilitas API | API publik memiliki versi dan aturan perubahan yang terdokumentasi. | Perubahan breaking memperbarui versi kontrak dan tes kompatibilitas terkait. |
| Nonfungsional-012 Batas cakupan | Perubahan integrasi tidak boleh menyelipkan fitur gameplay atau polesan anatomi di luar cakupan. | PR dipisahkan atau disetujui secara eksplisit bila menyentuh tujuan bukan-cakupan. |

### 8.1 Kinerja dan strategi pembaruan gambar

Kinerja awal pada Fakta-014 bukan jaminan kinerja pada komputer pengembang atau saat dijalankan melalui Godot. Karena itu:

1. Catat baseline render Inti, biaya konversi buffer menjadi `Image`, biaya pembuatan `ImageTexture`, dan biaya keseluruhan preview.
2. Hindari pembangkitan ulang penampilan ketika pengguna hanya mengubah arah atau waktu animasi.
3. Hindari pembuatan `ImageTexture` baru bila frame yang ditampilkan tidak berubah.
4. Jangan membangun cache sprite/atlas pada tahap ini. Tambahkan hanya jika pengukuran pada jumlah NPC dan frekuensi render yang realistis menunjukkan kebutuhan.
5. Jangan mengklaim target ribuan NPC terpenuhi hanya berdasarkan mikrobenchmark satu karakter.

## 9. Perubahan pada proyek dan struktur berkas

Struktur berikut merupakan usulan tujuan, bukan pernyataan bahwa berkas-berkas ini sudah ada. Agen implementasi harus memeriksa struktur aktual sebelum menambah atau memindahkan berkas.

```text
npc-builder/
├── CMakeLists.txt
├── CMakePresets.json                 # bila berguna untuk preset lintas platform
├── core/
│   ├── include/npc/
│   └── src/
├── extension/
│   ├── CMakeLists.txt
│   ├── project/                      # bila perlu contoh proyek/build tooling
│   ├── src/
│   │   ├── register_types.cpp
│   │   ├── register_types.hpp
│   │   ├── npc_generator_extension.cpp
│   │   └── npc_generator_extension.hpp
│   └── npc_builder.gdextension
├── godot/
│   ├── project.godot
│   ├── scenes/preview.tscn
│   └── scripts/
│       └── preview_ui.gd
├── tests/
│   ├── test_generator.cpp
│   ├── test_c_api.cpp
│   ├── test_gdextension/              # smoke/integration test bila memungkinkan
│   └── golden/
│       └── matriks-acuan.txt
├── .github/workflows/
│   └── ci.yml
├── docs/
│   ├── NPC_BUILDER_V021_AUDIT.md
│   └── ...                            # dokumen yang ada dipertahankan
└── README.md
```

Ketentuan perubahan berkas:

- Jangan menghapus atau mengganti nama dokumen `BRD_` lama.
- Jangan menghapus tes lama hanya karena tes baru dibuat. Perbaiki atau ganti hanya bila ada alasan dan cakupan tes pengganti yang setara atau lebih kuat.
- Jangan mengubah `core/` agar bergantung pada `godot-cpp`.
- Jangan menjadikan GDExtension tempat baru untuk algoritma anatomi; ia hanya adapter.
- Jangan menambahkan biner hasil build ke Git kecuali kebijakan rilis proyek secara eksplisit memerlukannya.
- Berkas `.gdextension` harus mengarah ke artefak yang benar-benar dihasilkan build, dengan nama dan suffix platform yang sesuai.

## 10. Alur kerja implementasi

### 10.1 Urutan tahap

| Tahap | Isi | Gerbang keluar |
| --- | --- | --- |
| Tahap 1 — Fondasi | CTest, pemeriksaan yang aktif di Release, hasil acuan, CMake lintas kompiler, CI | Tes benar-benar gagal bila hasil sengaja dirusak; Inti terbangun dan diuji di CI. |
| Tahap 2 — Jembatan C | API lengkap, validasi, status galat, versi, serializer | Jalur C identik dengan Inti dan seluruh input invalid ditolak dengan aman. |
| Tahap 3 — GDExtension | `godot-cpp`, registrasi kelas, API native, pemuatan biner, konversi hasil ke Godot | Preview memanggil C++ melalui GDExtension dan cocok dengan hasil acuan. |
| Tahap 4 — Perapian Inti | Pemecahan render, serializer dan dokumentasi | Tes penuh lulus, perubahan hasil visual tercatat, README mencerminkan arsitektur final. |

Tahap dapat dikerjakan dalam beberapa pull request kecil. Jangan melanjutkan ke gerbang tahap berikutnya bila gerbang sebelumnya gagal. Jika keterbatasan CI membuat salah satu platform belum dapat diuji, tandai secara eksplisit sebagai belum terverifikasi; jangan menyatakan semua platform didukung.

### 10.2 Aturan pelaksanaan untuk agen coding

Instruksi berikut berlaku untuk agen coding yang mengerjakan BRD ini:

1. Mulai dengan memeriksa branch, commit, `git status`, CMake, semua header publik, serializer, tes, dan cara proyek Godot saat ini berjalan. Jangan mengasumsikan struktur dari dokumen masih sama dengan working tree.
2. Buat rencana perubahan per tahap dan daftar berkas yang akan diubah sebelum mengedit.
3. Pertahankan perubahan pengguna yang sudah ada. Jangan melakukan reset keras, checkout destruktif, atau menghapus perubahan yang tidak dibuat oleh agen.
4. Buat satu perubahan konseptual per commit bila workflow pengguna mendukung commit; jangan membuat commit otomatis tanpa diminta.
5. Setelah setiap tahap, jalankan build dan tes yang relevan, laporkan perintah dan hasil nyata. Jangan menulis “lulus” jika tes tidak dijalankan.
6. Jika ada keputusan yang memengaruhi ABI, serialisasi, determinisme, nama API publik, atau kompatibilitas versi Godot, jelaskan dampaknya sebelum memilih.
7. Jangan mengganti seluruh implementasi hanya agar kode tampak lebih rapi. Utamakan perubahan minimal yang memenuhi kebutuhan.
8. Jangan menghapus GDScript renderer sebelum GDExtension berhasil dimuat dan hasilnya dibandingkan dengan Inti.
9. Jangan memperbarui golden fixtures secara otomatis ketika tes gagal. Perbedaan harus diperiksa dan disetujui terlebih dahulu.
10. Jangan menutupi kegagalan lintas platform dengan menonaktifkan tes, menandai tes sebagai lulus, atau mengecualikan platform tanpa alasan yang tercatat.
11. Jangan memasukkan pakaian, gender, genom, cache atlas, atau perubahan visual yang dikecualikan ke dalam pekerjaan migrasi ini.
12. Di akhir setiap tahap, laporkan: ringkasan, berkas berubah, keputusan teknis, tes yang dijalankan, hasil, risiko tersisa, dan langkah berikutnya.
13. Jika dependensi atau versi engine tidak bisa dipastikan dari repo, berhenti pada pemeriksaan dan berikan opsi dengan alasan; jangan mengarang nomor versi.
14. Gunakan bahasa Indonesia untuk laporan kerja, tetapi pertahankan nama simbol, opsi compiler, perintah terminal, dan istilah teknis apa adanya.
15. Jangan menganggap penggunaan GDExtension otomatis mempercepat seluruh alur. Ukur overhead render, konversi `Image`, dan pembaruan tekstur.

### 10.3 Daftar perubahan visual

Setiap perubahan terencana terhadap hasil piksel dicatat dengan format:

| Tanggal | Perubahan | Alasan | Hasil acuan diperbarui? | Pemeriksa |
| --- | --- | --- | --- | --- |
| Belum ada | Belum ada perubahan yang disetujui untuk dokumen ini | — | Tidak | — |

Aturan:

- Integrasi GDExtension seharusnya tidak mengubah hasil render Inti.
- Bila hasil Godot berubah setelah migrasi, pertama-tama anggap itu sebagai kemungkinan bug integrasi, konversi kanal, orientasi baris, ukuran buffer, atau data yang hilang.
- Golden fixture hanya diperbarui setelah perbedaan dipahami dan disetujui sebagai perubahan yang memang diinginkan.
- Catat apakah perubahan mengubah hasil Inti, hasil jalur C, atau hanya jalur konversi Godot.

## 11. Kriteria penerimaan menyeluruh

BRD dianggap selesai hanya bila semua kriteria berikut terpenuhi atau pengecualian tertulis telah disetujui.

- [ ] `ctest` menjalankan semua tes yang terdaftar dan keluar dengan kode gagal saat satu tes gagal.
- [ ] Pemeriksaan tetap aktif pada Debug dan Release.
- [ ] Hasil acuan berisi 7.360 kombinasi yang didefinisikan, atau perubahan jumlah disetujui dan didokumentasikan.
- [ ] Jalur render Inti dan Jembatan C menghasilkan byte RGBA identik untuk matriks uji.
- [ ] Serializer mempertahankan semua field kontrak dan menolak data rusak/versi yang tidak didukung.
- [ ] Target Inti tetap dapat dibangun tanpa Godot dan `godot-cpp`.
- [ ] GDExtension dapat dibangun dengan versi dependensi yang tercatat dan dimuat oleh versi Godot yang ditetapkan.
- [ ] Skrip preview memanggil API GDExtension untuk pembangkitan dan render; tidak ada generator atau rasterizer duplikat di GDScript.
- [ ] Hasil render yang ditampilkan Godot identik dengan hasil acuan C++.
- [ ] Enam keadaan animasi, empat arah, amputasi, dan cedera yang didukung dapat dikirim melalui API native.
- [ ] Perubahan input invalid menghasilkan galat yang aman dan dapat didiagnosis.
- [ ] CI membangun dan menjalankan tes pada semua platform/konfigurasi yang diklaim didukung. Kombinasi yang belum diverifikasi dinyatakan terbuka.
- [ ] Preview tidak membuat tekstur baru tanpa kebutuhan pada setiap frame.
- [ ] README menjelaskan setup dependensi, build, tes, pemuatan ekstensi, dan pemecahan masalah.
- [ ] Tidak ada perubahan fitur di luar ruang lingkup yang terselip tanpa persetujuan.
- [ ] Daftar perubahan visual diperbarui bila golden fixture berubah.

## 12. Risiko dan mitigasi

| Pengenal | Risiko | Dampak | Mitigasi |
| --- | --- | --- | --- |
| Risiko-001 | Versi `godot-cpp` tidak cocok dengan versi Godot | Ekstensi gagal dibangun atau dimuat | Patok tag/commit yang sesuai; verifikasi versi editor dan konfigurasi proyek sebelum coding. |
| Risiko-002 | Perbedaan ABI atau compiler pada platform berbeda | Build berhasil di Linux tetapi gagal di Windows/macOS | Pisahkan Inti dan ekstensi; CI per platform; gunakan tipe portabel dan opsi compiler yang sesuai. |
| Risiko-003 | Hasil piksel berubah saat konversi ke `Image` | Visual tidak identik meski Inti benar | Tetapkan kontrak RGBA, stride, ukuran, orientasi baris; bandingkan byte dan golden hash. |
| Risiko-004 | GDScript renderer dihapus terlalu cepat | Preview tidak dapat digunakan saat migrasi belum selesai | Pertahankan jalur lama sementara sebagai fallback pengembangan, tetapi jangan biarkan dua jalur menjadi implementasi permanen. Hapus setelah smoke test dan kesetaraan lolos. |
| Risiko-005 | Hash fixture diperbarui untuk menghilangkan kegagalan | Regresi visual lolos tanpa tinjauan | Pisahkan perintah pembaruan fixture dari tes normal; wajibkan catatan perubahan dan review. |
| Risiko-006 | Tes Release tidak memeriksa apa pun | CI memberi rasa aman palsu | Ganti `assert` dengan mekanisme pemeriksaan yang tetap aktif di semua konfigurasi; uji dengan kegagalan yang disengaja. |
| Risiko-007 | Kontrak serialisasi berubah tanpa versi | Karakter tersimpan rusak atau kehilangan field | Versi format, validasi, round-trip semua field, kebijakan migrasi versi lama. |
| Risiko-008 | API native mengekspos detail internal | Refaktor Inti memaksa perubahan skrip Godot | Ekspos operasi tingkat domain dan tipe Godot yang stabil, bukan pointer atau kelas internal. |
| Risiko-009 | Pembuatan tekstur setiap frame membebani preview | Kinerja UI memburuk | Pisahkan render gambar dari pembaruan tekstur dan perbarui hanya ketika diperlukan. |
| Risiko-010 | Klaim dukungan tiga OS dibuat sebelum diverifikasi | Pengguna tidak dapat membangun di platform tertentu | Bedakan “ditargetkan” dari “terverifikasi”; CI hijau menjadi bukti dukungan. |
| Risiko-011 | Lingkup membesar ke fitur anatomi/gender/pakaian | Migrasi memakan waktu dan sulit ditinjau | Tegakkan bukan-tujuan; buka BRD terpisah untuk perubahan visual dan fitur domain. |
| Risiko-012 | Determinisme lintas OS gagal akibat operasi atau tipe yang tidak stabil | Hash berbeda pada platform tertentu | Gunakan tipe integer dengan lebar tetap untuk RNG, hindari perilaku tak terdefinisi, telusuri perbedaan byte sebelum mengubah persyaratan. |

## 13. Keputusan terbuka

| Pengenal | Pertanyaan | Pilihan | Rekomendasi awal | Pemilik/keputusan |
| --- | --- | --- | --- | --- |
| Terbuka-001 | Apakah GDExtension memanggil Inti langsung atau melalui Jembatan C? | A. Langsung dari adapter C++ ke Inti. B. GDExtension memanggil Jembatan C. | A, karena menghindari duplikasi kontrak C saat adapter berada dalam basis kode C++ yang sama. Jembatan C tetap dipertahankan untuk konsumen lain. | Menunggu persetujuan pemilik proyek |
| Terbuka-002 | Versi Godot dan `godot-cpp` mana yang menjadi baseline? | Versi yang dipakai proyek saat ini atau versi lain yang disetujui. | Tetapkan berdasarkan versi editor nyata dan tag `godot-cpp` yang cocok; jangan mengasumsikan hanya dari `config/features`. | Menunggu verifikasi |
| Terbuka-003 | Sistem operasi apa yang menjadi target rilis awal? | Linux saja untuk prototipe; atau Linux, Windows, dan macOS. | Mulai dari platform pengembangan yang dapat diuji, lalu tambahkan platform lain di CI sebelum mengklaim dukungan. | Menunggu keputusan |
| Terbuka-004 | Apakah serialisasi harus mempertahankan format lama? | A. Migrasi format lama. B. Tolak versi lama dengan pesan jelas. C. Dukungan terbatas. | Pilih berdasarkan apakah ada data karakter yang perlu dipertahankan; jangan membuat migrasi tanpa contoh fixture lama. | Menunggu inventaris data |
| Terbuka-005 | API GDExtension mengembalikan `Image` atau `ImageTexture`? | A. `Image`, skrip membuat tekstur. B. Helper native menghasilkan `ImageTexture`. | Mulai dengan `Image` agar batas antara rasterizer dan sumber daya tampilan jelas; ukur sebelum menambahkan helper tekstur. | Menunggu implementasi |
| Terbuka-006 | Apakah hash harus identik lintas semua compiler/OS atau hanya per platform? | A. Byte identik di semua platform. B. Identik dalam platform/versi generator tertentu. | Pertahankan target A sebagai sasaran awal; bila ada alasan teknis yang terbukti, setiap pengecualian harus disetujui dan didokumentasikan. | Menunggu hasil CI |
| Terbuka-007 | Framework tes apa yang digunakan? | Framework ringan yang sudah dipakai proyek, framework eksternal, atau helper internal. | Pilih yang paling kecil dan mudah dipelihara, asalkan pemeriksaan tetap aktif di Release dan pesan gagal informatif. | Menunggu pemeriksaan dependensi |
| Terbuka-008 | Apakah CI harus menjalankan smoke test Godot headless? | A. C++ CI saja. B. C++ CI ditambah Godot headless pada platform yang didukung. | B bila editor/headless runner dan biner ekstensi tersedia di CI; jika tidak, dokumentasikan smoke test manual sementara. | Menunggu verifikasi runner |
| Terbuka-009 | Bagaimana API menangani perubahan generator yang disengaja? | Naikkan `generator_version`, migrasikan data, atau izinkan hasil lama hanya dari fixture. | Tetapkan kebijakan versi yang eksplisit sebelum fitur genom atau variasi karakter ditambahkan. | Menunggu keputusan |

## 14. Definisi selesai (Definition of Done)

Satu tahap dinyatakan selesai bila:

1. Kebutuhan pada tahap tersebut telah diimplementasikan atau pengecualiannya disetujui tertulis.
2. Build dan tes yang relevan dijalankan, dan hasil aktual dicantumkan.
3. Tidak ada kegagalan tes yang disembunyikan atau fixture yang diperbarui tanpa alasan.
4. Dokumentasi yang berkaitan dengan build, API, atau perilaku telah diperbarui.
5. Perubahan tidak menghapus pekerjaan pengguna atau dokumen lama.
6. Perubahan yang menyentuh hasil visual memiliki bukti perbandingan dan entri Daftar perubahan visual.
7. Agen melaporkan berkas yang diubah, perintah yang dijalankan, hasil, risiko tersisa, serta keputusan yang masih menunggu.

## Lampiran A. Pengulangan pengukuran awal

Lampiran ini mencatat metode yang perlu diulang dari checkout commit `365badd`. Nama target atau perintah bisa berubah setelah CMake diperbaiki; perintah akhir yang benar harus ditulis ke README berdasarkan build aktual.

1. **Status awal:** catat `git rev-parse HEAD`, `git status --short`, versi compiler, versi CMake, dan versi Godot.
2. **Build Inti:** lakukan konfigurasi/build Release dan Debug secara terpisah. Jangan menyimpulkan tes bekerja hanya dari pesan “lulus”; pastikan jumlah tes dan kode keluarnya benar.
3. **Tes determinisme:** jalankan generator dua kali untuk benih yang sama dan bandingkan seluruh field penampilan, bukan hanya ID atau sebagian field.
4. **Kesetaraan renderer:** untuk benih 1–100 dengan Idle, South, waktu 0,0, bandingkan semua byte keluaran Inti langsung dengan jalur C.
5. **Serializer:** untuk 100 benih, serialisasi lalu deserialisasi; bandingkan seluruh field. Uji juga nilai invalid dan versi format tidak dikenal.
6. **Build Release:** sengaja gagalkan satu pemeriksaan tes untuk memastikan tes benar-benar gagal ketika `NDEBUG` aktif; pulihkan nilai harapan setelah eksperimen.
7. **Benchmark:** gunakan build dan mesin yang sama untuk baseline dan pengukuran sesudah migrasi; pisahkan waktu render Inti dari konversi dan pembaruan tekstur Godot.
8. **Fixture:** hasil acuan dibuat hanya dari implementasi Inti yang sudah ditinjau. Simpan metadata versi generator dan format fixture agar perubahan algoritma tidak tersamar sebagai perubahan acak.

Catatan: hasil pengukuran yang tercantum pada Bagian 5 adalah hasil yang dilaporkan untuk commit dasar dokumen ini. Agen implementasi harus mengulang pengukuran sebelum memakai angka tersebut sebagai baseline yang masih berlaku.

## Lampiran B. Rekomendasi susunan pull request

Urutan berikut direkomendasikan agar tinjauan dan diagnosis tetap sederhana.

1. **PR-1 — Tes dan CMake:** CTest, pemeriksaan tahan Release, target fixture, pengaturan warning, `.gitignore`, CI Inti.
2. **PR-2 — Hasil acuan:** generator fixture deterministik, tes hash, dokumentasi pembaruan fixture.
3. **PR-3 — Jembatan C:** validasi, field lengkap, kode galat, versi API, tes kesetaraan Inti/C.
4. **PR-4 — Serializer:** round-trip lengkap, validasi input, versi format, tes data rusak.
5. **PR-5 — Kerangka GDExtension:** patok `godot-cpp`, target build, registrasi kelas, `.gdextension`, smoke test pemuatan.
6. **PR-6 — Integrasi preview:** sambungkan UI ke native API, bandingkan output, pertahankan fallback hanya sampai gerbang penerimaan lolos.
7. **PR-7 — Penghapusan duplikasi:** hapus algoritma renderer/generator GDScript setelah hasil setara terbukti; rapikan dokumentasi.
8. **PR-8 — Perapian Inti:** refaktor internal `render()` tanpa perubahan piksel yang tidak disetujui.

Urutan PR boleh disesuaikan bila dependensi aktual mengharuskan, tetapi jangan mencampur perubahan visual, perubahan serializer, dan migrasi GDExtension dalam satu perubahan besar tanpa kebutuhan kuat.

## Lampiran C. Glosarium

- **Inti/Core:** pustaka C++20 portabel yang menyimpan algoritma dan model data Pembuat Karakter.
- **Jembatan C/C-ABI:** antarmuka fungsi C yang dapat dipanggil konsumen lintas bahasa dengan kontrak ABI yang eksplisit.
- **GDExtension:** mekanisme Godot untuk memuat kode native sebagai ekstensi tanpa membangun ulang engine.
- **godot-cpp:** binding C++ yang digunakan untuk mengembangkan kelas native GDExtension.
- **Golden fixture/hasil acuan:** berkas hasil yang telah disetujui untuk dibandingkan oleh tes regresi.
- **Hash/sidik digital:** ringkasan data untuk mendeteksi perubahan; dalam dokumen ini FNV-1a 64 bit atas byte RGBA.
- **RGBA:** empat saluran warna: merah, hijau, biru, dan alfa.
- **ABI:** kontrak biner yang menentukan cara data dan fungsi diekspos antar-komponen terkompilasi.
- **Smoke test:** tes singkat untuk membuktikan komponen dasar dapat dibangun, dimuat, dan menjalankan operasi utama.
- **Satu sumber kebenaran:** hanya satu implementasi yang memiliki rumus generator dan rasterizer; lapisan lain menjadi adapter atau UI.
