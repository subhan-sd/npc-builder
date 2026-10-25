# BRD v0.3.2 — Character Profile Validation, Serialization & CI Reliability

## 1. Informasi Umum

**Nama proyek:** NPC Builder  
**Versi target:** v0.3.2  
**Jenis pekerjaan:** Reliability, validation hardening, serialization integrity, CI/CD  
**Komponen terdampak:**

- C++ Core
- Character Profile System
- C API
- Profile Serializer / Deserializer
- Renderer integration
- CMake build configuration
- GitHub Actions CI
- Godot GDExtension
- Native tests dan Godot smoke tests

## 2. Latar Belakang

Implementasi Character Profile System pada v0.3.1 telah memperkenalkan pemisahan data profil karakter menjadi anatomi, penampilan, kondisi fisik, dan metadata generasi. Namun, masih terdapat beberapa risiko yang perlu ditutup sebelum sistem dianggap andal:

1. CI gagal saat konfigurasi CMake karena submodule `extension/godot-cpp` tidak tersedia pada runner.
2. Validasi enum dan field yang diterima melalui C API berpotensi belum mencakup seluruh input.
3. `npc_profile_render()` perlu menjamin bahwa profil invalid ditolak sebelum mencapai renderer.
4. Round-trip serialization perlu membuktikan bahwa seluruh field yang didukung tetap utuh, termasuk injury dan amputasi.
5. Tes native dan Godot perlu dijalankan terhadap hasil build extension yang benar-benar baru, bukan fallback atau binary lama.

Masalah tersebut dapat menyebabkan CI tidak memberikan bukti kualitas yang valid, data profil invalid lolos ke proses rendering, atau kondisi karakter berubah/hilang setelah proses serialisasi.

## 3. Tujuan

- Membuat seluruh matrix CI dapat dikonfigurasi, dibangun, dan dijalankan dengan benar.
- Menjadikan validasi profil konsisten di seluruh jalur masuk C API.
- Menjamin renderer hanya menerima profil yang valid.
- Menjamin round-trip serialisasi mempertahankan seluruh field yang didukung.
- Memastikan hasil pengujian Godot menggunakan GDExtension hasil build terbaru.
- Mempertahankan C++ Core sebagai **single source of truth**.

## 4. Ruang Lingkup

### 4.1 Termasuk

- Perbaikan checkout Git submodule di GitHub Actions.
- Pemeriksaan konfigurasi CMake dan build pada seluruh job matrix.
- Audit validator Character Profile.
- Audit konversi data C API ke model C++.
- Konsistensi error pada API validasi dan rendering.
- Audit serializer dan deserializer profil.
- Pengujian field anatomi, penampilan, kondisi fisik, injury, dan amputasi.
- Native regression tests.
- Godot smoke test yang membuktikan extension baru berhasil dimuat dan digunakan.
- Dokumentasi singkat mengenai cara menjalankan validasi dan pengujian.

### 4.2 Tidak termasuk

- Perubahan besar arsitektur Character Profile.
- Penambahan field profil baru yang tidak dibutuhkan untuk menutup gap.
- Penulisan ulang renderer atau sistem anatomi.
- Perubahan gaya visual atau penambahan fitur gameplay.
- Migrasi format serialisasi secara breaking tanpa kebutuhan yang terbukti.
- Refactor menyeluruh sistem build di luar kebutuhan CI dan pengujian extension.

## 5. Kebutuhan Fungsional

### FR-01 — Perbaikan Checkout Submodule dan CI Matrix

**Deskripsi:** Seluruh job CI harus memperoleh source code yang dibutuhkan untuk konfigurasi dan build, termasuk submodule Godot C++.

**Kebutuhan:**

1. Workflow checkout harus menginisialisasi submodule secara recursive, menggunakan konfigurasi yang sesuai, misalnya:

   ```yaml
   - name: Checkout code
     uses: actions/checkout@v4
     with:
       submodules: recursive
   ```

2. Pastikan `extension/godot-cpp` terisi pada runner sebelum CMake memanggil `add_subdirectory()`.
3. Periksa apakah seluruh platform dan konfigurasi dalam matrix memiliki dependency yang sama atau membutuhkan konfigurasi khusus.
4. Jalankan ulang seluruh matrix CI setelah perubahan diterapkan.
5. Pastikan tidak ada job yang gagal sebelum tahap compile akibat dependency atau path yang tidak tersedia.
6. Jika suatu job memang hanya menguji core tanpa extension, pemisahan tersebut harus eksplisit dan tidak boleh membuat smoke test Godot melewati build extension.

**Acceptance Criteria:**

- [ ] Submodule tersedia pada seluruh runner yang membutuhkannya.
- [ ] Konfigurasi CMake berhasil pada seluruh job matrix.
- [ ] Seluruh job matrix mencapai tahap build dan test yang semestinya.
- [ ] Seluruh job wajib berstatus sukses.
- [ ] Log CI membuktikan hasil aktual, bukan sekadar perubahan konfigurasi workflow.

### FR-02 — Validasi Enum dan Seluruh Field C API

**Deskripsi:** Semua nilai yang diterima melalui C API harus dipetakan dengan benar ke model C++ dan diperiksa sesuai domain serta aturan validasinya.

**Kebutuhan:**

1. Audit seluruh field yang diterima oleh fungsi C API untuk pembuatan, pembaruan, validasi, serialisasi, dan rendering profil.
2. Pastikan konversi C API → model C++ tidak menghilangkan atau mengubah field secara tidak sengaja.
3. Validasi semua enum dengan membandingkan nilai terhadap pilihan yang benar-benar didukung, bukan menganggap setiap integer sebagai nilai valid.
4. Periksa seluruh field enum yang digunakan oleh profil, termasuk jika didukung:
   - Ekspresi wajah.
   - Bentuk mata.
   - Jarak mata.
   - Bentuk hidung.
   - Bentuk mulut.
   - Gaya rambut.
   - Bagian tubuh yang mengalami injury.
   - Jenis atau kategori kondisi fisik.
   - Field enum lain yang ditemukan saat audit implementasi.
5. Periksa field non-enum berdasarkan kontrak masing-masing: rentang numerik, nilai yang diperbolehkan, konsistensi antar-field, dan aturan wajib/opsional.
6. Periksa pointer, panjang string, ukuran array, dan nilai numerik yang berpotensi tidak valid jika field tersebut memang diterima oleh API.
7. Jangan menambahkan aturan arbitrer yang tidak didukung oleh model atau kontrak API.
8. Pastikan semua jalur validasi menggunakan aturan yang konsisten.

**Acceptance Criteria:**

- [ ] Tersedia inventaris seluruh field yang diterima melalui C API dan jalur konversinya.
- [ ] Setiap enum memiliki validasi nilai yang eksplisit.
- [ ] Setiap field non-enum diperiksa sesuai kontrak dan batas yang berlaku.
- [ ] Field yang dikirim melalui C API tidak hilang saat dikonversi ke model C++.
- [ ] Tes membuktikan bahwa nilai valid diterima dan nilai invalid ditolak.
- [ ] Tidak ada field yang dianggap tervalidasi jika ternyata tidak diperiksa.
- [ ] Validasi tidak mengubah profil valid secara diam-diam.

### FR-03 — Konsistensi Validasi `npc_profile_render()`

**Deskripsi:** `npc_profile_render()` wajib menolak profil invalid sebelum proses rendering dimulai.

**Kebutuhan:**

1. Jalankan validasi profil melalui validator resmi Character Profile sebelum memanggil renderer.
2. Jangan hanya mengandalkan validasi internal renderer atau meneruskan data langsung ke renderer.
3. Jika profil invalid, hentikan proses dan kembalikan status error sesuai kontrak C API.
4. Gunakan mekanisme error yang konsisten dengan fungsi C API lain, termasuk format atau kode error yang telah ditetapkan proyek.
5. Pastikan profil invalid tidak menghasilkan output render parsial yang dianggap sukses.
6. Pastikan output, buffer, dan parameter keluaran ditangani dengan aman ketika validasi gagal.
7. Pertahankan kompatibilitas perilaku untuk profil valid.
8. Jangan membuat sistem error baru jika proyek sudah memiliki mekanisme error yang bisa digunakan kembali.

**Acceptance Criteria:**

- [ ] Profil valid dapat dirender seperti sebelumnya.
- [ ] Profil invalid ditolak sebelum renderer dipanggil.
- [ ] Kode/status error konsisten dengan kontrak C API.
- [ ] Output render tidak dilaporkan sukses ketika validasi gagal.
- [ ] Tes mencakup profil invalid karena enum, nilai field, dan kondisi inkonsisten.
- [ ] Tes memastikan tidak terjadi crash akibat input invalid.

### FR-04 — Integritas Round-Trip Serialization

**Deskripsi:** Seluruh field profil yang didukung harus tetap terpelihara setelah proses serialize → deserialize.

**Kebutuhan:**

1. Audit serializer dan deserializer untuk seluruh format profil yang masih didukung.
2. Identifikasi field yang:
   - Diserialisasi dan dipulihkan dengan benar.
   - Diserialisasi tetapi tidak dipulihkan.
   - Tidak ikut diserialisasi.
   - Dipulihkan menggunakan nilai default tanpa dasar yang benar.
3. Periksa pemetaan antara model C++, representasi serialized, dan objek hasil deserialisasi.
4. Pastikan injury dan amputasi dipertahankan, termasuk detail yang benar-benar didukung model, seperti bagian tubuh terdampak, tipe kondisi, atau atribut terkait.
5. Jangan menganggap injury dan amputasi identik. Pertahankan representasi dan semantik masing-masing sesuai model yang ada.
6. Pastikan koleksi kondisi fisik tidak hilang, tertukar urutannya jika urutan merupakan bagian kontrak, atau terduplikasi secara tidak sengaja.
7. Pertahankan metadata profil yang didukung, termasuk anatomi, penampilan, kondisi fisik, dan metadata generasi.
8. Jika terdapat versi format lama, pertahankan perilaku kompatibilitas yang memang sudah dijanjikan oleh proyek.
9. Field yang tidak didukung oleh versi format lama harus mengikuti kebijakan migrasi/default yang eksplisit, bukan diklaim sebagai round-trip lossless.
10. Gunakan perbandingan semantik profil sebelum dan sesudah round-trip, bukan hanya memeriksa apakah proses berhasil atau string JSON terbentuk.

**Acceptance Criteria:**

- [ ] Seluruh field yang didukung format terkait tercakup dalam audit.
- [ ] Profil lengkap dapat di-serialize dan di-deserialize tanpa kehilangan field yang dijanjikan.
- [ ] Injury tetap ada setelah round-trip dengan nilai dan atribut yang didukung tetap sama.
- [ ] Amputasi tetap ada setelah round-trip dengan representasi dan atribut yang didukung tetap sama.
- [ ] Profil dengan beberapa kondisi fisik dapat dipulihkan secara benar.
- [ ] Tes mencakup nilai default, profil non-default, koleksi kosong, dan beberapa kondisi fisik.
- [ ] Tes mencakup kompatibilitas format lama jika masih menjadi bagian kontrak proyek.
- [ ] Perbedaan yang memang diharapkan akibat migrasi format didokumentasikan dan diuji secara eksplisit.

### FR-05 — Native Regression Tests dan Godot Smoke Test

**Deskripsi:** Verifikasi dilakukan pada hasil build yang aktual dan memastikan Godot menggunakan extension yang baru dibangun.

**Kebutuhan:**

1. Jalankan seluruh tes native yang relevan untuk core, validator, C API, serializer, dan renderer.
2. Bangun GDExtension dari source code yang sedang diuji.
3. Pastikan artifact hasil build ditempatkan pada path yang dibaca oleh proyek Godot.
4. Jalankan Godot smoke test dengan extension yang baru dibangun.
5. Pastikan test tidak diam-diam memakai binary lama, extension yang di-bundle dari commit sebelumnya, atau fallback GDScript.
6. Verifikasi bahwa extension benar-benar berhasil dimuat dan fungsi native yang relevan dapat dipanggil.
7. Jika extension gagal dimuat, test harus gagal dengan alasan yang jelas.
8. Jika proyek memiliki fallback, smoke test harus dapat membedakan keberhasilan native dari keberhasilan fallback.
9. Simpan log atau hasil test yang dapat digunakan untuk menelusuri kegagalan.

**Acceptance Criteria:**

- [ ] Native test suite lulus.
- [ ] GDExtension dibangun dari source pada commit yang sedang diuji.
- [ ] Artifact extension yang diuji dapat ditelusuri ke hasil build tersebut.
- [ ] Godot berhasil memuat extension.
- [ ] Smoke test memanggil jalur native yang benar-benar menggunakan extension.
- [ ] Kegagalan load extension menyebabkan test gagal, bukan lulus melalui fallback.
- [ ] Hasil pengujian dapat diverifikasi melalui log CI atau laporan test.
- [ ] Tidak ada klaim sukses jika test belum benar-benar dijalankan.

## 6. Kebutuhan Non-Fungsional

### NFR-01 — Konsistensi Arsitektur

C++ Core tetap menjadi sumber kebenaran untuk model karakter, aturan validasi, dan semantik data. Godot berfungsi sebagai integrasi/rendering layer dan tidak boleh memiliki aturan validasi duplikat yang menghasilkan perilaku berbeda.

### NFR-02 — Backward Compatibility

Perubahan tidak boleh merusak format serialized atau kontrak C API yang masih didukung tanpa keputusan migrasi yang eksplisit.

### NFR-03 — Determinisme

Input profil yang sama harus menghasilkan hasil validasi dan hasil round-trip yang konsisten, kecuali ada perilaku nondeterministik yang memang sudah menjadi bagian kontrak sistem.

### NFR-04 — Keterlacakan

Setiap acceptance criteria harus dapat ditautkan ke tes, log CI, atau bukti hasil build.

### NFR-05 — Keamanan Kegagalan

Input invalid harus ditolak secara terkendali. Kegagalan validasi tidak boleh menyebabkan crash, korupsi output, atau status sukses palsu.

## 7. Strategi Pengujian

### 7.1 Unit Tests — Validator

Cakupan minimum:

- Profil default yang valid.
- Profil valid dengan nilai non-default.
- Setiap enum dengan nilai valid.
- Nilai enum di luar domain yang didukung.
- Nilai numerik di luar rentang yang diizinkan.
- Kombinasi field yang melanggar aturan konsistensi.
- Kondisi fisik dengan bagian tubuh atau tipe yang invalid.
- Field yang masuk melalui C API dan field yang sama pada model C++.

### 7.2 C API Tests

- Profil valid diterima oleh API validasi.
- Profil invalid ditolak dengan error yang konsisten.
- Konversi C API mempertahankan seluruh field yang didukung.
- `npc_profile_render()` menolak profil invalid sebelum rendering.
- Penanganan pointer atau buffer invalid sesuai kontrak API.
- Output dan status error benar ketika validasi gagal.

### 7.3 Serialization Tests

- Serialize → deserialize → compare untuk profil default.
- Serialize → deserialize → compare untuk profil dengan seluruh kelompok field terisi.
- Profil dengan injury.
- Profil dengan amputasi.
- Profil dengan injury dan amputasi sekaligus.
- Profil dengan beberapa kondisi fisik.
- Field opsional atau koleksi kosong.
- Format lama dan migrasi versi jika masih didukung.
- Verifikasi field yang hilang, berubah, atau mendapat nilai default secara tidak semestinya.

### 7.4 CI Matrix Tests

- Checkout submodule.
- CMake configure.
- Build native core.
- Native tests.
- Build GDExtension pada platform yang didukung.
- Godot smoke test pada runner yang memenuhi kebutuhan runtime Godot.
- Pemeriksaan bahwa semua job wajib selesai dan sukses.

Jika suatu platform tidak dapat menjalankan Godot secara langsung, keterbatasan tersebut harus ditangani melalui konfigurasi runner atau pembagian job yang eksplisit. Jangan menganggap build native sebagai pengganti smoke test Godot.

### 7.5 Godot Smoke Test

Urutan minimal:

1. Bersihkan atau ganti artifact lama sesuai prosedur build.
2. Build extension dari source yang sedang diuji.
3. Verifikasi artifact tersedia di lokasi yang diharapkan.
4. Jalankan Godot dalam mode headless jika didukung oleh setup proyek.
5. Pastikan extension termuat.
6. Panggil jalur native yang relevan.
7. Pastikan hasil test gagal jika extension tidak tersedia atau gagal dimuat.
8. Catat exit code dan log.

## 8. Prioritas Implementasi

**P0 — CI dan validitas bukti pengujian**

- Perbaiki checkout submodule.
- Pastikan CMake configure dan seluruh matrix berjalan.
- Pastikan pengujian extension tidak menggunakan binary lama.

**P1 — Validasi profil dan C API**

- Inventaris field.
- Lengkapi enum dan field validation.
- Perbaiki jalur konversi C API.
- Terapkan validasi sebelum render.
- Samakan error handling.

**P2 — Integritas serialisasi**

- Audit semua field.
- Perbaiki round-trip.
- Tambahkan regression tests untuk injury dan amputasi.
- Verifikasi backward compatibility.

**P3 — Verifikasi akhir**

- Jalankan native tests.
- Jalankan Godot smoke test menggunakan extension hasil build terbaru.
- Simpan bukti hasil build dan test.

Urutan ini dapat disesuaikan jika dependency teknis mengharuskan sebagian pekerjaan dilakukan paralel. Namun, laporan akhir harus membedakan pekerjaan yang selesai dari pekerjaan yang baru direncanakan.

## 9. Deliverables

1. Perubahan GitHub Actions untuk checkout submodule dan matrix CI.
2. Audit atau dokumentasi mapping field C API → model C++.
3. Validator yang mencakup seluruh field yang didukung.
4. Perbaikan `npc_profile_render()` dan error handling.
5. Perbaikan serializer/deserializer jika ditemukan kehilangan data.
6. Regression tests untuk validasi, rendering, dan round-trip.
7. Native test report.
8. Godot smoke test report dengan bukti extension yang digunakan.
9. Ringkasan perubahan, keputusan kompatibilitas, dan masalah yang masih tersisa.

## 10. Definition of Done

Versi v0.3.2 dinyatakan selesai jika:

- [ ] Seluruh job wajib pada matrix CI lulus.
- [ ] Checkout submodule bekerja pada runner yang membutuhkannya.
- [ ] Semua field yang diterima melalui C API telah diaudit dan divalidasi sesuai kontrak.
- [ ] `npc_profile_render()` menolak profil invalid sebelum memanggil renderer.
- [ ] Error yang dikembalikan konsisten dengan kontrak API.
- [ ] Round-trip mempertahankan seluruh field yang dijanjikan oleh format, termasuk injury dan amputasi.
- [ ] Native regression tests lulus.
- [ ] Godot smoke test membuktikan extension hasil build terbaru benar-benar dimuat dan digunakan.
- [ ] Fallback tidak dapat menyamarkan kegagalan extension pada smoke test.
- [ ] Tidak ada regresi yang diketahui pada profil valid dan format lama yang masih didukung.
- [ ] Bukti CI dan pengujian tersedia untuk ditinjau.

## 11. Batasan dan Prinsip Implementasi

- Audit implementasi aktual sebelum mengubah kode.
- Jangan mengasumsikan semua field sudah tercakup hanya karena validator berhasil dipanggil.
- Jangan mengasumsikan serialisasi lossless hanya karena proses serialize dan deserialize tidak menghasilkan error.
- Jangan mengubah kontrak C API tanpa alasan dan pengujian yang jelas.
- Jangan menambah field profil atau melakukan refactor luas di luar scope.
- Jangan menonaktifkan tes atau menjadikan smoke test opsional hanya untuk memperoleh status CI hijau.
- Jika ada field yang memang tidak didukung serializer atau format lama, dokumentasikan batas tersebut secara eksplisit.
- Semua perbaikan harus disertai regression test yang mereproduksi masalah yang diperbaiki.

**Target akhir:** v0.3.2 memberikan bukti yang dapat diverifikasi bahwa Character Profile System konsisten dari input C API, validasi, rendering, hingga serialisasi dan integrasi Godot.
