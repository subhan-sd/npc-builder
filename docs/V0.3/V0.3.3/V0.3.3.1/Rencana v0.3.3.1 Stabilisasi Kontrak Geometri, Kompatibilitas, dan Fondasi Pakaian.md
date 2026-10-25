# Rencana v0.3.3.1: Stabilisasi Kontrak Geometri, Kompatibilitas, dan Fondasi Pakaian

Proyek: Living World · Modul: Pembuat Karakter (repositori `npc-builder`) · Tanggal: 2026-10-25 · Status: Usulan untuk ditinjau

Dasar analisis: cabang `master` setelah merge PR #3 (commit `503b766`, v0.3.3).

## 1. Keputusan

Tiga pertimbangan (kontrak geometri dan keterbukaan, kontrak kompatibilitas dan regresi, fondasi pakaian modular) cocok digabung menjadi satu rilis v0.3.3.1, dengan satu invarian: **tidak ada piksel yang berubah**. Berkas hasil acuan (golden) yang berisi 7.884 baris harus tetap identik. Perubahan yang menggeser satu piksel saja bukan bagian v0.3.3.1. Sistem pakaian tidak diimplementasikan pada rilis ini.

## 2. Kontrak geometri: simpan atau hitung

### 2.1 Empat nilai yang diaudit

Rumus di `core/src/generator.cpp`, fungsi `render()` (baris 117 sampai 127):

| Nilai | Rumus saat ini | Kesimpulan |
| --- | --- | --- |
| `total_height` | `36 + round(height × 6)` | Turunan murni, tidak perlu disimpan |
| `leg_len` | `13 + round(leg_ratio × 4)` | Turunan murni, tidak perlu disimpan |
| `torso_h` | `total_height − 8 − 2 − leg_len`, minimal 11 | Turunan murni, tidak perlu disimpan |
| `arm_len` | `torso_h + 2` | Turunan murni, tidak perlu disimpan |

Saran: seluruh tujuh nilai lebar di `BodyGeometry` ditambah empat nilai di atas dihitung dalam satu fungsi dan tidak disimpan.

### 2.2 Geometri diam dan penempatan berpose

Pada keadaan Fall, `render()` menimpa nilai-nilai itu (`total_height = 14`, `head_h = 6`, `torso_h = 5`, `leg_len = 3`, baris 163 sampai 166). Animasi juga menambah `leg_dy`, `arm_dy`, dan `torso_lean_x`. Karena itu ada dua hal yang berbeda:

- **Geometri diam**: turunan dari profil, deterministik, tidak bergantung animasi.
- **Penempatan berpose**: hasil geometri diam digabung dengan `AnimationCommand`.

### 2.3 Override eksplisit

Override eksplisit bisa didukung tanpa mengorbankan determinisme, dengan syarat override adalah bagian dari profil yang disimpan dan bukan parameter terpisah.

- Bentuknya simpangan bilangan bulat yang dibatasi, bernilai nol bila tidak ada. Nol berarti sama dengan rumus.
- Geometri efektif adalah fungsi murni dari profil.
- Parameter `const BodyGeometry* override_geo` pada `render()` sebaiknya menjadi celah uji internal, bukan tanda tangan publik.
- Batas nilai (misalnya bahu `[9, 18]`) didefinisikan satu kali di `body_geometry.hpp` dan dipakai bersama oleh validator dan tes, bukan ditulis ulang di validator.

## 3. Kontrak kompatibilitas dan regresi

### 3.1 Risiko yang ada sekarang

Geometri kini disimpan di profil, serializer (format 4), dan struktur C (`NPC_API_VERSION` 4). Di jalur normalisasi, geometri dihitung ulang tanpa `if` (`core/include/npc/profile.hpp` baris 476, walau komentarnya menyebut “jika dinormalisasi”). Jalur yang memanggil `render_profile` langsung memakai nilai tersimpan apa adanya. Hasilnya dua sumber kebenaran untuk lebar yang sama.

### 3.2 Aturan yang disarankan bila geometri kembali menjadi turunan

1. Pembaca tetap menerima format 4 dan membuang nilai geometrinya. Tes: string format 4 yang geometrinya tidak cocok dengan anatomi harus menghasilkan keluaran turunan.
2. Putuskan kebijakan Jembatan C sekarang. Ubah `struct_size` dari kesetaraan ketat menjadi “ukuran minimal yang dikenal, field baru hanya ditambahkan di akhir”. Selama kesetaraan ketat dipertahankan, setiap penambahan field memecahkan pemanggil lama.
3. Kenaikan versi (skema, format, antarmuka C) hanya dilakukan bila ada alasan tertulis dan tes kompatibilitas yang menyertainya.
4. Hasil acuan, validasi, dan aturan migrasi serializer yang ada tetap dipertahankan.

## 4. Fondasi pakaian modular

Pakaian, zirah, dan perlengkapan memerlukan tiga hal. Geometri diam saja tidak cukup.

1. **Geometri diam** (Bagian 2).
2. **Penempatan berpose**: persegi panjang dan titik jangkar tiap bagian tubuh untuk satu bingkai, termasuk simpangan animasi dan keadaan Fall.
3. **Keberadaan bagian tubuh** (amputasi, termasuk kaskade) dan **urutan lapisan** yang eksplisit.

Cedera saat ini masih memakai variabel lokal di `render()`, sehingga cedera adalah konsumen pertama yang wajar untuk kontrak yang sama. Opsi untuk nanti: peta pengenal bagian tubuh per piksel, agar pakaian bisa dipotong sesuai bagian tubuh.

## 5. Pembagian pekerjaan

| Pekerjaan | Rilis |
| --- | --- |
| Keputusan simpan atau hitung, override sebagai bagian profil, aturan versi dan migrasi, satu sumber batas nilai | v0.3.3.1 |
| Definisi struct penempatan dan keberadaan (tipe, dokumen, tes bahwa persegi panjang cocok dengan yang dipakai `render()`) | v0.3.3.1 |
| Memindahkan perhitungan pose dan persegi panjang cedera ke struct itu (berisiko mengubah piksel) | Rilis tersendiri |
| Implementasi sistem pakaian | Bukan bagian v0.3.3.1 |

## 6. Catatan dan batasan

- Status integrasi berkelanjutan untuk merge v0.3.3 belum diperiksa.
- Perilaku cedera pada bagian tubuh yang diamputasi (kini dilewati) dan persegi panjang cedera yang berubah masih perlu dicatat sebagai perubahan visual.
- Semua rujukan baris diverifikasi pada commit `503b766`.
