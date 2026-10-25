# Stabilisasi Kontrak Geometri, Kompatibilitas, dan Fondasi Pakaian

Tanggal: 2026-10-25 · Versi: 1

## 1. Ringkasan

Penyimpanan geometri tubuh di profil memicu risiko ketidakselarasan data dan dua sumber kebenaran pada versi sebelumnya. Dokumen ini menetapkan seluruh nilai geometri sebagai turunan murni dari anatomi dan simpangan override, mengubah verifikasi ukuran struktur jembatan menjadi ramah pengembangan masa depan, serta mendefinisikan tipe fondasi pakaian tanpa mengubah piksel keluaran hasil acuan. Perubahan ini menjamin kompatibilitas pembacaan data serial, menjaga 7.884 baris hasil acuan tetap identik, dan menyediakan fondasi modular untuk sistem perlengkapan masa depan.

## 2. Latar belakang

Versi sebelumnya memperkenalkan penyimpanan langsung tujuh nilai geometri tubuh ke dalam profil karakter dan penyusun serial format 4. Hal tersebut menciptakan risiko divergensi karena jalur normalisasi menghitung ulang geometri sedangkan jalur lain membaca nilai mentah. Selain itu, verifikasi ukuran struktur pada antarmuka biner bersifat kaku terhadap penambahan bidang baru di masa depan. Fondasi penempatan pakaian modular membutuhkan kontrak koordinat berpose yang stabil tanpa mengubah tampilan visual yang sudah disetujui.

## 3. Tujuan dan bukan-tujuan

Tujuan rilis ini adalah:
- Menyatukan sebelas nilai geometri tubuh dalam satu fungsi kalkulasi kanonikal murni.
- Mendukung simpangan override eksplisit sebagai bagian profil dengan nilai bawaan nol.
- Menjadikan verifikasi ukuran struktur pada antarmuka C toleran terhadap penambahan bidang di akhir.
- Mendefinisikan tipe data penempatan bagian tubuh berpose dan urutan lapisan untuk fondasi pakaian.
- Memastikan tidak ada satu piksel pun yang berubah pada 7.884 baris matriks hasil acuan.

Bukan-tujuan rilis ini adalah:
- Mengimplementasikan sistem pakaian atau peralatan aktif.
- Memindahkan kalkulasi pose atau persegi panjang cedera dari fungsi penggambaran saat ini ke struktur baru.
- Mengubah susunan piksel atau warna pada matriks hasil acuan yang ada.

## 4. Ruang lingkup

Ruang lingkup yang termasuk:
- Berkas `core/include/npc/body_geometry.hpp`, `profile.hpp`, `serializer.hpp`, `generator.hpp`, `c_api.h`, `types.hpp`.
- Berkas implementasi `core/src/generator.cpp` dan `core/src/c_api.cpp`.
- Pengujian unit pada `tests/test_geometry.cpp`, `test_serializer.cpp`, `test_profile.cpp`, `test_c_api.cpp`.

Ruang lingkup yang tidak termasuk:
- Pembuatan aset visual pakaian baru.
- Modifikasi logika shader atau antarmuka editor Godot di luar verifikasi biner.

## 5. Kondisi saat ini

- Berkas `core/include/npc/body_geometry.hpp` sebelumnya hanya menghitung tujuh nilai lebar secara parsial tanpa mencakup tinggi total, panjang kaki, tinggi badan, dan panjang lengan.
- Berkas `core/src/generator.cpp` baris 117 sampai 127 sebelumnya menghitung empat dimensi vertikal secara terpisah di dalam fungsi penggambaran.
- Berkas `core/src/c_api.cpp` memeriksa ukuran struktur dengan kesetaraan ketat `struct_size != sizeof(...)` yang menolak struktur versi masa depan.
- Berkas `core/include/npc/serializer.hpp` membaca dan menyimpan format 4 yang mengunci tujuh nilai geometri mentah.
- Berkas `tests/golden/matriks-acuan.txt` berisi 7.884 baris sidik digital yang menjadi penentu kebenaran visual.

## 6. Arsitektur sasaran

Arsitektur modul terdiri dari empat lapisan berurutan:
- Inti Pembuat Karakter: Berisi logika kalkulasi geometri murni, validasi profil, dan penggambaran kanonikal pada C++20.
- Jembatan C: Antarmuka biner ramah kompatibilitas dengan penanganan ukuran struktur fleksibel.
- Perluasan Godot: Pengikat asli GDExtension untuk konsumsi kelas karakter oleh Godot.
- Skrip Godot: Pengendali antarmuka dan pengujian visual otomatis.

Aliran data geometri mengalir dari parameter anatomi ditambah simpangan override menuju fungsi kalkulasi kanonikal, lalu menghasilkan geometri efektif yang dikonsumsi oleh fungsi penggambaran dan penempatan berpose.

## 7. Kebutuhan fungsional

| Pengenal | Judul | Pernyataan | Alasan | Cara verifikasi | Tahap | Bergantung pada |
| --- | --- | --- | --- | --- | --- | --- |
| Fungsional-001 | Kalkulasi geometri terpadu | Sistem harus menghitung sebelas dimensi geometri tubuh secara terpadu dalam satu fungsi kanonikal murni. | Mencegah inkonsistensi antara dimensi horizontal dan vertikal. | Tes unit memeriksa kesesuaian nilai terhitung dengan rumus acuan untuk 100 benih acak. | 1 | Tidak ada |
| Fungsional-002 | Batas nilai terpusat | Sistem harus mendefinisikan batas nilai geometri satu kali pada konstanta geometri kanonikal. | Menghilangkan duplikasi angka batas di berbagai berkas. | Pemeriksaan kode dan pengujian validasi profil terhadap batas kanonikal. | 1 | Fungsional-001 |
| Fungsional-003 | Simpangan override profil | Sistem harus mendukung struktur simpangan bilangan bulat pada profil yang bernilai netral saat bernilai nol. | Memungkinkan penyesuaian khusus tanpa merusak determinisme perhitungan. | Tes unit memverifikasi bahwa override nol menghasilkan geometri yang identik dengan rumus dasar. | 1 | Fungsional-001 |
| Fungsional-004 | Tanda tangan publik bersih | Sistem harus menyembunyikan parameter penimpaan geometri dari antarmuka publik dan menjadikannya fungsi internal. | Menjaga kerapian antarmuka pemrograman publik agar profil menjadi sumber tunggal. | Pemeriksaan berkas tajuk pustaka dan pemanggilan fungsi publik. | 1 | Fungsional-003 |
| Fungsional-005 | Kompatibilitas pembaca serial | Pembaca serial format 4 harus menerima data lama dan menurunkan ulang nilai geometri dari anatomi. | Memastikan data tersimpan versi sebelumnya tetap dapat dimuat dengan geometri kanonikal. | Tes deserialisasi string format 4 memverifikasi nilai geometri diturunkan secara kanonikal. | 2 | Fungsional-001 |
| Fungsional-006 | Ukuran struktur toleran masa depan | Jembatan C harus mengizinkan ukuran struktur yang lebih besar dari ukuran dasar yang diketahui. | Menjamin kompatibilitas biner saat penambahan bidang baru di akhir struktur. | Tes antarmuka C memanggil fungsi dengan ukuran struktur lebih besar dan memastikan status berhasil. | 2 | Tidak ada |
| Fungsional-007 | Tipe fondasi penempatan berpose | Sistem harus menyediakan definisi tipe penempatan berpose, titik jangkar, dan urutan lapisan tubuh. | Menjadi fondasi modular untuk pengembangan sistem pakaian pada rilis berikutnya. | Tes unit memverifikasi koordinat batas pada tipe baru cocok dengan koordinat penggambaran. | 3 | Fungsional-001 |

## 8. Kebutuhan nonfungsional

| Pengenal | Judul | Pernyataan | Alasan | Cara verifikasi | Tahap | Bergantung pada |
| --- | --- | --- | --- | --- | --- | --- |
| Nonfungsional-001 | Invarian visual mutlak | Sistem harus menghasilkan keluaran piksel yang identik dengan matriks acuan tanpa perbedaan satu piksel pun. | Menjaga stabilitas visual yang telah disetujui pada rilis sebelumnya. | Pemeriksaan perbedaan git pada berkas matriks acuan harus kosong dan pengujian hasil acuan lulus. | 1 | Tidak ada |
| Nonfungsional-002 | Kinerja kalkulasi geometri | Kalkulasi geometri tubuh harus selesai di bawah 1 mikrodetik per karakter. | Mendukung pembentukan ratusan karakter secara langsung tanpa menurunkan frekuensi gambar. | Pengujian tolok ukur mikro di lingkungan uji. | 1 | Fungsional-001 |
| Nonfungsional-003 | Determinisme lintas platform | Hasil kalkulasi geometri harus menghasilkan bilangan bulat yang identik pada arsitektur Linux dan Windows. | Mencegah perbedaan perilaku simulasi antarpemain di sistem operasi berbeda. | Eksekusi rangkaian tes integrasi berkelanjutan pada Linux dan Windows. | 2 | Fungsional-001 |

## 9. Antarmuka lintas lapisan

| Nama | Arah | Masukan | Keluaran | Penanganan galat | Pemilik memori | Versi | Kompatibilitas |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `compute_full_geometry` | Lapisan dalam Inti | Empat nilai desimal anatomi dan struktur override | Objek struktur geometri tubuh lengkap | Nilai masukan dibatasi ke rentang sah | Pemanggil (di tumpukan memori) | 1 | Kompatibel penuh |
| `npc_profile_validate` | Luar ke Inti | Penunjuk ke struktur profil C | Kode bilangan bulat | Mengembalikan kode galat bila ukuran struktur lebih kecil dari ukuran minimal | Pemanggil | 4 | Ramah penambahan masa depan |
| `npc_profile_render` | Luar ke Inti | Penunjuk profil C, daftar cedera, parameter animasi, penyangga piksel | Kode bilangan bulat | Mengembalikan kode galat bila penyangga kurang dari 6.144 bita | Pemanggil menyediakan penyangga | 4 | Ramah penambahan masa depan |

## 10. Dampak dan risiko

| Pengenal | Risiko | Dampak | Mitigasi |
| --- | --- | --- | --- |
| Risiko-001 | Perubahan koordinat pada penempatan bagian tubuh | Berpotensi menggeser piksel pakaian atau cedera di masa depan. | Mengunci invarian melalui pengujian unit yang memverifikasi kesamaan koordinat terhadap logika penggambaran kanonikal. |
| Risiko-002 | Struktur C menerima ukuran lebih besar secara tidak aman | Pembacaan memori liar bila pemanggil mengirim pointer tidak valid. | Hanya membaca bidang sesuai batas ukuran yang diketahui pustaka saat ini. |
| Perubahan-Visual-001 | Penanganan cedera amputasi dan dimensi baru pada v0.3.3 | Cedera pada bagian teramputasi kini dilewati, lengan berkurang dua piksel, kaki berkurang satu piksel, telapak tangan dan kaki memiliki batas sendiri. | Seluruh perubahan visual telah dikunci ke dalam 7.884 baris matriks acuan pada rilis v0.3.3 dan diverifikasi ulang tanpa perbedaan pada rilis ini. |

## 11. Rencana tahap

1. Tahap 1: Pembentukan batas terpusat, fungsi kalkulasi kanonikal 11 dimensi, dan tipe simpangan override di Inti Pembuat Karakter.
2. Tahap 2: Penyesuaian deserializer format serial lama serta pembaruan kebijakan ukuran struktur pada Jembatan C.
3. Tahap 3: Pendefinisian tipe data fondasi pakaian, penempatan berpose, dan pengujian keselarasan koordinat.
4. Tahap 4: Dokumentasi menyeluruh, verifikasi matriks acuan, dan pengujian mutasi kegagalan terkendali.

## 12. Kriteria penerimaan

- [x] `Penerimaan-001`: Seluruh 7.884 baris pada berkas `tests/golden/matriks-acuan.txt` tetap identik tanpa perbedaan satu karakter pun.
- [x] `Penerimaan-002`: Pengujian unit `test_geometry` membuktikan bahwa 11 nilai geometri cocok dengan rumus kanonikal untuk 100 benih acak.
- [x] `Penerimaan-003`: Pengujian deserialisasi membuktikan data format 4 menurunkan kembali geometri secara kanonikal.
- [x] `Penerimaan-004`: Pengujian jembatan C membuktikan struktur dengan ukuran lebih besar dari ukuran minimal diterima dengan kode status sukses.
- [x] `Penerimaan-005`: Tipe data `PosedPlacement` terbukti cocok dengan koordinat penggambaran melalui pengujian otomatis.

## 13. Pertanyaan terbuka

Tidak ada.

## 14. Glosarium

| Istilah | Arti |
| --- | --- |
| Karakter | Entitas makhluk hidup di dalam dunia simulasi. |
| Penampilan | Data visual wajah, warna, dan proporsi tubuh karakter. |
| Geometri tubuh | Kumpulan ukuran piksel untuk bagian-bagian tubuh karakter. |
| Simpangan override | Nilai pergeseran relatif bilangan bulat yang diterapkan di atas nilai dasar geometri. |
| Penempatan berpose | Posisi persegi panjang dan titik jangkar bagian tubuh pada satu bingkai animasi tertentu. |
| Penyangga piksel | Ruang memori berisi susunan warna piksel berukuran 32 kali 48 piksel. |
| Hasil acuan | Berkas catatan nilai kebenaran visual yang dijadikan patokan kelulusan uji regresi. |
| Sidik digital | Nilai ringkas penanda keunikan data piksel. |

## 15. Riwayat revisi

| Tanggal | Versi | Perubahan | Alasan |
| --- | --- | --- | --- |
| 2026-10-25 | 1 | Pembuatan dokumen spesifikasi stabilisasi geometri dan fondasi pakaian | Menegakkan kontrak geometri tunggal, kompatibilitas ke depan, dan pencatatan perubahan visual |
