# Hasil Review Cabang v0.3.3.1: Verifikasi dan Perbaikan Sebelum Merge

Proyek: Living World · Modul: Pembuat Karakter (repositori `npc-builder`) · Tanggal: 2026-10-25 · Status: Hasil review untuk ditindaklanjuti

Objek review: cabang `v0.3.3.1-geometry-contract` (commit teratas `ff17bac`), dibandingkan dengan `master` pada commit `503b766` (v0.3.3).

## 1. Ringkasan

Cabang ini memenuhi syarat utama: tidak ada piksel yang berubah. Namun dua hal belum memenuhi kontrak yang dijanjikan dalam laporan: override geometri tidak bertahan setelah simpan dan muat, dan tes `PosedPlacement` tidak membuktikan kecocokan dengan `render()`. Keduanya kecil dan jelas, sehingga sebaiknya diperbaiki di cabang yang sama sebelum merge.

## 2. Yang terbukti benar

| Klaim laporan | Hasil verifikasi |
| --- | --- |
| `ctest` lulus di Release dan Debug | Terbukti: 7 dari 7 tes lulus di kedua konfigurasi |
| Berkas acuan tidak berubah | Terbukti: `git diff tests/golden/` kosong, berkas tetap 7.884 baris |
| Perubahan visual dari v0.3.3 dicatat | Terbukti: dokumen memuat `Perubahan-Visual-001` (cedera pada bagian teramputasi dilewati, lengan berkurang dua piksel, kaki berkurang satu piksel, tangan dan kaki punya batas sendiri) |
| Dokumen tanpa singkatan | Terbukti: pemindaian singkatan berhuruf kapital pada dokumen baru bersih |
| Pemeriksaan `struct_size` Jembatan C forward-compatible | Terbukti di kode: pemeriksaan kini `<` sehingga struct yang lebih besar diterima |
| Format 4 dibaca dan geometrinya diturunkan dari anatomi | Terbukti di kode: pembaca membuang tujuh bilangan lalu memanggil `derived_geometry()` |

## 3. Masalah yang perlu diperbaiki sebelum merge

### 3.1 Override tidak bertahan setelah simpan dan muat

Pengujian yang dilakukan: profil benih 7 diberi override `shoulder_w = +2`, lalu dirender, diserialisasi, dideserialisasi, dan dirender ulang.

- Override memang mengubah gambar.
- Setelah simpan dan muat, override terbaca 0 dan gambar berbeda dari sebelum disimpan.
- Penulis serializer tidak diubah dan tidak menulis override. Pembaca juga tidak membacanya.
- Jembatan C dan ekstensi Godot juga tidak membawa field override.

Akibatnya klaim “override adalah bagian profil dan tetap deterministik” baru setengah jadi. Profil yang memiliki override akan tampil berbeda setelah dimuat ulang.

**Dua pilihan perbaikan:**

1. **Simpan override.** Ini mengubah isi format, sehingga perlu nomor format baru (format 5) dan tes migrasi.
2. **Anggap override sementara.** Hapus dari profil yang publik dan simpan sebagai celah uji internal sampai override benar-benar disimpan.

Pilihan pertama lebih tepat bila override dibutuhkan di v0.3.4. Pilihan kedua lebih tepat bila tidak.

### 3.2 Tes `PosedPlacement` tidak membuktikan kecocokan dengan `render()`

Judul tes dan komentar fungsi `compute_posed_placement` menyebut kecocokan dengan `render()`. Tes yang dibaca hanya memeriksa lebar terhadap geometri dan konstanta (misalnya lebar kepala selalu 7, tinggi 8 atau 6 saat Fall). Itu hanya konsistensi internal, bukan perbandingan dengan keluaran penggambaran. Posisi x dan y juga tidak diperiksa.

Tanpa ikatan itu, `compute_posed_placement` menjadi sumber kebenaran kedua yang bisa menyimpang secara diam-diam dari `render()`.

**Perbaikan tanpa mengubah `render()`:** untuk setiap bagian tubuh, arah, dan keadaan animasi, render cedera pada bagian itu. Ambil piksel yang berbeda dari render tanpa cedera, lalu pastikan semuanya jatuh di dalam persegi panjang `PosedPlacement`.

## 4. Catatan lebih kecil

- **Field `geo_*` di struct C** (dari v0.3.3) tampaknya kini diabaikan. Nilainya dimasukkan ke `body_geometry`, tetapi `render_profile` memakai `derived_geometry()`. Ini dari membaca kode dan belum diuji. Pemanggil C yang mengisinya tidak sadar bahwa nilainya tidak berlaku, tanpa kenaikan versi. Dokumentasikan sebagai diabaikan, atau jadikan field keluaran saja.
- **`render_internal` masih publik** di `generator.hpp`. Fungsi ini hanya dipakai oleh `render_profile`, `render`, dan tes, sehingga maksud “celah uji internal” belum sepenuhnya tercapai.
- **Integrasi berkelanjutan tiga sistem operasi belum berjalan** untuk cabang ini, dan belum ada permintaan tarik (PR). Tunggu enam kombinasi hijau sebelum merge.

## 5. Saran tindak lanjut

1. Perbaiki Bagian 3.1 dan 3.2 di cabang yang sama.
2. Dokumentasikan atau ubah perilaku field `geo_*` pada struct C.
3. Buka permintaan tarik dan tunggu integrasi berkelanjutan hijau di enam kombinasi.
4. Setelah itu merge sebagai v0.3.3.1.

## 6. Dasar verifikasi dan batasan

- Pembangunan dan `ctest` dijalankan pada klon cabang di Linux (Release dan Debug).
- Eksperimen simpan dan muat (Bagian 3.1) dijalankan dengan program uji sementara yang tidak ada di repositori.
- Isi tes `PosedPlacement` dibaca sebagian (baris 100 sampai 146 dari `tests/test_geometry.cpp`).
- Perilaku field `geo_*` pada struct C belum diuji, hanya dibaca dari kode.
