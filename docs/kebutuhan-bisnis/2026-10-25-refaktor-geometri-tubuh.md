# Refaktor Geometri Tubuh, Perbaikan Cedera-Amputasi, Perluasan Hasil Acuan, dan Kebijakan Versi

Proyek: Living World · Modul: Pembuat Karakter (repositori `npc-builder`) · Tanggal: 2026-10-25 · Versi: 1

---

## 1. Ringkasan

Fungsi penggambaran karakter di `core/src/generator.cpp` menghitung geometri tubuh seperti lebar bahu, pinggang, pinggul secara tersebar di dalam satu fungsi sepanjang 423 baris. Tahap cedera memakai ulang nilai tersebut melalui variabel lokal tanpa memeriksa status amputasi. Matriks hasil acuan hanya mencakup cedera pada badan dan amputasi tanpa cedera, sehingga kombinasi keduanya tidak teruji. Tiga nomor versi lintas lapisan belum memiliki kebijakan kenaikan yang jelas.

Solusinya adalah mengekstrak geometri tubuh ke dalam struktur data bersama `BodyGeometry` yang dihitung sekali dan dipakai oleh seluruh tahap penggambaran, memperbaiki loop cedera agar memeriksa status amputasi, memperluas matriks hasil acuan, dan menetapkan kebijakan versi lintas lapisan. Hasil yang diharapkan berupa kode penggambaran yang modular, cedera pada bagian yang diamputasi tidak digambar, cakupan pengujian meningkat dari 7.360 menjadi lebih dari 7.800 kombinasi, dan setiap perubahan kontrak lintas lapisan memiliki aturan kenaikan versi yang terdokumentasi.

---

## 2. Latar belakang

Versi `v0.3.2` menyelesaikan pengerasan validasi profil, integritas serialisasi, dan keandalan integrasi berkelanjutan. Dokumen tanggapan koreksi rencana v0.3.3 pada `docs/V0.3/Tanggapan atas Koreksi Rencana v0.3.3 Kompatibilitas, Cakupan Hasil Acuan, Versi, dan Kriteria Transisi.md` mengidentifikasi lima masalah:

1. Kompatibilitas cedera dan amputasi tidak didapat otomatis dari geometri.
2. Cakupan hasil acuan belum mencakup cedera pada seluruh bagian tubuh dan kombinasi cedera-amputasi.
3. Tiga nomor versi tidak boleh dinaikkan secara otomatis tanpa analisis dampak.
4. Kriteria transisi geometri mengenai kemonotonan dan ambang diskret belum presisi.
5. Fungsi `compute_geometry()` harus kecil dan hanya menyimpan nilai dasar.

Pekerjaan ini menjadi prasyarat bagi fitur mendatang seperti profil anatomi wanita, pakaian modular, dan ras fantasi, yang semuanya memerlukan geometri tubuh yang terstruktur dan terpisah dari logika penggambaran.

---

## 3. Tujuan dan bukan-tujuan

### Tujuan

- Mengekstrak 7 nilai dasar geometri tubuh ke dalam struktur `BodyGeometry` yang dihitung dari parameter anatomi.
- Menyimpan `BodyGeometry` ke dalam `CharacterProfile` dan format serialisasi baru.
- Memperbaiki loop cedera agar cedera pada bagian tubuh yang diamputasi tidak digambar.
- Memperluas matriks hasil acuan dengan kasus cedera pada seluruh bagian tubuh, kombinasi cedera-amputasi, dan profil batas.
- Menetapkan kebijakan kenaikan versi untuk `NPC_API_VERSION`, `PROFILE_SCHEMA_VERSION`, dan `PROFILE_FORMAT_VERSION`.
- Mendokumentasikan 12 ambang diskret dan tabel kemonotonan per pasangan parameter-wilayah.

### Bukan-tujuan

- Mengubah rumus geometri yang sudah ada: lebar bahu, pinggang, dan seterusnya tetap sama.
- Menambah variasi anatomi wanita.
- Menambah pakaian atau ras fantasi.
- Mengubah format serialisasi lama V2: `SERIALIZER_FORMAT_VERSION` tetap 2.
- Mengubah kanvas penggambaran dari 32 × 48 piksel.

---

## 4. Ruang lingkup

### Termasuk

- Struktur data `BodyGeometry` dan fungsi `compute_geometry()` di Inti Pembuat Karakter.
- Perubahan `CharacterProfile` dengan skema versi 2 untuk menyimpan `BodyGeometry`.
- Format serialisasi V4 dengan `PROFILE_FORMAT_VERSION` bernilai 4 dengan migrasi otomatis dari V3.
- Struktur biner `CNpcProfile` yang diperluas dengan field `BodyGeometry`, dan kenaikan `NPC_API_VERSION` ke 4.
- Perluasan Godot: fungsi `profile_to_dict` dan `dict_to_profile` menyertakan `BodyGeometry`.
- Perbaikan loop cedera di `CharacterGenerator::render()` untuk memeriksa `is_amputated()`.
- Regenerasi `tests/golden/matriks-acuan.txt` dan perluasan cakupan.
- Target tes baru: `test_geometry.cpp` dan `test_transition.cpp`.
- Pembanding referensi sementara di target tes terpisah untuk verifikasi piksel demi piksel selama refaktor.
- Pembaruan skrip uji Godot untuk memverifikasi field `BodyGeometry` di kamus profil.

### Tidak termasuk

- Dokumen lama berawalan `BRD_` di `docs/`.
- Perubahan pada `SERIALIZER_FORMAT_VERSION` yang tetap 2 untuk `HumanAppearance` lama.
- Perubahan pada skrip Godot selain skrip uji asap (smoke test).

---

## 5. Kondisi saat ini

### 5.1. Geometri tubuh tersebar di dalam fungsi penggambaran

Fungsi `CharacterGenerator::render()` di `core/src/generator.cpp:101-523` menghitung 18 variabel geometri dari 4 parameter anatomi yaitu `height`, `body_mass`, `muscularity`, `posture` secara berurutan di dalam satu blok kode. Variabel ini tidak dikelompokkan dalam struktur data dan hanya hidup sebagai variabel lokal.

| Variabel | Baris | Rumus |
| --- | --- | --- |
| `total_height` | 113 | `36 + round(h * 6.0)` |
| `head_w` | 116 | Konstan 7 |
| `head_h` | 117 | Konstan 8 |
| `neck_w` | 118 | 3 jika `musc > 0.6 atau mass > 0.7`, selainnya 2 |
| `leg_len` | 120 | `13 + round(leg_ratio * 4.0)` |
| `torso_h` | 121-122 | `total_height - head_h - 2 - leg_len`, minimal 11 |
| `arm_len` | 123 | `torso_h + 2` |
| `shoulder_w` | 195 | `9 + round(musc * 4.0 + mass * 1.5)` |
| `waist_w` | 196-198 | `6 + round(mass * 4.5 + (1 - musc) * 1.0)`, dibatasi bentuk V |
| `hips_w` | 200 | `max(waist_w, 7 + round(mass * 3.5))` |
| `leg_gap` | 203 | 1 jika `musc > 0.6 atau mass < 0.35`, selainnya 0 |
| `thigh_thick` | 204-206 | 4, 3, atau 2 berdasarkan ambang `mass` dan `musc` |
| `arm_base_thick` | 344 | 3 jika `musc > 0.65 atau mass > 0.75`, selainnya 2 |

Sumber: `core/src/generator.cpp:113-206, 344`.

### 5.2. Loop cedera tidak memeriksa amputasi

Loop penggambaran cedera di `core/src/generator.cpp:486-520` mengulangi seluruh daftar cedera tanpa memeriksa `is_amputated()`. Cedera pada bagian tubuh yang sudah diamputasi tetap digambar. Bagian tubuh `LeftHand`, `RightHand`, `LeftFoot`, dan `RightFoot` tidak memiliki cabang sendiri dalam percabangan kondisi di baris 490-500, sehingga jatuh ke persegi panjang bawaan badan.

Sumber: `core/src/generator.cpp:486-520`, `core/src/generator.cpp:489`.

### 5.3. Cakupan matriks hasil acuan

Berkas `tests/golden/matriks-acuan.txt` berisi 7.360 baris dengan rincian:

| Kelompok | Jumlah baris | Kondisi uji |
| --- | --- | --- |
| `base` | 7.200 | Benih 1 sampai 100, 6 keadaan animasi, 4 arah, 3 nilai waktu |
| `inj_0` sampai `inj_5` | 60 | Satu jenis cedera per benih dari benih 1 sampai 10, hanya pada badan, keparahan 0,5, diam, arah selatan, waktu 0,0 |
| `amp_0` sampai `amp_9` | 100 | Satu amputasi per bagian tubuh dari benih 1 sampai 10, diam, arah selatan, waktu 0,0 |

Cakupan yang belum ada meliputi cedera pada kepala, lengan, tangan, kaki, atau telapak kaki; cedera pada arah selain selatan; kombinasi cedera dan amputasi; beberapa amputasi sekaligus; profil batas dengan tinggi 0,0 atau 1,0, massa 0,0 atau 1,0, dan otot 0,0 atau 1,0.

Sumber: `tests/golden/matriks-acuan.txt`, `tests/test_golden.cpp:44-59`, `tests/generate_fixtures.cpp`.

### 5.4. Kebijakan versi saat ini

| Pengenal | Nilai | Berkas | Perilaku pemeriksaan |
| --- | --- | --- | --- |
| `NPC_API_VERSION` | 3 | `core/include/npc/c_api.h:11` | `struct_size` diperiksa dengan kesetaraan ketat di `c_api.cpp:19, 62, 116` |
| `PROFILE_SCHEMA_VERSION` | 1 | `core/include/npc/profile.hpp:14` | Validator menolak setiap nilai yang tidak sama dengan 1 di `profile.hpp:208` |
| `SERIALIZER_FORMAT_VERSION` | 2 | `core/include/npc/serializer.hpp:13` | Pembaca memeriksa nomor versi format di `serializer.hpp:68` |
| `PROFILE_FORMAT_VERSION` | 3 | `core/include/npc/serializer.hpp:14` | Pembaca memeriksa nomor versi format di `serializer.hpp:235` |

Sumber: berkas pada kolom Berkas.

### 5.5. Tabel ambang diskret dalam fungsi penggambaran

Dua belas ambang diskret ditemukan di `core/src/generator.cpp`:

| No | Baris | Kondisi | Parameter yang terlibat | Efek pada geometri |
| --- | --- | --- | --- | --- |
| 1 | 118 | `musc > 0,6 atau mass > 0,7` | otot, massa | `neck_w` melompat dari 2 ke 3 piksel |
| 2 | 195 | `musc * 4,0 + mass * 1,5` | otot, massa | `shoulder_w` berubah kontinu |
| 3 | 196 | `mass * 4,5 + (1 - musc) * 1,0` | massa, otot | `waist_w` berubah kontinu |
| 4 | 197-198 | `musc > 0,5 dan mass < 0,6` | otot, massa | Bentuk V: `waist_w` dipotong ke `shoulder_w - 4` |
| 5 | 203 | `musc > 0,6 atau mass < 0,35` | otot, massa | `leg_gap` melompat dari 0 ke 1 piksel |
| 6 | 205 | `mass > 0,7 dan musc > 0,5` | massa, otot | `thigh_thick` melompat dari 3 ke 4 piksel |
| 7 | 206 | `mass < 0,3 dan musc < 0,3` | massa, otot | `thigh_thick` melompat dari 3 ke 2 piksel |
| 8 | 228, 262 | `musc > 0,4 atau mass > 0,4` | otot, massa | Betis mengembang 1 piksel |
| 9 | 297 | `mass > 0,65` | massa | Perut menggembung sampai 2,5 piksel |
| 10 | 344 | `musc > 0,65 atau mass > 0,75` | otot, massa | `arm_base_thick` melompat dari 2 ke 3 piksel |
| 11 | 319 | `musc > 0,45` | otot | Penanda dada muncul |
| 12 | 324 | `musc > 0,55` | otot | Garis tengah perut muncul |

Sumber: `core/src/generator.cpp:118-344`.

### 5.6. Status `CharacterProfile` dan domain

Struktur `CharacterProfile` di `core/include/npc/profile.hpp:141-160` berisi empat domain: `GenerationProfile`, `AnatomyProfile`, `AppearanceProfile`, dan `PhysicalConditionProfile`. Belum ada domain geometri tubuh.

Sumber: `core/include/npc/profile.hpp:141-160`.

### 5.7. Status `CNpcProfile` dan ukuran struct

Struktur `CNpcProfile` di `core/include/npc/c_api.h:70-106` berisi field generasi, anatomi, penampilan, dan masker amputasi. Ukurannya diperiksa dengan kesetaraan ketat melalui `struct_size != sizeof(CNpcProfile)` di tiga tempat pada `c_api.cpp`.

Sumber: `core/include/npc/c_api.h:70-106`, `core/src/c_api.cpp:19, 62, 116`.

---

## 6. Arsitektur sasaran

Empat lapisan proyek:

1. **Inti Pembuat Karakter** (`core/`): kode C++20 yang memiliki seluruh logika geometri, validasi, serialisasi, dan penggambaran.
2. **Jembatan C** (`core/include/npc/c_api.h`, `core/src/c_api.cpp`): antarmuka berbahasa C di atas inti.
3. **Perluasan Godot** (`extension/`): modul GDExtension yang meneruskan panggilan ke inti.
4. **Skrip Godot** (`godot/scripts/`): kode GDScript untuk antarmuka pratinjau.

Perubahan arsitektur pada v0.3.3:

- Inti menyediakan struktur baru `BodyGeometry` dan fungsi `compute_geometry()`.
- `CharacterProfile` memiliki domain baru `BodyGeometry` dengan skema versi 2.
- `CharacterGenerator::render()` menerima geometri yang sudah dihitung.
- Loop cedera memeriksa `is_amputated()` menggunakan geometri yang sama.

Alur data penggambaran dimulai dari `AnatomyProfile` menuju `compute_geometry()` menghasilkan `BodyGeometry`. `CharacterProfile` menyimpan `BodyGeometry` lalu meneruskannya ke `CharacterGenerator::render()`. Fungsi ini kemudian memanggil penggambaran torso dan penempelan efek cedera untuk menghasilkan penyangga piksel berukuran 32 × 48 piksel.

---

## 7. Kebutuhan fungsional

### Fungsional-001: Struktur geometri tubuh bersama

| Kolom | Isi |
| --- | --- |
| Pengenal | `Fungsional-001` |
| Judul | Struktur geometri tubuh bersama |
| Pernyataan | Inti harus menyediakan struktur `BodyGeometry` yang menyimpan 7 nilai dasar bilangan bulat: `shoulder_w`, `waist_w`, `hips_w`, `thigh_thick`, `neck_w`, `arm_base_thick`, dan `leg_gap`. |
| Alasan | Tahap penggambaran dan tahap cedera memerlukan nilai geometri yang sama. Menghitung sekali dan menyimpannya mencegah ketidakkonsistenan. |
| Cara verifikasi | Tes membuat `BodyGeometry` dari `AnatomyProfile` bawaan dan memeriksa bahwa ketujuh nilai cocok dengan keluaran rumus yang ada di `generator.cpp:118-344`. |
| Tahap | 1 |
| Bergantung pada | Tidak ada |

### Fungsional-002: Fungsi penghitung geometri tubuh

| Kolom | Isi |
| --- | --- |
| Pengenal | `Fungsional-002` |
| Judul | Fungsi penghitung geometri tubuh |
| Pernyataan | Inti harus menyediakan fungsi `compute_geometry(const AnatomyProfile&)` yang mengembalikan `BodyGeometry` dengan rumus yang identik dengan kode saat ini di `generator.cpp:113-206, 344`. |
| Alasan | Fungsi ini menjadi satu tempat rumus geometri dihitung agar perubahan rumus di masa depan terpusat. |
| Cara verifikasi | Tes memanggil `compute_geometry()` untuk 100 benih acak dan membandingkan hasilnya dengan nilai dari rumus lama. Seluruh 7 nilai harus identik. |
| Tahap | 1 |
| Bergantung pada | `Fungsional-001` |

### Fungsional-003: Geometri tubuh di profil karakter

| Kolom | Isi |
| --- | --- |
| Pengenal | `Fungsional-003` |
| Judul | Geometri tubuh tersimpan di profil |
| Pernyataan | `CharacterProfile` harus menyimpan `BodyGeometry` sebagai domain baru dengan `PROFILE_SCHEMA_VERSION` bernilai 2. |
| Alasan | Menyimpan geometri ke profil memungkinkan penentuan deterministik tanpa menghitung ulang dan menjadi bagian dari genom karakter. |
| Cara verifikasi | Tes membuat profil bawaan, memeriksa bahwa `schema_version` bernilai 2, dan memeriksa bahwa `body_geometry.shoulder_w` cocok dengan `compute_geometry()` dari `anatomy` profil tersebut. |
| Tahap | 2 |
| Bergantung pada | `Fungsional-001`, `Fungsional-002` |

### Fungsional-004: Serialisasi format V4 menyimpan geometri

| Kolom | Isi |
| --- | --- |
| Pengenal | `Fungsional-004` |
| Judul | Serialisasi format V4 menyimpan geometri |
| Pernyataan | `Serializer::serialize_profile()` harus menyimpan 7 nilai `BodyGeometry` setelah masker amputasi dengan `PROFILE_FORMAT_VERSION` bernilai 4. |
| Alasan | Format V3 tidak menyimpan geometri sehingga format baru diperlukan agar profil yang disimpan memuat geometri lengkap. |
| Cara verifikasi | Tes melakukan serialisasi dan deserialisasi 100 profil yang memuat geometri. Seluruh 7 nilai geometri harus identik setelah perjalanan bolak-balik. |
| Tahap | 2 |
| Bergantung pada | `Fungsional-003` |

### Fungsional-005: Migrasi otomatis data lama

| Kolom | Isi |
| --- | --- |
| Pengenal | `Fungsional-005` |
| Judul | Migrasi otomatis data lama |
| Pernyataan | `Serializer::deserialize_profile()` harus dapat membaca format V3 skema 1 dan format V2, lalu menghasilkan `CharacterProfile` skema 2 dengan `BodyGeometry` yang dihitung dari `AnatomyProfile` memakai `compute_geometry()`. |
| Alasan | Profil yang disimpan oleh versi sebelumnya harus tetap dapat dimuat tanpa kehilangan data. |
| Cara verifikasi | Tes membuat teks serialisasi format V3 dan V2, memuatnya dengan deserializer baru, dan memeriksa bahwa geometri yang dihasilkan cocok dengan `compute_geometry()` dari anatomi. |
| Tahap | 2 |
| Bergantung pada | `Fungsional-004` |

### Fungsional-006: Struktur biner diperluas dengan geometri

| Kolom | Isi |
| --- | --- |
| Pengenal | `Fungsional-006` |
| Judul | Struktur biner diperluas dengan geometri |
| Pernyataan | `CNpcProfile` harus mendapat 7 field bilangan bulat bertanda 16 bit untuk `BodyGeometry` setelah `amputations_mask` dengan `NPC_API_VERSION` bernilai 4. |
| Alasan | Pemanggil Jembatan C memerlukan akses ke geometri tubuh untuk keperluan antarmuka pengguna. Kenaikan versi mencegah pemanggil lama membaca field yang tidak ada. |
| Cara verifikasi | Tes memanggil `npc_profile_create_default()` dengan ukuran struct baru dan memeriksa bahwa 7 field geometri terisi. Tes memanggil dengan ukuran struct lama dan memeriksa pengembalian kode galat `NPC_ERR_VERSION_MISMATCH`. |
| Tahap | 2 |
| Bergantung pada | `Fungsional-003` |

### Fungsional-007: Penggambaran memakai geometri yang tersimpan

| Kolom | Isi |
| --- | --- |
| Pengenal | `Fungsional-007` |
| Judul | Penggambaran memakai geometri tersimpan |
| Pernyataan | `CharacterGenerator::render()` harus memakai nilai dari `BodyGeometry` yang diterimanya tanpa menghitung ulang dari parameter anatomi. |
| Alasan | Menghilangkan duplikasi rumus dan memastikan tahap penggambaran dan tahap cedera memakai nilai yang sama. |
| Cara verifikasi | Tes membuat profil dengan geometri bawaan, menggambarnya, lalu membandingkan penyangga piksel dengan keluaran penggambaran lama untuk 100 benih acak. Sidik digital penyangga piksel harus identik. |
| Tahap | 2 |
| Bergantung pada | `Fungsional-001`, `Fungsional-002` |

### Fungsional-008: Cedera pada amputasi dilewati

| Kolom | Isi |
| --- | --- |
| Pengenal | `Fungsional-008` |
| Judul | Cedera pada amputasi dilewati |
| Pernyataan | Loop cedera di `CharacterGenerator::render()` harus memeriksa status `is_amputated(inj.body_part)` dan melewati cedera pada bagian tubuh yang sudah diamputasi. |
| Alasan | Menggambar cedera pada bagian tubuh yang hilang menghasilkan anomali visual yang mustahil secara fisik. |
| Cara verifikasi | Tes membuat profil dengan amputasi lengan kiri dan cedera pada lengan kiri, menggambarnya, dan memeriksa bahwa piksel di wilayah lengan kiri tidak berubah dibandingkan profil tanpa cedera. Perubahan visual ditandai. |
| Tahap | 3 |
| Bergantung pada | `Fungsional-007` |

### Fungsional-009: Geometri cedera tangan dan kaki

| Kolom | Isi |
| --- | --- |
| Pengenal | `Fungsional-009` |
| Judul | Geometri cedera tangan dan kaki |
| Pernyataan | Loop cedera harus menyediakan batas persegi panjang untuk `LeftHand`, `RightHand`, `LeftFoot`, dan `RightFoot` yang sesuai dengan posisi penggambaran anggota tubuh tersebut. |
| Alasan | Saat ini keempat bagian tubuh itu jatuh ke persegi panjang bawaan badan sehingga cedera tangan muncul di wilayah perut. |
| Cara verifikasi | Tes membuat profil dengan cedera pada tangan kiri, menggambarnya, dan memeriksa bahwa piksel cedera berada di dalam wilayah tangan kiri dan bukan di badan. Perubahan visual ditandai. |
| Tahap | 3 |
| Bergantung pada | `Fungsional-007`, `Fungsional-008` |

### Fungsional-010: Pembanding referensi sementara

| Kolom | Isi |
| --- | --- |
| Pengenal | `Fungsional-010` |
| Judul | Pembanding referensi sementara |
| Pernyataan | Target tes terpisah harus menyimpan salinan fungsi penggambaran dari commit sebelum refaktor dan membandingkan piksel demi piksel dengan fungsi baru untuk profil batas, seluruh bagian tubuh, cedera, amputasi, dan seluruh arah. |
| Alasan | Pembanding referensi memastikan refaktor kode tidak mengubah keluaran piksel secara tidak sengaja pada kasus di luar perbaikan cedera. |
| Cara verifikasi | Target tes `test_refactor_parity` menghasilkan 0 selisih piksel pada kasus yang tidak terpengaruh `Fungsional-008` dan `Fungsional-009`. |
| Tahap | 3 |
| Bergantung pada | `Fungsional-007` |

### Fungsional-011: Perluasan matriks hasil acuan

| Kolom | Isi |
| --- | --- |
| Pengenal | `Fungsional-011` |
| Judul | Perluasan matriks hasil acuan |
| Pernyataan | Berkas `tests/golden/matriks-acuan.txt` harus diregenerasi dan diperluas mencakup cedera pada seluruh 10 bagian tubuh dalam 4 arah, kombinasi cedera dan amputasi, serta 6 profil batas ekstrem. |
| Alasan | Cakupan 7.360 baris sebelumnya tidak mencakup cedera lintas bagian tubuh, kombinasi cedera-amputasi, atau profil batas. |
| Cara verifikasi | Eksekusi `test_golden.cpp` memverifikasi seluruh baris baru tanpa ketidakcocokan dengan jumlah baris melampaui 7.800 baris. |
| Tahap | 4 |
| Bergantung pada | `Fungsional-008`, `Fungsional-009` |

### Fungsional-012: Perluasan Godot menyertakan geometri

| Kolom | Isi |
| --- | --- |
| Pengenal | `Fungsional-012` |
| Judul | Perluasan Godot menyertakan geometri |
| Pernyataan | Fungsi `profile_to_dict()` dan `dict_to_profile()` di `extension/src/npc_generator.cpp` harus menyertakan subdomain `body_geometry` dalam kamus profil. |
| Alasan | Skrip Godot memerlukan akses ke geometri untuk keperluan visualisasi antarmuka pengguna. |
| Cara verifikasi | Skrip uji Godot memanggil `create_default_profile()` dan memeriksa keberadaan kunci `body_geometry` dengan nilai `shoulder_w` lebih besar dari 0. |
| Tahap | 2 |
| Bergantung pada | `Fungsional-003` |

### Fungsional-013: Tabel kemonotonan dan tes ambang

| Kolom | Isi |
| --- | --- |
| Pengenal | `Fungsional-013` |
| Judul | Tabel kemonotonan dan tes ambang |
| Pernyataan | Target tes `test_transition.cpp` harus memverifikasi 12 ambang diskret dengan menguji nilai sebelum, pada, dan sesudah setiap ambang, serta memeriksa pasangan yang dirancang monoton sesuai tabel. |
| Alasan | Kemonotonan tidak berlaku umum untuk semua parameter dan harus diuji per pasangan wilayah. |
| Cara verifikasi | Target tes `test_transition` menghasilkan 0 kegagalan uji. |
| Tahap | 5 |
| Bergantung pada | `Fungsional-002` |

Tabel kemonotonan yang dirancang monoton:

| Pasangan | Wilayah parameter | Arah monoton |
| --- | --- | --- |
| (`muscularity`, `shoulder_w`) | 0,0 sampai 1,0 untuk semua nilai massa tubuh | Naik |
| (`body_mass`, `waist_w`) | 0,0 sampai 1,0 untuk semua nilai otot | Naik |
| (`body_mass`, `hips_w`) | 0,0 sampai 1,0 | Naik |
| (`body_mass`, `shoulder_w`) | 0,0 sampai 1,0 untuk semua nilai otot | Naik |
| (`muscularity`, `waist_w`) | 0,0 sampai 1,0 dengan massa tubuh konstan | Turun |

Pasangan yang tidak monoton:

| Pasangan | Alasan |
| --- | --- |
| (`body_mass`, lebar torso baris tengah) | Suku `mass > 0.65` di baris 297 menambah sampai 2,5 piksel di wilayah perut |
| (`muscularity`, `thigh_thick`) | Dipengaruhi dua ambang diskret terpisah yaitu ambang tebal 4 dan ambang kurus 2 |

---

## 8. Kebutuhan nonfungsional

### Nonfungsional-001: Determinisme lintas platform

| Kolom | Isi |
| --- | --- |
| Pengenal | `Nonfungsional-001` |
| Judul | Determinisme lintas platform |
| Pernyataan | Fungsi `compute_geometry()` harus menghasilkan nilai yang identik pada sistem operasi Linux, Windows, dan macOS untuk masukan yang sama. |
| Alasan | Geometri yang disimpan ke profil menjadi bagian dari genom karakter sehingga nilai yang berbeda antarplatform merusak reproduksi visual. |
| Cara verifikasi | Pipa integrasi berkelanjutan menjalankan tes `test_geometry` pada runner Linux dan Windows dengan seluruh 7 nilai identik. |
| Tahap | 1 |
| Bergantung pada | `Fungsional-002` |

### Nonfungsional-002: Kinerja penghitungan geometri

| Kolom | Isi |
| --- | --- |
| Pengenal | `Nonfungsional-002` |
| Judul | Kinerja penghitungan geometri |
| Pernyataan | Fungsi `compute_geometry()` harus selesai dalam waktu di bawah 0,01 milidetik per panggilan pada prosesor dengan kecepatan 3 gigahertz. |
| Alasan | Fungsi ini dipanggil setiap kali profil karakter dibuat sehingga beban komputasi harus minimal. |
| Cara verifikasi | Tolok ukur kinerja memanggil `compute_geometry()` sebanyak 100.000 kali dan menghitung rata-rata waktu eksekusi di bawah 0,01 milidetik. |
| Tahap | 1 |
| Bergantung pada | `Fungsional-002` |

### Nonfungsional-003: Kompatibilitas mundur serialisasi

| Kolom | Isi |
| --- | --- |
| Pengenal | `Nonfungsional-003` |
| Judul | Kompatibilitas mundur serialisasi |
| Pernyataan | Fungsi deserialisasi harus dapat membaca format V2, V3, dan V4 tanpa kehilangan data, serta menghitung otomatis geometri untuk format lama. |
| Alasan | Berkas simpanan karakter dari versi rilis sebelumnya harus dapat dimuat tanpa kendala. |
| Cara verifikasi | Pengujian memuat sampel data format V2 dan V3, memvalidasi keberhasilan pemuatan, dan memeriksa kecocokan geometri hasil hitung otomatis. |
| Tahap | 2 |
| Bergantung pada | `Fungsional-005` |

---

## 9. Antarmuka lintas lapisan

### 9.1. `compute_geometry()`

| Kolom | Isi |
| --- | --- |
| Nama | `compute_geometry` |
| Arah | Inti Pembuat Karakter ke modul pemanggil internal |
| Masukan | `const AnatomyProfile&` berupa 8 nilai pecahan dalam rentang 0,0 sampai 1,0 |
| Keluaran | `BodyGeometry` berisi 7 nilai bilangan bulat dengan rentang piksel terdefinisi |
| Penanganan galat | Masukan divalidasi terlebih dahulu oleh `ProfileValidator` sebelum dihitung |
| Pemilik memori | Pemanggil memiliki salinan struktur `BodyGeometry` yang dikembalikan |
| Versi | Mengikuti skema profil versi 2 |
| Kompatibilitas | Fungsi baru internal C++20 |

### 9.2. `CNpcProfile` (versi 4)

| Kolom | Isi |
| --- | --- |
| Nama | `CNpcProfile` |
| Arah | Jembatan C ke aplikasi pemanggil |
| Masukan | Pemanggil mengisi `struct_size = sizeof(CNpcProfile)` beserta parameter profil |
| Keluaran | Field geometri terisi oleh `npc_profile_create_default()` |
| Penanganan galat | Mengembalikan kode galat `NPC_ERR_VERSION_MISMATCH` jika `struct_size` tidak cocok |
| Pemilik memori | Pemanggil mengalokasikan dan membebaskan struktur memori `CNpcProfile` |
| Versi | `NPC_API_VERSION` bernilai 4 |
| Kompatibilitas | Pemanggil lama dengan ukuran versi 3 ditolak secara aman tanpa menimbulkan kegagalan memori |

### 9.3. Kamus profil Perluasan Godot

| Kolom | Isi |
| --- | --- |
| Nama | Subdomain `body_geometry` dalam kamus profil |
| Arah | Perluasan Godot ke Skrip Godot |
| Masukan | Dihasilkan dari instansiasi profil karakter native |
| Keluaran | Kamus berisi 7 kunci bilangan bulat untuk nilai geometri tubuh |
| Penanganan galat | Jika kunci tidak ada saat konversi ke profil native, geometri dihitung otomatis dari anatomi |
| Pemilik memori | Dikelola oleh sistem manajemen memori Godot Variant |
| Versi | Mengikuti skema profil versi 2 |
| Kompatibilitas | Skrip lama yang belum membaca kunci geometri tetap berjalan normal |

---

## 10. Dampak dan risiko

| Pengenal | Risiko | Dampak | Cara mengurangi |
| --- | --- | --- | --- |
| `Risiko-001` | Variasi pembulatan fungsi pecahan antar-kompiler menghasilkan geometri berbeda | Sidik digital profil tidak cocok antar-platform | `compute_geometry()` hanya menggunakan `std::round()` pada tipe pecahan presisi tunggal dengan pengujian otomatis di Linux dan Windows. |
| `Risiko-002` | Regenerasi matriks hasil acuan menghilangkan riwayat regresi visual versi lama | Kerusakan visual baru berpotensi luput dari deteksi | Menggunakan pembanding referensi sementara untuk memverifikasi piksel demi piksel sebelum memperbarui berkas matriks acuan. |
| `Risiko-003` | Pemanggil lama Jembatan C ditolak karena ukuran struktur biner berubah | Aplikasi luar yang belum diperbarui gagal melakukan inisialisasi | Perilaku ini disengaja untuk menjaga integritas data dan didokumentasikan dalam panduan rilis. |
| `Risiko-004` | Perbaikan pengabaian cedera pada amputasi mengubah piksel keluaran | Matriks hasil acuan lama tidak cocok | Memperbarui matriks acuan secara terkontrol pada tahap 4 setelah verifikasi lulus. Ditandai sebagai perubahan visual. |
| `Risiko-005` | Penyesuaian koordinat cedera tangan dan kaki memindahkan lokasi piksel efek | Pergeseran letak titik cedera pada anggota badan terkait | Menambahkan skenario uji khusus untuk memverifikasi penempatan koordinat baru. Ditandai sebagai perubahan visual. |

---

## 11. Rencana tahap

### Tahap 1: Struktur `BodyGeometry` dan `compute_geometry()`

- **Keluaran:** Berkas `core/include/npc/body_geometry.hpp` berisi definisi `BodyGeometry` dan implementasi `compute_geometry()`.
- **Ketergantungan:** Tidak ada.
- **Kebutuhan:** `Fungsional-001`, `Fungsional-002`, `Nonfungsional-001`, `Nonfungsional-002`.
- **Berkas terdampak:**

| Berkas | Tindakan |
| --- | --- |
| `core/include/npc/body_geometry.hpp` | Dibuat |
| `tests/test_geometry.cpp` | Dibuat |
| `CMakeLists.txt` | Diubah untuk menambahkan target tes baru |

### Tahap 2: Integrasi ke profil, serialisasi, dan lintas lapisan

- **Keluaran:** Profil karakter skema versi 2, serialisasi format V4, struktur biner versi 4, dan dukungan geometri pada modul Godot.
- **Ketergantungan:** Tahap 1.
- **Kebutuhan:** `Fungsional-003`, `Fungsional-004`, `Fungsional-005`, `Fungsional-006`, `Fungsional-007`, `Fungsional-012`, `Nonfungsional-003`.
- **Berkas terdampak:**

| Berkas | Tindakan |
| --- | --- |
| `core/include/npc/profile.hpp` | Diubah |
| `core/include/npc/serializer.hpp` | Diubah |
| `core/include/npc/c_api.h` | Diubah |
| `core/src/c_api.cpp` | Diubah |
| `core/src/generator.cpp` | Diubah |
| `core/include/npc/generator.hpp` | Diubah |
| `extension/src/npc_generator.cpp` | Diubah |
| `tests/test_profile.cpp` | Diubah |
| `tests/test_serializer.cpp` | Diubah |
| `godot/scripts/smoke_test_profile.gd` | Diubah |

### Tahap 3: Perbaikan cedera-amputasi dan pembanding referensi

- **Keluaran:** Loop cedera mengabaikan anggota tubuh yang diamputasi, penempatan cedera tangan dan kaki memiliki batas tersendiri, dan verifikasi pembanding referensi selesai.
- **Ketergantungan:** Tahap 2.
- **Kebutuhan:** `Fungsional-008`, `Fungsional-009`, `Fungsional-010`.
- **Berkas terdampak:**

| Berkas | Tindakan |
| --- | --- |
| `core/src/generator.cpp` | Diubah |
| `tests/test_refactor_parity.cpp` | Dibuat untuk pengujian sementara |
| `CMakeLists.txt` | Diubah |

### Tahap 4: Perluasan dan regenerasi matriks hasil acuan

- **Keluaran:** Berkas acuan `tests/golden/matriks-acuan.txt` diregenerasi dengan jumlah lebih dari 7.800 baris kombinasi uji.
- **Ketergantungan:** Tahap 3.
- **Kebutuhan:** `Fungsional-011`.
- **Berkas terdampak:**

| Berkas | Tindakan |
| --- | --- |
| `tests/generate_fixtures.cpp` | Diubah |
| `tests/golden/matriks-acuan.txt` | Diubah melalui proses regenerasi |
| `tests/test_golden.cpp` | Diubah |

### Tahap 5: Tes ambang dan kemonotonan

- **Keluaran:** Target pengujian `test_transition.cpp` yang memvalidasi 12 ambang batas diskret dan kemonotonan parameter.
- **Ketergantungan:** Tahap 1.
- **Kebutuhan:** `Fungsional-013`.
- **Berkas terdampak:**

| Berkas | Tindakan |
| --- | --- |
| `tests/test_transition.cpp` | Dibuat |
| `CMakeLists.txt` | Diubah |

---

## 12. Kriteria penerimaan

| Pengenal | Pernyataan |
| --- | --- |
| `Penerimaan-001` | Fungsi `compute_geometry()` menghasilkan 7 nilai dasar yang identik dengan rumus lama pada 100 benih acak. |
| `Penerimaan-002` | Struktur `CharacterProfile` menggunakan skema versi 2 dan menyimpan nilai `BodyGeometry`. |
| `Penerimaan-003` | Serialisasi format V4 berhasil melakukan perjalanan bolak-balik tanpa kehilangan data pada 100 profil acak. |
| `Penerimaan-004` | Deserialisasi data format V3 dan V2 berhasil membaca data lama dan menghasilkan geometri yang valid. |
| `Penerimaan-005` | Nilai `NPC_API_VERSION` adalah 4 dan pemanggilan dengan ukuran struktur versi lama ditolak dengan aman. |
| `Penerimaan-006` | Sidik digital penyangga piksel identik antara fungsi baru dan fungsi lama untuk kasus tanpa kombinasi cedera-amputasi. |
| `Penerimaan-007` | Cedera pada bagian tubuh yang diamputasi tidak menghasilkan piksel efek cedera pada kanvas keluaran. |
| `Penerimaan-008` | Titik efek cedera pada tangan kiri atau kanan digambar di wilayah tangan dan tidak jatuh ke badan. |
| `Penerimaan-009` | Matriks acuan baru memuat lebih dari 7.800 baris kombinasi dan seluruhnya lulus verifikasi 100 persen. |
| `Penerimaan-010` | Seluruh 12 ambang diskret terverifikasi perilakunya melalui pengujian transisi. |
| `Penerimaan-011` | Pengujian kemonotonan memverifikasi 5 pasangan parameter monoton tanpa kegagalan. |
| `Penerimaan-012` | Seluruh target uji pada CTest lulus tanpa ada kegagalan. |
| `Penerimaan-013` | Skrip uji Godot berhasil memvalidasi keberadaan kunci `body_geometry` dalam kamus profil karakter. |

---

## 13. Pertanyaan terbuka

| No | Pertanyaan | Dampak |
| --- | --- | --- |
| 1 | Apakah `BodyGeometry` perlu menyimpan nilai posisi seperti `head_x` dan `torso_y`? | Menyimpan koordinat posisi membuat ukuran struktur lebih besar dan nilai posisi berubah tergantung arah hadap serta animasi. |
| 2 | Kapan target tes pembanding referensi sementara dihapus dari repositori? | Menentukan apakah berkas pembanding disertakan saat rilis publik atau dihapus sebelum penggabungan cabang master. |
| 3 | Apakah perbaikan letak cedera tangan dan kaki memengaruhi interpretasi data pada konten dunia lama? | Menentukan perlunya panduan penyesuaian bagi modul simulasi dunia di luar modul pembuat karakter. |
| 4 | Berapa angka pasti jumlah baris matriks acuan setelah seluruh kombinasi baru ditambahkan? | Menentukan angka pembanding batas minimal pada berkas pengujian hasil acuan. |
| 5 | Apakah `total_height` dan `torso_h` perlu dimasukkan ke dalam `BodyGeometry`? | Kedua nilai tersebut merupakan dimensi ukuran tubuh yang berpotensi dipakai oleh modul pakaian di masa depan. |

---

## 14. Glosarium

| Istilah | Arti |
| --- | --- |
| Karakter | Makhluk di dunia yang digerakkan simulasi, bukan pemain. |
| Geometri tubuh | Kumpulan 7 nilai dasar bilangan bulat yang menentukan ukuran bagian tubuh karakter dalam satuan piksel. |
| Penampilan | Nilai bentuk tubuh dan warna yang dihitung dari genom karakter. |
| Penggambaran | Proses mengubah penampilan, kondisi fisik, dan perintah animasi menjadi susunan piksel warna. |
| Penyangga piksel | Larik warna berukuran 32 × 48 piksel hasil proses penggambaran. |
| Jembatan C | Lapisan antarmuka berbahasa C di atas inti logika karakter. |
| Perluasan Godot | Mekanisme pemuatan modul kode asli C++ ke dalam mesin permainan Godot. |
| Hasil acuan | Data keluaran tersimpan yang dibandingkan dengan keluaran baru untuk mendeteksi perubahan visual. |
| Sidik digital | Nilai ringkas hasil algoritma pencacah yang mewakili keutuhan isi data. |
| Pembanding referensi sementara | Salinan fungsi sebelum refaktor yang digunakan untuk memverifikasi kesetaraan piksel selama masa transisi. |
| Ambang diskret | Kondisi percabangan logika yang menyebabkan ukuran geometri melompat dan tidak berubah secara kontinu. |
| Kemonotonan | Karakteristik nilai yang selalu meningkat atau selalu menurun seiring pertambahan parameter masukan. |
| Perjalanan bolak-balik | Siklus penyimpanan data ke format teks atau biner kemudian dibaca kembali hingga menghasilkan struktur data yang identik. |
| Genom | Kumpulan data genetik karakter yang dikelola oleh inti simulasi dunia. |

---

## 15. Riwayat revisi

| Tanggal | Versi | Perubahan | Alasan |
| --- | --- | --- | --- |
| 2026-10-25 | 1 | Dokumen awal | Mengakomodasi koreksi rencana v0.3.3 untuk memisahkan geometri tubuh, memperbaiki penanganan cedera pada amputasi, memperluas cakupan acuan, dan menata kebijakan versi. |
