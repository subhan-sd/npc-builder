---
name: dokumen-kebutuhan-bisnis
description: "Konvensi menulis dokumen kebutuhan bisnis (Business Requirements Document) untuk proyek Living World dan npc-builder, tanpa singkatan, dengan struktur baku dan kebutuhan yang bisa diuji. Gunakan setiap kali pengguna meminta membuat, merevisi, meninjau, atau merapikan dokumen kebutuhan, rencana perbaikan, spesifikasi antarmuka, atau dokumen migrasi (termasuk migrasi ke GDExtension), walau tidak menyebut kata BRD."
---

# Dokumen Kebutuhan Bisnis

Skill ini mengatur cara menulis dokumen kebutuhan di repositori `npc-builder`. Terapkan semua aturan di bawah. Skill ini hanya untuk menulis dokumen. Jangan mengubah kode sumber kecuali pengguna memintanya secara terpisah.

## 1. Pilih mode

- **Mode lengkap**: untuk perubahan besar, lintas lapisan, atau lebih dari satu tahap. Pakai 15 bagian pada Bagian 5.
- **Mode ringkas**: untuk perubahan kecil, misalnya satu fungsi atau satu perbaikan. Pakai templat pada Bagian 9.

Bila ragu, pilih mode lengkap dan sebutkan pilihan itu kepada pengguna.

## 2. Alur kerja

1. Kumpulkan fakta dari repositori sebelum menulis. Baca `README.md`, folder `docs/`, `core/`, `godot/`, `tests/`, dan `CMakeLists.txt` sesuai topik.
2. Tulis hanya fakta yang sudah diverifikasi. Setiap fakta di bagian Kondisi saat ini menyebut berkas sumbernya, dan nomor baris bila perlu.
3. Pisahkan fakta dari rencana. Jangan mencampur keduanya dalam satu kalimat.
4. Catat hal yang belum diputuskan di bagian Pertanyaan terbuka. Jangan menebak angka, versi alat, atau keputusan arsitektur.
5. Tulis dokumen sesuai struktur mode yang dipilih.
6. Jalankan pemeriksaan singkatan (Bagian 4), lalu daftar periksa (Bagian 10).
7. Simpan di `docs/kebutuhan-bisnis/AAAA-BB-HH-topik.md`, misalnya `docs/kebutuhan-bisnis/2026-10-24-integrasi-gdextension.md`. Tambahkan satu baris di Riwayat revisi.
8. Laporkan kepada pengguna: berkas yang dibuat, asumsi yang diambil, dan daftar pertanyaan terbuka.

## 3. Prinsip penulisan

1. Satu kalimat memuat satu gagasan, maksimal 25 kata.
2. Satu paragraf maksimal 4 kalimat.
3. Pakai kalimat aktif. Tulis "Sistem harus memvalidasi arah.", bukan "Validasi arah harus dilakukan."
4. Pakai angka dan satuan. Tulis "di bawah 2 milidetik", bukan "cepat".
5. Satu konsep, satu istilah. Jangan bergantian memakai istilah berbeda untuk hal yang sama.
6. Hindari kata kabur: "mungkin", "kira-kira", "dan sebagainya", "secara umum", "cukup".
7. Tulis dalam bahasa Indonesia baku dengan format Markdown. Judul maksimal tiga tingkat.

## 4. Aturan istilah dan singkatan

Tidak ada singkatan di dalam teks. Tulis bentuk lengkap dalam bahasa Indonesia. Untuk istilah asing, tulis padanan Indonesia lebih dulu, lalu istilah aslinya dalam kurung hanya pada kemunculan pertama. Contoh: sidik digital (hash).

Pengecualian hanya untuk dua hal:

- Nama diri produk dan teknologi: GDExtension, Godot, GDScript, C++20, CMake, GitHub.
- Nama berkas, fungsi, kelas, dan variabel, yang selalu ditulis dalam format kode, misalnya `render()` dan `npc_bridge.gd`.

| Singkatan | Bentuk yang dipakai di teks |
| --- | --- |
| BRD | Dokumen Kebutuhan Bisnis |
| NPC | karakter |
| API | antarmuka pemrograman |
| ABI | antarmuka biner (kesepakatan tata letak data antara dua binari) |
| CI | integrasi berkelanjutan |
| RNG | pembangkit bilangan acak |
| POD | struktur data sederhana |
| LOD | tingkat detail |
| MVP | produk minimum layak |
| UI | antarmuka pengguna |
| OS | sistem operasi |
| DLL | pustaka dinamis |
| ID | pengenal |
| seed | benih acak |
| dll., dsb. | sebutkan semua butir, atau tulis "dan seterusnya" hanya bila daftarnya memang tak terbatas |

Pemeriksaan otomatis. Perintah berikut harus tidak menghasilkan keluaran (atau hanya nama diri yang diizinkan) pada dokumen yang baru ditulis:

```bash
grep -nE '\b[A-Z]{2,}\b' docs/kebutuhan-bisnis/BERKAS.md
grep -niE '\b(dll|dsb|dst|yg|dg|utk)\b\.?' docs/kebutuhan-bisnis/BERKAS.md
```

## 5. Struktur mode lengkap

Tulis bagian dengan urutan berikut. Bagian kosong tetap dicantumkan dengan tulisan "Tidak ada."

| No | Bagian | Isi |
| --- | --- | --- |
| 1 | Ringkasan | Satu paragraf: masalah, solusi, hasil yang diharapkan |
| 2 | Latar belakang | Alasan perubahan dilakukan sekarang, dan kaitannya dengan visi Living World |
| 3 | Tujuan dan bukan-tujuan | Apa yang dicapai, dan apa yang sengaja tidak dikerjakan |
| 4 | Ruang lingkup | Daftar yang termasuk dan yang tidak termasuk |
| 5 | Kondisi saat ini | Fakta terverifikasi, masing-masing dengan rujukan berkas |
| 6 | Arsitektur sasaran | Lapisan, tanggung jawab tiap lapisan, dan arah data |
| 7 | Kebutuhan fungsional | Apa yang harus bisa dilakukan sistem |
| 8 | Kebutuhan nonfungsional | Kinerja, determinisme, portabilitas, keterujian |
| 9 | Antarmuka lintas lapisan | Tabel antarmuka sesuai Bagian 7 |
| 10 | Dampak dan risiko | Efek samping tiap perubahan beserta cara menguranginya |
| 11 | Rencana tahap | Urutan, ketergantungan, dan keluaran tiap tahap |
| 12 | Kriteria penerimaan | Daftar pernyataan ya atau tidak yang bisa diuji |
| 13 | Pertanyaan terbuka | Hal yang belum diputuskan |
| 14 | Glosarium | Semua istilah baku proyek |
| 15 | Riwayat revisi | Tanggal, versi, perubahan, alasan |

## 6. Cara menulis kebutuhan

Setiap butir punya pengenal yang tidak pernah dipakai ulang: `Fungsional-001`, `Nonfungsional-001`, `Risiko-001`, `Penerimaan-001`.

Kata kewajiban:

| Kata | Arti |
| --- | --- |
| harus | Wajib. Tanpa ini, perubahan dianggap gagal. |
| sebaiknya | Diinginkan. Boleh ditunda bila ada alasan tertulis. |
| boleh | Opsional. |
| tidak boleh | Dilarang. |

Templat satu kebutuhan:

| Kolom | Isi |
| --- | --- |
| Pengenal | Contoh: `Fungsional-004` |
| Judul | Maksimal 8 kata |
| Pernyataan | Satu kalimat dengan kata kewajiban |
| Alasan | Satu sampai dua kalimat |
| Cara verifikasi | Tes atau pemeriksaan yang menghasilkan lulus atau gagal |
| Tahap | Tahap rencana tempat kebutuhan ini dikerjakan |
| Bergantung pada | Pengenal kebutuhan lain, atau "Tidak ada" |

Contoh. Buruk: "Jembatan harus cepat dan aman dipakai Godot." Baik: "`Fungsional-004`: Jembatan C harus menolak nilai arah di luar rentang 0 sampai 3 dan mengembalikan kode galat -2 tanpa menulis ke penyangga keluaran. Cara verifikasi: tes memanggil fungsi dengan arah 9, lalu memeriksa kode galat dan memastikan penyangga tidak berubah."

## 7. Aturan khusus pekerjaan lintas lapisan

Berlaku untuk migrasi ke GDExtension dan pekerjaan lain yang melewati batas antarlapisan.

**Nama baku lapisan.** Nyatakan di bagian Arsitektur sasaran, lalu pakai konsisten. Nama bawaan:

- Inti Pembuat Karakter: kode C++20 di folder `core/`.
- Jembatan C: antarmuka berbahasa C di atas inti.
- Perluasan Godot: modul GDExtension yang dimuat oleh Godot.
- Skrip Godot: kode GDScript untuk antarmuka pratinjau.

**Tabel wajib untuk setiap antarmuka lintas lapisan.** Kolom: Nama, Arah (dari lapisan mana ke lapisan mana), Masukan (tipe, satuan, rentang sah), Keluaran, Penanganan galat (kode dan artinya), Pemilik memori (siapa mengalokasikan dan siapa membebaskan), Versi, Kompatibilitas.

**Aturan tambahan:**

1. Tandai setiap perubahan yang mengubah piksel keluaran sebagai "perubahan visual", beserta alasan dan cara memperbarui hasil acuan (golden).
2. Tulis kesamaan hasil dalam bentuk yang bisa diuji, misalnya: "Sidik digital (hash) penyangga piksel identik untuk benih acak 1 sampai 100 di Linux, Windows, dan macOS."
3. Catat setiap berkas yang terdampak dalam tabel: Sebelum, Sesudah, Tindakan (dibuat, diubah, atau dihapus).
4. Tulis batas kinerja dengan satuan: jumlah karakter per bingkai, milidetik per penggambaran, megabita memori.
5. Sebutkan versi Godot, versi godot-cpp, dan versi kompiler minimum. Ambil dari `godot/project.godot` dan `CMakeLists.txt`. Bila belum ada di repositori, taruh di Pertanyaan terbuka.

## 8. Format dan tata letak

- Tabel untuk perbandingan dan antarmuka, daftar bernomor untuk urutan langkah, daftar poin untuk hal yang tidak berurutan.
- Nama berkas, fungsi, dan nilai ditulis dalam format kode.
- Angka selalu disertai satuan. Tanggal memakai urutan tahun-bulan-tanggal, misalnya 2026-10-24.
- Setiap diagram didahului satu kalimat pengantar.
- Dokumen lama berawalan `BRD_` di `docs/` dibiarkan apa adanya. Jangan mengubah atau mengganti namanya.

## 9. Templat mode ringkas

```markdown
# <Judul perubahan>

Tanggal: AAAA-BB-HH · Versi: 1

## 1. Ringkasan
Satu paragraf: masalah, solusi, hasil yang diharapkan.

## 2. Kebutuhan
Tabel kebutuhan sesuai templat Bagian 6.

## 3. Dampak dan risiko
Efek samping perubahan dan cara menguranginya. Tandai perubahan visual bila ada.

## 4. Kriteria penerimaan
Daftar pernyataan ya atau tidak yang bisa diuji.
```

## 10. Daftar periksa sebelum menyimpan

- [ ] Tidak ada singkatan di teks, selain pengecualian pada Bagian 4, dan pemeriksaan otomatis bersih.
- [ ] Setiap istilah baru sudah ada di glosarium (mode lengkap).
- [ ] Setiap kalimat kebutuhan hanya memuat satu hal.
- [ ] Setiap kebutuhan punya cara verifikasi.
- [ ] Setiap angka punya satuan.
- [ ] Setiap fakta menyebut berkas sumbernya.
- [ ] Fakta dan rencana tidak tercampur.
- [ ] Setiap perubahan visual sudah ditandai.
- [ ] Bagian dampak dan risiko terisi.
- [ ] Hal yang belum terjawab dicatat sebagai pertanyaan terbuka, bukan ditebak.

## 11. Glosarium awal istilah baku

| Istilah | Arti |
| --- | --- |
| Karakter | Makhluk di dunia yang digerakkan simulasi, bukan pemain. Menggantikan istilah NPC. |
| Genom | Kumpulan gen karakter. Dimiliki oleh inti simulasi. |
| Penampilan | Nilai bentuk tubuh dan warna yang dihitung dari genom. Menjadi masukan penggambaran. |
| Penggambaran (render) | Proses mengubah penampilan, kondisi fisik, dan perintah animasi menjadi piksel. |
| Penyangga piksel | Larik warna berukuran 32 × 48 piksel hasil penggambaran. |
| Jembatan C | Lapisan antarmuka berbahasa C di atas inti. |
| Perluasan Godot (GDExtension) | Mekanisme Godot untuk memuat kode asli C++. |
| Hasil acuan (golden) | Keluaran tersimpan yang dibandingkan dengan keluaran baru untuk mendeteksi perubahan. |
| Sidik digital (hash) | Nilai ringkas yang mewakili isi data. |

## 12. Larangan

- Jangan mengarang hasil tes, angka kinerja, atau isi berkas yang belum dibaca.
- Jangan mengubah dokumen lama berawalan `BRD_`.
- Jangan mengubah kode sumber saat tugasnya hanya menulis dokumen.
- Jangan memakai kata kewajiban "harus" untuk hal yang belum disetujui pengguna. Pakai "sebaiknya" dan catat di Pertanyaan terbuka.
