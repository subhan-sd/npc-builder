# Refaktor Geometri Tubuh, Kontrak Kompatibilitas, Kebijakan Versi, dan Hasil Acuan

Proyek: Living World · Modul: Pembuat Karakter (repositori `npc-builder`) · Tanggal: 2026-10-25 · Versi: 2

---

## 1. Ringkasan

Fungsi penggambaran karakter di `core/src/generator.cpp` menghitung geometri tubuh seperti lebar bahu, pinggang, dan pinggul secara tersebar di dalam satu fungsi sepanjang 423 baris. Kondisi ini menyulitkan penambahan variasi proporsi tubuh dan busana modular tanpa memicu regresi visual. Matriks hasil acuan sebelumnya hanya mencakup 7.360 kombinasi dengan pengujian cedera yang terbatas pada bagian tubuh badan dan arah hadap selatan. Tiga nomor versi lintas lapisan belum memiliki batas tanggung jawab serta kontrak kompatibilitas yang terdefinisi dengan tegas.

Solusinya adalah merestrukturisasi geometri tubuh ke dalam struktur data bersama `BodyGeometry` yang dihitung secara deterministik oleh fungsi murni `compute_geometry()`, mendokumentasikan batas kontrak kompatibilitas terhadap implementasi dasar secara eksplisit, memperluas matriks hasil acuan menjadi 7.884 kombinasi, dan menetapkan kebijakan versi lintas lapisan secara terkoordinasi. Hasil yang diharapkan berupa pemisahan domain geometri yang bersih, perlindungan menyeluruh terhadap regresi visual, serta aturan migrasi data lama yang aman dari pemotongan tipe data.

---

## 2. Latar belakang

Versi `v0.3.2` menyelesaikan pengerasan validasi profil karakter, serialisasi, dan keandalan integrasi berkelanjutan pada commit `a7c90dd`. Dokumen evaluasi teknis pada `docs/V0.3/Tanggapan atas Koreksi Rencana v0.3.3 Kompatibilitas, Cakupan Hasil Acuan, Versi, dan Kriteria Transisi.md` mengidentifikasi bahwa rencana pemisahan geometri tubuh memerlukan penegasan kontrak kompatibilitas:

1. Kompatibilitas perilaku cedera dan amputasi tidak dapat diasumsikan terjadi secara otomatis dari struktur geometri.
2. Matriks hasil acuan lama memiliki celah pengujian pada cedera di luar bagian tubuh badan, arah hadap non-selatan, dan kombinasi kondisi fisik.
3. Empat nomor versi sistem memiliki tingkat kekakuan biner yang berbeda dan tidak boleh dinaikkan tanpa analisis pemanggil.
4. Kriteria kemonotonan geometri harus dibatasi pada pasangan parameter yang memang secara matematis dirancang monoton.
5. Struktur `BodyGeometry` harus berukuran ringkas dan hanya menyimpan nilai dasar lintas tahap tanpa menduplikasi nilai turunan per baris piksel.

Pembaruan v0.3.3 menjadi landasan bagi simulasi Living World sebelum penambahan proporsi anatomi wanita, variasi wajah lanjutan, dan lapisan pakaian dinamis pada rilis berikutnya.

---

## 3. Tujuan dan bukan-tujuan

### Tujuan

- Mengekstrak 7 nilai dasar geometri tubuh ke dalam struktur data `BodyGeometry` melalui fungsi deterministik `compute_geometry()`.
- Menjadikan parameter anatomi sebagai sumber kebenaran tunggal geometri dengan aturan sinkronisasi yang jelas di seluruh lapisan.
- Mempertahankan 7.360 hasil acuan dasar lama secara identik tanpa pergeseran satu piksel pun.
- Memperluas cakupan hasil acuan menjadi 7.884 kombinasi untuk memverifikasi seluruh 10 bagian tubuh dalam 4 arah hadap, kombinasi kondisi fisik, dan nilai parameter batas.
- Memisahkan perbaikan perilaku visual yang disengaja ke dalam kategori tersendiri dengan kriteria pengujian mandiri.
- Menetapkan kebijakan versi lintas lapisan untuk antarmuka pemrograman C, skema profil, dan format serialisasi beserta perlindungan migrasi data lama dari pemotongan tipe data.
- Menguji 12 ambang batas diskretisasi geometri dan memverifikasi tabel kemonotonan parameter.

### Bukan-tujuan

- Mengubah rumus dasar lebar bahu, pinggang, pinggul, ketebalan paha, dan ketebalan lengan yang sudah ada.
- Menambahkan variasi anatomi tubuh wanita pada rilis ini.
- Menambahkan sistem pakaian modular atau ras non-manusia.
- Mengubah format serialisasi lama format versi 2 untuk struktur penampilan manusia lama.
- Mengubah dimensi kanvas penggambaran dari standar 32 × 48 piksel.

---

## 4. Ruang lingkup

### Termasuk

- Struktur data `BodyGeometry` dan fungsi murni `compute_geometry()` pada berkas `core/include/npc/body_geometry.hpp`.
- Integrasi `BodyGeometry` ke dalam `CharacterProfile` dengan nomor skema profil versi 2 pada `core/include/npc/profile.hpp`.
- Format serialisasi profil versi 4 dengan kemampuan membaca data lama versi 3 dan versi 2 pada `core/include/npc/serializer.hpp`.
- Perluasan struktur biner `CNpcProfile` dengan 7 field geometri dan kenaikan versi antarmuka C ke versi 4 pada `core/include/npc/c_api.h` dan `core/src/c_api.cpp`.
- Integrasi subdomain `body_geometry` pada lapisan Perluasan Godot di `extension/src/npc_generator.cpp`.
- Pengujian geometri, transisi ambang batas, serialisasi, antarmuka C, dan hasil acuan pada folder `tests/`.
- Penegasan kontrak kompatibilitas dan pemisahan kategori pengujian.

### Tidak termasuk

- Pengubahan dokumen lama berawalan `BRD_` pada folder `docs/`.
- Perubahan pada fungsi serialisasi format versi 2 untuk struktur penampilan manusia lama.
- Perubahan logika permainan pada skrip pratinjau Godot di luar skrip uji asap.

---

## 5. Kondisi saat ini dan audit kode

Audit kode dilakukan terhadap commit `ea0ca77bf27048d57398b9ae27c1f3afc5c2bd81` dibandingkan dengan commit acuan rilis sebelumnya `a7c90dd72da34f404ca87d1dfcbb3e18cfdf0a1f`.

### 5.1. Geometri tubuh dan ekstraksi fungsi

Pada implementasi acuan `a7c90dd`, perhitungan geometri tersebar langsung di dalam `core/src/generator.cpp:113-206, 344` sebagai variabel lokal. Pada commit `ea0ca77`, 7 nilai dasar telah diekstrak ke dalam struktur `BodyGeometry` pada `core/include/npc/body_geometry.hpp`:

| Variabel | Berkas dan baris asal | Rumus matematis | Rentang sah |
| --- | --- | --- | --- |
| `neck_w` | `core/src/generator.cpp:118` | `(musc > 0.6 atau mass > 0.7) ? 3 : 2` | [2, 3] piksel |
| `shoulder_w` | `core/src/generator.cpp:195` | `9 + round(musc * 4.0 + mass * 1.5)` | [9, 18] piksel |
| `waist_w` | `core/src/generator.cpp:196-198` | `6 + round(mass * 4.5 + (1 - musc) * 1.0)`, dijepit `shoulder_w - 4` jika `musc > 0.5` dan `mass < 0.6` | [6, 16] piksel |
| `hips_w` | `core/src/generator.cpp:200` | `max(waist_w, 7 + round(mass * 3.5))` | [6, 16] piksel |
| `leg_gap` | `core/src/generator.cpp:203` | `(musc > 0.6 atau mass < 0.35) ? 1 : 0` | [0, 1] piksel |
| `thigh_thick` | `core/src/generator.cpp:204-206` | `(mass > 0.7 dan musc > 0.5) ? 4 : ((mass < 0.3 dan musc < 0.3) ? 2 : 3)` | [2, 4] piksel |
| `arm_base_thick` | `core/src/generator.cpp:344` | `(musc > 0.65 atau mass > 0.75) ? 3 : 2` | [2, 3] piksel |

Sumber: `core/include/npc/body_geometry.hpp:26-49`, `core/src/generator.cpp:112, 118, 198-206, 343`.

### 5.2. Perilaku penggambaran cedera dan amputasi

Audit terhadap `core/src/generator.cpp:486-538` pada commit acuan `a7c90dd` menunjukkan bahwa loop cedera tidak memeriksa status amputasi bagian tubuh terkait. Cedera pada tangan dan kaki jatuh ke koordinat bawaan badan karena ketiadaan cabang kondisi khusus.

Pada commit `ea0ca77`, terdapat dua perubahan perilaku visual yang diterapkan:
1. Pengecekan `if (cond.is_amputated(inj.body_part)) continue;` di baris 489.
2. Penambahan kotak pembatas khusus untuk `LeftHand`, `RightHand`, `LeftFoot`, dan `RightFoot` di baris 498-515.

Pemeriksaan terhadap 7.200 kasus dasar lama dan 160 kasus kondisi lama membuktikan bahwa 7.360 kombinasi dasar tersebut tidak memuat kombinasi cedera bersama amputasi, dan cedera lama hanya diuji pada badan. Oleh karena itu, seluruh 7.360 hash dasar lama tetap identik 100 persen. Perubahan visual hanya terjadi pada skenario baru yang memuat kombinasi tersebut.

Sumber: `core/src/generator.cpp:486-515`, `tests/golden/matriks-acuan.txt:1-7360`.

### 5.3. Status matriks hasil acuan

Berkas `tests/golden/matriks-acuan.txt` pada commit `ea0ca77` berisi 7.884 baris dengan rincian:

| Kelompok | Jumlah baris | Status kompatibilitas | Kondisi uji |
| --- | --- | --- | --- |
| `base` | 7.200 | Kompatibel penuh | Benih 1 sampai 100, 6 animasi, 4 arah hadap, 3 waktu siklus |
| `amp_0` sampai `amp_9` | 100 | Kompatibel penuh | 10 amputasi tunggal, benih 1 sampai 10, diam, arah selatan |
| `inj_0` sampai `inj_5` | 60 | Kompatibel penuh | 6 jenis cedera badan, benih 1 sampai 10, diam, arah selatan |
| `part_inj_0` sampai `part_inj_9` | 400 | Kasus perluasan baru | Cedera pada 10 bagian tubuh dalam 4 arah hadap untuk benih 1 sampai 10 |
| `combo_0` sampai `combo_9` | 100 | Kasus perluasan baru | Kombinasi amputasi anggota gerak dengan cedera badan untuk benih 1 sampai 10 |
| `bound_0` sampai `bound_5` | 24 | Kasus perluasan baru | 6 profil batas parameter ekstrem dalam 4 arah hadap |

Total baris acuan adalah 7.884 baris. Sebanyak 7.360 baris pertama identik dengan commit acuan `a7c90dd`.

Sumber: `tests/golden/matriks-acuan.txt`, `tests/generate_fixtures.cpp:29-222`.

### 5.4. Kebijakan versi aktual

Nilai versi aktual pada commit `ea0ca77`:

| Pengenal | Nilai aktual | Berkas sumber | Mekanisme validasi |
| --- | --- | --- | --- |
| `NPC_API_VERSION` | 4 | `core/include/npc/c_api.h:11` | Kesetaraan ketat `struct_size != sizeof(CNpcProfile)` di `core/src/c_api.cpp:19, 62, 124` |
| `PROFILE_SCHEMA_VERSION` | 2 | `core/include/npc/profile.hpp:15` | Kesetaraan ketat `schema_version != PROFILE_SCHEMA_VERSION` di `core/include/npc/profile.hpp:216` |
| `SERIALIZER_FORMAT_VERSION` | 2 | `core/include/npc/serializer.hpp:13` | Penanda format teks penampilan manusia lama di `core/include/npc/serializer.hpp:68` |
| `PROFILE_FORMAT_VERSION` | 4 | `core/include/npc/serializer.hpp:14` | Format teks profil karakter di `core/include/npc/serializer.hpp:236` |
| `LEGACY_PROFILE_FORMAT_VERSION` | 3 | `core/include/npc/serializer.hpp:15` | Jalur migrasi format profil lama di `core/include/npc/serializer.hpp:348` |

Sumber: berkas sumber pada tabel di atas.

---

## 6. Sumber kebenaran dan invarian geometri

Untuk mencegah terjadinya inkonsistensi antara dua representasi data, aturan invarian geometri ditetapkan sebagai berikut:

### 6.1. Invarian sumber kebenaran tunggal

1. Parameter anatomi (`body_mass` dan `muscularity`) adalah sumber kebenaran primer dari `BodyGeometry`.
2. Struktur `BodyGeometry` adalah representasi nilai turunan terhitung yang disimpan untuk efisiensi pembacaan cepat dan determinisme.
3. Fungsi `compute_geometry(body_mass, muscularity)` adalah satu-satunya algoritma resmi penghitungan geometri dasar.
4. Nilai turunan per baris piksel, seperti kurva lebar dada, lekukan pinggang, dan pelebaran pinggul, tidak disimpan di dalam struktur data melainkan dihitung secara lokal saat proses penggambaran berlangsung.

### 6.2. Kebijakan sinkronisasi dan penanganan data basi

1. **Pembuatan Profil:** Fungsi `CharacterProfile::create_default()` dan `CharacterProfile::from_legacy()` wajib memanggil `compute_geometry()` untuk menginisialisasi field `body_geometry`.
2. **Normalisasi Profil:** Fungsi `ProfileValidator::normalize()` wajib menghitung ulang `body_geometry` dari parameter anatomi setelah nilai pecahan dijepit ke rentang sah.
3. **Validasi Profil:** Fungsi `ProfileValidator::validate()` memeriksa bahwa setiap nilai di dalam `body_geometry` berada dalam batas fisik yang sah. Jika nilai berada di luar batas, profil dinyatakan tidak valid.
4. **Deserialisasi Data Lama:** Saat membaca format versi 3 atau format versi 2 yang tidak memuat field geometri, deserializer secara otomatis menghitung `body_geometry` menggunakan `compute_geometry()`.
5. **Jembatan Antarmuka C:** Pemanggilan `npc_profile_create_default()` mengisi 7 field geometri secara otomatis. Pemanggilan `npc_profile_render()` membaca field geometri dari struktur biner dan memvalidasinya sebelum proses render dimulai.
6. **Perluasan Godot:** Jika kamus masukan pada `dict_to_profile()` tidak memiliki kunci `body_geometry`, fungsi menghitungnya langsung dari anatomi menggunakan `compute_geometry()`.

---

## 7. Kontrak kompatibilitas dan strategi pengujian

Pengujian sistem dibagi secara tegas menjadi tiga kategori:

### Kategori A: Regresi dasar (Regression Baseline)

- **Cakupan:** 7.360 kombinasi acuan lama yang terdiri dari 7.200 kasus variasi animasi dasar, 100 kasus amputasi tunggal, dan 60 kasus cedera badan.
- **Kriteria Lulus:** Hash 64 bit dari penyangga piksel wajib identik 100 persen dengan commit acuan `a7c90dd`.
- **Perintah Verifikasi:** `ctest --test-dir build -R GoldenFixtureTest --output-on-failure`.

### Kategori B: Perluasan kompatibilitas (Compatibility Expansion)

- **Cakupan:** 524 kombinasi acuan baru yang terdiri dari 400 kasus cedera seluruh 10 bagian tubuh dalam 4 arah hadap, 100 kasus kombinasi amputasi bersama cedera, dan 24 kasus profil batas ekstrem.
- **Kriteria Lulus:** Seluruh kombinasi baru menghasilkan hash yang deterministik dan konsisten pada seluruh platform.
- **Verifikasi Kemonotonan:** Target uji `test_transition` memverifikasi 12 ambang batas diskretisasi dan 5 pasangan kemonotonan parameter.
- **Verifikasi Rumus Geometri:** Target uji `test_geometry` memverifikasi kesetaraan nilai `compute_geometry()` terhadap 100 benih acak.

### Kategori C: Perubahan perilaku visual yang disengaja (Intentional Behavior Changes)

Dua perubahan perilaku visual disetujui untuk memperbaiki anomali grafis:

1. **Pengabaian Cedera pada Bagian Tubuh Teramputasi:**
   - *Perilaku Lama:* Cedera tetap digambar melayang pada koordinat bagian tubuh yang telah diamputasi.
   - *Perilaku Baru:* Cedera dilewati jika `cond.is_amputated(inj.body_part)` bernilai benar.
   - *Verifikasi:* Pengujian unit pada `tests/test_profile.cpp` dan kombinasi `combo_0` sampai `combo_9` pada matriks acuan.

2. **Kotak Pembatas Cedera Tangan dan Kaki:**
   - *Perilaku Lama:* Bagian tubuh `LeftHand`, `RightHand`, `LeftFoot`, dan `RightFoot` tidak memiliki percabangan koordinat khusus sehingga menggunakan kotak pembatas badan.
   - *Perilaku Baru:* Keempat bagian tubuh tersebut memiliki kotak pembatas koordinat yang sesuai dengan posisi fisiknya pada kanvas.
   - *Verifikasi:* Kasus `part_inj_4`, `part_inj_5`, `part_inj_8`, dan `part_inj_9` pada matriks acuan.

---

## 8. Kebijakan versi dan migrasi data

### 8.1. Antarmuka Pemrograman C (`NPC_API_VERSION`)

- **Nilai:** 4 (sebelumnya 3).
- **Alasan Kenaikan:** Penambahan 7 field bilangan bulat bertanda 16 bit (`int16_t`) pada akhir struktur biner `CNpcProfile`.
- **Kontrak antarmuka biner:** Pemeriksaan `struct_size != sizeof(CNpcProfile)` menggunakan kesetaraan ketat. Pemanggil lama yang menggunakan struktur ukuran versi 3 ditolak dengan kode galat `NPC_ERR_VERSION_MISMATCH` (-3).
- **Keamanan Memori:** Penolakan ukuran lama mencegah terjadinya pembacaan memori liar di luar batas alokasi pemanggil.

### 8.2. Skema Profil Karakter (`PROFILE_SCHEMA_VERSION`)

- **Nilai:** 2 (sebelumnya 1).
- **Alasan Kenaikan:** Penambahan domain `body_geometry` ke dalam struktur `CharacterProfile`.
- **Kebijakan Validasi:** Fungsi `ProfileValidator::validate()` menolak profil dengan nomor skema selain 2.
- **Migrasi:** Profil skema 1 yang dimuat melalui deserializer secara otomatis ditingkatkan menjadi skema 2 dengan menghitung nilai geometri dari anatomi.

### 8.3. Format Serialisasi Profil (`PROFILE_FORMAT_VERSION`)

- **Nilai:** 4 (sebelumnya 3).
- **Alasan Kenaikan:** Penyimpanan 7 nilai integer geometri tubuh setelah data masker amputasi.
- **Dukungan Format Lama:**
  - Format versi 4: Membaca seluruh field profil termasuk 7 nilai geometri.
  - Format versi 3: Membaca data profil lama dan menghitung field geometri secara otomatis.
  - Format versi 2: Membaca struktur penampilan lama dan memigrasikannya ke profil lengkap.
- **Pencegahan Pemotongan Tipe Data:** Seluruh nilai numerik mentah dibaca ke dalam variabel penampung bertipe integer ukuran penuh sebelum divalidasi rentang batasnya. Nilai seperti `256`, `999`, atau angka negatif ditolak sebelum proses konversi ke tipe data yang lebih kecil dilakukan.

---

## 9. Kriteria transisi dan batas geometri

### 9.1. Matriks ambang batas diskretisasi

Dua belas ambang batas pada algoritma penggambaran diuji perilakunya pada nilai sebelum, tepat pada ambang, dan sesudah ambang:

| No | Kondisi logika | Parameter | Titik uji sebelum | Titik uji sesudah | Perilaku keluaran |
| --- | --- | --- | --- | --- | --- |
| 1 | `musc > 0.6 atau mass > 0.7` | Otot, massa | `musc = 0.59`, `mass = 0.50` | `musc = 0.61`, `mass = 0.50` | `neck_w` melompat dari 2 ke 3 piksel |
| 2 | `musc * 4.0 + mass * 1.5` | Otot, massa | Rentang kontinu [0.0, 1.0] | Rentang kontinu [0.0, 1.0] | `shoulder_w` bertambah secara bertahap |
| 3 | `mass * 4.5 + (1 - musc) * 1.0` | Massa, otot | Rentang kontinu [0.0, 1.0] | Rentang kontinu [0.0, 1.0] | `waist_w` bertambah seiring massa |
| 4 | `musc > 0.5 dan mass < 0.6` | Otot, massa | `musc = 0.49`, `mass = 0.40` | `musc = 0.90`, `mass = 0.40` | Bentuk V: `waist_w <= shoulder_w - 4` |
| 5 | `musc > 0.6 atau mass < 0.35` | Otot, massa | `musc = 0.59`, `mass = 0.50` | `musc = 0.61`, `mass = 0.50` | `leg_gap` melompat dari 0 ke 1 piksel |
| 6 | `mass > 0.7 dan musc > 0.5` | Massa, otot | `mass = 0.69`, `musc = 0.55` | `mass = 0.75`, `musc = 0.55` | `thigh_thick` melompat dari 3 ke 4 piksel |
| 7 | `mass < 0.3 dan musc < 0.3` | Massa, otot | `mass = 0.35`, `musc = 0.25` | `mass = 0.25`, `musc = 0.25` | `thigh_thick` melompat dari 3 ke 2 piksel |
| 8 | `musc > 0.4 atau mass > 0.4` | Otot, massa | `musc = 0.39`, `mass = 0.30` | `musc = 0.41`, `mass = 0.30` | Pelebaran lateral betis bertambah 1 piksel |
| 9 | `mass > 0.65` | Massa | `mass = 0.64` | `mass = 0.66` | Tonjolan perut bertambah hingga 2,5 piksel |
| 10 | `musc > 0.65 atau mass > 0.75` | Otot, massa | `musc = 0.64`, `mass = 0.50` | `musc = 0.66`, `mass = 0.50` | `arm_base_thick` melompat dari 2 ke 3 piksel |
| 11 | `musc > 0.45` | Otot | `musc = 0.44` | `musc = 0.46` | Titik bayangan dada muncul pada tampak selatan |
| 12 | `musc > 0.55` | Otot | `musc = 0.54` | `musc = 0.56` | Garis tengah otot perut muncul |

Sumber: `core/src/generator.cpp:118-344`, diverifikasi oleh `tests/test_transition.cpp:8-44`.

### 9.2. Tabel kemonotonan terverifikasi

Kemonotonan hanya diwajibkan untuk pasangan parameter yang dirancang monoton:

| Pasangan parameter | Kondisi parameter lain | Sifat kemonotonan | Dasar perancangan |
| --- | --- | --- | --- |
| (`muscularity`, `shoulder_w`) | Semua nilai `body_mass` | Tidak pernah turun (naik) | Pertambahan otot selalu melebarkan bahu |
| (`body_mass`, `waist_w`) | `muscularity` konstan | Tidak pernah turun (naik) | Pertambahan massa selalu melebarkan pinggang |
| (`body_mass`, `hips_w`) | Semua nilai `muscularity` | Tidak pernah turun (naik) | Pertambahan massa selalu melebarkan pinggul |
| (`body_mass`, `shoulder_w`) | `muscularity` konstan | Tidak pernah turun (naik) | Pertambahan massa menambah massa bahu |
| (`muscularity`, `waist_w`) | `body_mass` konstan | Tidak pernah naik (turun) | Pertambahan otot merampingkan pinggang |

Pasangan yang tidak monoton:
- Hubungan antara `body_mass` dengan lebar baris tengah badan tidak monoton karena keberadaan suku diskret di baris 297 yang menambahkan tonjolan perut secara terlokalisir.
- Hubungan antara `muscularity` dengan `thigh_thick` tidak monoton global karena dipengaruhi oleh interaksi ambang ganda antara massa dan otot.

Sumber: `tests/test_transition.cpp:46-93`.

---

## 10. Kriteria penerimaan final

| Pengenal | Pernyataan kriteria penerimaan | Status verifikasi |
| --- | --- | --- |
| `Penerimaan-001` | Sebanyak 7.360 kombinasi hasil acuan dasar lama menghasilkan nilai hash piksel identik 100 persen. | Terverifikasi lulus pada commit `ea0ca77` |
| `Penerimaan-002` | Sebanyak 524 kombinasi hasil acuan perluasan baru terverifikasi deterministik dan lulus tanpa selisih. | Terverifikasi lulus pada commit `ea0ca77` |
| `Penerimaan-003` | Fungsi `compute_geometry()` menghasilkan 7 nilai dasar yang identik dengan rumus kanonikal untuk 100 benih acak. | Terverifikasi lulus pada commit `ea0ca77` |
| `Penerimaan-004` | Seluruh 12 ambang batas diskretisasi dan 5 pasangan kemonotonan parameter lulus pengujian transisi. | Terverifikasi lulus pada commit `ea0ca77` |
| `Penerimaan-005` | Serialisasi format versi 4 melakukan perjalanan bolak-balik tanpa kehilangan data untuk 100 profil acak. | Terverifikasi lulus pada commit `ea0ca77` |
| `Penerimaan-006` | Deserializer mampu memigrasikan data format versi 3 dan format versi 2 dengan penghitungan otomatis geometri yang valid. | Terverifikasi lulus pada commit `ea0ca77` |
| `Penerimaan-007` | Nilai enum dan integer mentah tidak valid ditolak sebelum pemotongan tipe data saat deserialisasi. | Terverifikasi lulus pada commit `ea0ca77` |
| `Penerimaan-008` | Pemanggilan antarmuka C dengan ukuran biner versi lama ditolak secara aman dengan kode `NPC_ERR_VERSION_MISMATCH`. | Terverifikasi lulus pada commit `ea0ca77` |
| `Penerimaan-009` | Cedera pada bagian tubuh yang diamputasi tidak menghasilkan piksel visual anomali pada kanvas keluaran. | Terverifikasi lulus pada commit `ea0ca77` |
| `Penerimaan-010` | Cedera pada pergelangan tangan dan kaki digambar pada posisi fisik anggota badan terkait dan tidak jatuh ke badan. | Terverifikasi lulus pada commit `ea0ca77` |
| `Penerimaan-011` | Skrip uji asap Godot headless memvalidasi keberadaan dan keabsahan subdomain `body_geometry`. | Terverifikasi lulus pada commit `ea0ca77` |
| `Penerimaan-012` | Seluruh 7 target uji pada rangkaian CTest lulus 100 persen tanpa kegagalan. | Terverifikasi lulus pada commit `ea0ca77` |

---

## 11. Prosedur reproduksi pengujian

Perintah pengujian mandiri untuk memverifikasi seluruh kriteria penerimaan:

```bash
# 1. Konfigurasi dan kompilasi target rilis
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# 2. Eksekusi seluruh rangkaian pengujian unit CTest
ctest --test-dir build --output-on-failure

# 3. Eksekusi pengujian asap engine Godot secara headless
godot --headless --path godot --script scripts/smoke_test_profile.gd
```

---

## 12. Glosarium istilah baku

| Istilah | Arti |
| --- | --- |
| Karakter | Makhluk di dunia simulasi yang digerakkan oleh logika sistem, bukan pemain. |
| Geometri tubuh | Kumpulan 7 nilai dasar bilangan bulat yang menentukan ukuran fisik bagian tubuh karakter dalam satuan piksel. |
| Penampilan | Nilai bentuk tubuh, wajah, dan warna yang dihitung dari genom karakter. |
| Penggambaran | Proses mengubah data penampilan, kondisi fisik, dan perintah animasi menjadi susunan piksel warna. |
| Penyangga piksel | Larik memori warna berukuran 32 × 48 piksel hasil dari proses penggambaran. |
| Jembatan C | Lapisan antarmuka biner berbahasa C yang membungkus logika inti C++20. |
| Perluasan Godot | Modul pustaka dinamis berbasis GDExtension yang dimuat oleh mesin permainan Godot. |
| Hasil acuan | Kumpulan data keluaran hash piksel tersimpan yang digunakan untuk mendeteksi terjadinya regresi visual. |
| Sidik digital | Nilai hash ringkas 64 bit yang mewakili keutuhan data penyangga piksel secara deterministik. |
| Ambang batas diskret | Percabangan kondisi logika yang menyebabkan dimensi geometri melompat secara bertingkat. |
| Kemonotonan | Karakteristik matematis suatu nilai yang tidak pernah menurun atau tidak pernah meningkat terhadap pertambahan masukan. |
| Perjalanan bolak-balik | Proses pengujian data yang diserialisasi ke teks lalu dibaca kembali hingga menghasilkan struktur data yang identik. |
| Genom | Struktur data genetik karakter yang dikelola oleh inti simulasi dunia. |

---

## 13. Riwayat revisi

| Tanggal | Versi | Perubahan | Alasan |
| --- | --- | --- | --- |
| 2026-10-25 | 1 | Dokumen awal rencana v0.3.3 | Mengakomodasi kebutuhan pemisahan geometri tubuh dan penyesuaian nomor versi sistem. |
| 2026-10-25 | 2 | Revisi menyeluruh rencana v0.3.3 | Menegaskan kontrak kompatibilitas, membedakan regresi dasar dari perluasan acuan, mendokumentasikan invarian sumber kebenaran tunggal, memperinci kebijakan versi dan migrasi, serta menetapkan 12 kriteria penerimaan objektif berbasis audit kode commit `ea0ca77`. |
