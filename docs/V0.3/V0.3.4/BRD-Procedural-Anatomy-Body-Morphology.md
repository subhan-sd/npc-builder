# BRD — Procedural Anatomy & Body Morphology

**Project:** NPC Builder  
**Priority:** P2 — Fondasi Anatomi  
**Status:** Draft untuk review  
**Target:** v0.3.4 atau milestone anatomi terpisah sesuai hasil breakdown teknis  
**Architecture:** C++20 sebagai source of truth, Godot 4 sebagai renderer/UI

## 1. Ringkasan

Sistem ini menghasilkan variasi anatomi karakter secara prosedural, deterministik, dan terkontrol. Variasi tinggi, lebar, massa tubuh, muscularity, proporsi bagian tubuh, dan karakteristik anatomi wanita harus menghasilkan siluet berbeda secara bermakna, bukan hanya uniform scaling.

Prinsip utama:
1. Input dan seed yang sama menghasilkan morfologi yang sama.
2. Perubahan parameter tidak menghasilkan proporsi yang saling bertentangan.
3. Tinggi, lebar, massa, muscularity, dan proporsi dapat bervariasi dalam batas valid.
4. Parameter anatomi adalah sumber data utama; geometri adalah hasil kalkulasi.
5. Profil lama tetap dapat dibaca sesuai kontrak migrasi.
6. Perhitungan anatomi berada di C++ core, bukan GDScript.

## 2. Latar Belakang dan Masalah

Fondasi geometri, geometry override, dan pose sudah ada, tetapi fondasi geometri belum menjamin variasi anatomi yang kaya. Tanpa model morfologi terstruktur, karakter berisiko hanya tampak membesar atau mengecil.

Masalah yang diselesaikan:
- Proporsi torso, bahu, pinggul, lengan, dan kaki perlu bervariasi secara independen.
- Massa tubuh dan muscularity harus menghasilkan distribusi bentuk berbeda.
- Anatomi wanita membutuhkan parameter dan aturan proporsi eksplisit.
- Perubahan anatomi tidak boleh merusak pose, injury overlay, atau amputasi.
- Hasil generator harus deterministik dan dapat diuji.

## 3. Tujuan

- Menghasilkan variasi morfologi yang jelas dan dapat dikendalikan.
- Memisahkan parameter anatomi dari geometri hasil kalkulasi.
- Mendukung tubuh kurus, rata-rata, berisi, dan berotot.
- Mendukung variasi anatomi pria dan wanita tanpa memaksakan satu bentuk baku.
- Menjaga kontrak geometri untuk pose, injury, amputasi, serta pakaian di masa depan.
- Menjaga determinisme, serialisasi, dan kompatibilitas API.

## 4. Di Luar Cakupan

- Model 3D, rigging 3D, atau simulasi biomekanik.
- Nutrisi, pertumbuhan, penuaan, dan perubahan tubuh sepanjang hidup.
- Simulasi jaringan lunak.
- Sistem pakaian lengkap, armor, equipment, atau cloth simulation.
- Produksi sprite sheet final.
- Editor anatomi visual lengkap.
- Anatomi spesies nonmanusia khusus, kecuali melalui kontrak ekstensi berikutnya.

## 5. Kebutuhan Fungsional

### FR-01 — Variasi tinggi tubuh
- Tinggi memengaruhi dimensi vertikal tubuh.
- Tinggi tidak otomatis mengubah semua dimensi horizontal dengan faktor yang sama.
- Distribusi panjang torso dan kaki dapat berbeda pada tinggi keseluruhan yang sama.
- Hasil berada dalam batas geometri valid.
- Tinggi merupakan parameter sumber, bukan sekadar nilai yang disimpulkan dari geometri.

### FR-02 — Variasi lebar tubuh
Minimal mencakup lebar bahu, torso, pinggang, panggul, dan bagian anggota gerak jika didukung geometri. Perubahan lebar torso tidak boleh memaksa bahu dan panggul berubah dengan rasio yang sama.

### FR-03 — Variasi massa dan muscularity
Sistem menyediakan empat preset visual:

| Preset | Karakteristik yang diharapkan |
|---|---|
| Kurus | Siluet ramping dan volume jaringan lebih rendah |
| Rata-rata | Distribusi proporsi moderat sebagai baseline |
| Berisi | Volume lebih besar pada area yang ditentukan model |
| Berotot | Bahu, lengan, torso, atau bagian lain dapat tampak berotot tanpa harus bermassa tinggi |

Preset tidak saling eksklusif. Massa tubuh dan muscularity harus merupakan parameter berbeda; karakter berisi tidak selalu berotot dan karakter berotot tidak harus gemuk.

### FR-04 — Proporsi bagian tubuh
Parameter minimal:
- Panjang torso relatif terhadap tinggi.
- Lebar bahu relatif terhadap torso.
- Lebar pinggang relatif terhadap torso.
- Lebar panggul relatif terhadap tinggi dan torso.
- Panjang lengan dan kaki relatif terhadap tinggi.
- Rasio bagian atas/bawah anggota gerak apabila didukung geometri.

Setiap parameter memiliki rentang valid, default, dan aturan normalisasi. Perubahan parameter tidak boleh mengubah parameter tak terkait secara diam-diam.

### FR-05 — Anatomi wanita
Sistem mendukung:
- Variasi lebar bahu dan torso.
- Variasi lebar dan bentuk panggul.
- Variasi volume dada.
- Variasi proporsi torso, pinggang, dan panggul.
- Kombinasi tubuh ramping, rata-rata, berisi, dan berotot.

Dada dan panggul bukan satu faktor skala. Ukuran dada tidak otomatis menentukan bahu, tinggi, atau massa. Kombinasi parameter harus beragam dan tidak memaksakan stereotip.

**Batasan implementasi:** representasi visual dada harus mengikuti kemampuan renderer saat ini. Jika renderer sprite belum mendukung kontur yang memadai, implementasikan dan uji kontrak parameter terlebih dahulu; jadikan peningkatan visual sebagai subtask terpisah.

### FR-06 — Transisi proporsi terkontrol
- Hindari uniform scaling sebagai satu-satunya mekanisme.
- Karakter dengan tinggi sama dapat memiliki rasio torso/kaki berbeda.
- Karakter bermassa serupa dapat memiliki distribusi volume berbeda.
- Karakter berotot dapat memiliki proporsi bahu dan lengan berbeda.
- Karakter wanita dapat memiliki kombinasi volume dada dan lebar panggul berbeda.

Fungsi transformasi harus deterministik. Interpolasi antarprofil harus menghasilkan geometri valid pada seluruh titik transisi.

### FR-07 — Kompatibilitas body parts
Pertahankan identitas dan hubungan geometri minimal untuk Head, Torso, LeftArm/RightArm, LeftHand/RightHand, LeftLeg/RightLeg, LeftFoot/RightFoot. Bagian tubuh tidak boleh terlepas, tertukar sisi, atau kehilangan hubungan posisi dengan torso.

### FR-08 — Kompatibilitas pose, injury, dan amputasi
- Pose memakai geometri hasil morfologi yang sama dengan renderer.
- Bounding box injury mengikuti lokasi dan ukuran bagian tubuh terbaru.
- Cedera pada bagian tubuh yang tidak ada tidak dirender.
- Aturan amputasi berantai tetap berlaku.
- Perubahan morfologi tidak boleh menghidupkan kembali bagian yang diamputasi.
- Overlay injury harus tetap berada dalam batas penempatan yang disepakati.

### FR-09 — Determinisme
Input morfologi identik menghasilkan geometri identik sesuai kebijakan determinisme proyek. Randomization harus memakai seed terkontrol dan tidak bergantung pada waktu, urutan render, atau state global tak terdokumentasi.

### FR-10 — Validasi dan normalisasi
- Nilai di luar rentang ditangani secara konsisten.
- Normalisasi menghasilkan parameter dan geometri valid.
- Geometri hasil tidak menjadi sumber data utama yang menimpa parameter anatomi.
- Geometry override mengikuti kontrak core yang berlaku.
- Perubahan schema mempertimbangkan serialisasi dan kompatibilitas C API.

## 6. Kebutuhan Nonfungsional

- **Determinisme lintas platform:** pertahankan kebijakan determinisme proyek.
- **Regression safety:** golden fixtures tidak boleh berubah tanpa alasan, dokumentasi, dan review.
- **Performa:** kalkulasi ringan untuk menghasilkan banyak karakter; hindari kalkulasi mahal setiap render bila hasil dapat digunakan kembali dengan aman.
- **Maintainability:** jangan menduplikasi kalkulasi anatomi antara core, C API, dan GDExtension.
- **Extensibility:** model dapat diperluas di masa depan, tetapi variasi usia dan spesies bukan bagian dari deliverable ini.

## 7. Kontrak Data dan Arsitektur

Pisahkan tanggung jawab berikut:
1. **Anatomy Parameters:** sumber kebenaran parameter tubuh.
2. **Morphology Profile / Presets:** konfigurasi profil seperti kurus, rata-rata, berisi, dan berotot.
3. **Geometry Overrides:** nilai eksplisit yang diizinkan oleh kontrak.
4. **BodyGeometry:** geometri turunan/cache.
5. **Pose Placement:** posisi dan batas bagian tubuh sesuai geometri serta pose.
6. **Renderer:** menggambar hasil kalkulasi tanpa mengimplementasikan ulang aturan anatomi.

Nama struktur dan field final ditentukan setelah audit kode. Jangan membuat struktur baru bila tipe yang ada dapat diperluas dengan aman.

**Aturan utama:** `BodyGeometry` tidak boleh menjadi sumber kebenaran kedua. Gunakan satu jalur resmi untuk menghitung geometri dari parameter anatomi dan override.

## 8. Kriteria Penerimaan

- [ ] Variasi tinggi dan lebar tervalidasi.
- [ ] Empat preset menghasilkan perbedaan siluet yang terukur.
- [ ] Massa dan muscularity dapat dikombinasikan secara independen.
- [ ] Proporsi torso, bahu, pinggang, panggul, lengan, dan kaki dapat divariasikan.
- [ ] Anatomi wanita mendukung variasi dada dan panggul.
- [ ] Perubahan bukan hanya uniform scaling.
- [ ] Interpolasi menghasilkan geometri valid pada seluruh titik uji.
- [ ] Identitas, posisi, dan batas body parts tetap konsisten.
- [ ] Pose, injury, dan amputasi tetap konsisten.
- [ ] Input dan seed yang sama menghasilkan output deterministik.
- [ ] Validasi, normalisasi, serialisasi, dan round-trip sesuai kontrak.
- [ ] Regression tests lulus atau perubahan baseline dijustifikasi.
- [ ] C++ core menjadi satu-satunya implementasi kalkulasi anatomi resmi.
- [ ] Godot mengonsumsi hasil morfologi tanpa mengulang kalkulasi.

## 9. Strategi Pengujian

### Unit tests
- Batas minimum, default, dan maksimum setiap parameter.
- Normalisasi parameter tidak valid.
- Geometri hasil kalkulasi.
- Interaksi massa dan muscularity.
- Variasi proporsi torso, bahu, panggul, lengan, kaki, dan anatomi wanita.

### Property-based / matrix tests
- Kombinasi nilai ekstrem yang valid.
- Kombinasi massa, tinggi, lebar, dan proporsi.
- Tidak ada geometri non-finite atau di luar batas.
- Interpolasi antarprofil.
- Seed sama menghasilkan hasil sama.

### Integration tests
- Default profile → morphology → geometry → pose → render.
- Serialisasi/deserialisasi.
- Parity C API dan GDExtension.
- Injury overlay pada beragam morfologi.
- Amputasi dan cascade amputations.

### Visual regression
- Profil representatif setiap preset.
- Tinggi sama dengan proporsi berbeda.
- Massa sama dengan muscularity berbeda.
- Variasi anatomi wanita.
- Pose dan arah yang didukung.
- Perbandingan terhadap golden fixtures.

## 10. Tahapan Implementasi

### Tahap A — Audit dan kontrak parameter
- Audit `Anatomy`, `BodyGeometry`, `GeometryOverride`, dan fungsi kalkulasi.
- Identifikasi parameter yang sudah tersedia dan yang benar-benar belum ada.
- Definisikan rentang, default, unit, dan aturan interaksi.
- Tetapkan kontrak migrasi dan kompatibilitas.

**Exit criteria:** kontrak parameter disetujui dan tidak ada duplikasi sumber kebenaran.

### Tahap B — Morfologi dasar
- Implementasikan variasi tinggi dan lebar.
- Pisahkan massa tubuh dan muscularity.
- Implementasikan proporsi torso, bahu, pinggang, panggul, lengan, dan kaki.
- Tambahkan unit tests dan matrix tests.

**Exit criteria:** karakter dengan tinggi sama dapat memiliki siluet berbeda; seluruh geometri valid.

### Tahap C — Anatomi wanita
- Implementasikan parameter dada dan panggul.
- Definisikan interaksi dengan torso, massa, dan tinggi.
- Verifikasi konsistensi pose dan posisi.

**Exit criteria:** kombinasi parameter menghasilkan variasi visual yang terukur tanpa mengganggu bagian lain.

### Tahap D — Integrasi dan regression
- Integrasikan dengan pose placement.
- Verifikasi injury, amputasi, serializer, C API, dan GDExtension.
- Jalankan regression dan visual tests.
- Perbarui dokumentasi dan fixture hanya jika perubahan disengaja.

**Exit criteria:** seluruh kontrak terdampak lulus verifikasi.

## 11. Risiko dan Mitigasi

| Risiko | Mitigasi |
|---|---|
| Uniform scaling masih dominan | Parameter proporsi per bagian dan tes tinggi sama |
| Parameter saling bertentangan | Rentang, aturan interaksi, dan urutan normalisasi |
| Geometri valid tetapi visual buruk | Visual regression dengan profil representatif |
| Anatomi wanita hanya menjadi parameter tanpa efek visual | Kriteria visual terukur dan uji renderer |
| Pose/injury tidak mengikuti bentuk baru | Satu jalur kalkulasi untuk placement dan renderer |
| Serializer/API tidak kompatibel | Rancang migrasi dan uji round-trip |
| GDExtension berbeda dari core | Core sebagai satu-satunya kalkulator resmi |
| Scope melebar | Batasi pada morfologi dan kompatibilitas sistem yang ada |

## 12. Deliverables

- Kontrak parameter morfologi.
- Kalkulasi morfologi di C++ core.
- Preset anatomi dasar.
- Variasi proporsi tubuh dan anatomi wanita.
- Unit, matrix, integration, dan regression tests.
- Dokumentasi data, validasi, dan migrasi.
- Laporan verifikasi serta batasan yang tersisa.

## 13. Keputusan yang Harus Dikunci Sebelum Implementasi

1. Tinggi tubuh: nilai absolut, faktor relatif, atau keduanya?
2. Massa tubuh: parameter morfologi visual atau estimasi massa fisik? Rekomendasi: parameter visual, bukan simulasi berat fisik.
3. Anatomi wanita: tipe anatomi eksplisit atau kombinasi parameter independen? Rekomendasi: parameter relevan ditentukan oleh tipe anatomi, tetapi variasinya tetap independen.
4. Geometry override boleh mengubah semua bagian atau hanya subset tertentu?
5. Seberapa jauh renderer sprite mampu merepresentasikan dada, panggul, dan kontur torso tanpa mengganti pipeline?
6. Profil lama harus mempertahankan visual identik atau hanya kompatibilitas data? Rekomendasi: pertahankan visual lama jika memungkinkan; dokumentasikan setiap perubahan yang disengaja.

---

**Definisi selesai:** sistem menghasilkan variasi tubuh yang deterministik, jelas secara visual, tidak seragam, tervalidasi, dan kompatibel dengan body parts, pose, injury, serta amputasi.
