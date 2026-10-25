# Tanggapan atas Koreksi Rencana v0.3.3: Kompatibilitas, Cakupan Hasil Acuan, Versi, dan Kriteria Transisi

Proyek: Living World · Modul: Pembuat Karakter (repositori `npc-builder`) · Tanggal: 2026-10-25 · Status: Usulan untuk ditinjau

Dokumen ini merangkum penilaian atas empat koreksi terhadap rencana v0.3.3 dan satu catatan desain. Semua rujukan kode diverifikasi pada klon repositori terbaru.

## 1. Ringkasan penilaian

| No | Koreksi | Penilaian | Dampak pada rencana |
| --- | --- | --- | --- |
| 1 | Kompatibilitas tidak didapat otomatis dari `BodyGeometry` | Benar | Kompatibilitas dibuktikan lewat tes cedera dan amputasi |
| 2 | Cakupan hasil acuan (golden) belum jelas | Benar | Dokumentasikan isi berkas, tambah pembanding referensi sementara |
| 3 | Tiga versi jangan otomatis naik | Benar sebagian | Putuskan per desain; kode menunjukkan tiap versi punya kekakuan berbeda |
| 4 | Kriteria transisi kurang presisi | Benar | Tentukan metrik, kisi nilai, dan tabel kemonotonan per pasangan |
| 5 | `compute_geometry()` jangan menjadi fungsi besar baru | Setuju | Simpan hanya nilai dasar lintas tahap |

## 2. Kompatibilitas cedera dan amputasi

Kalimat “kompatibilitas didapat otomatis” di rencana sebelumnya terlalu kuat. Kode memiliki dua aturan yang terpisah dari geometri:

- **Amputasi:** ada 10 pemeriksaan `is_amputated` yang menentukan bagian mana digambar. Ini aturan keberadaan, bukan posisi.
- **Cedera:** loop penggambaran cedera di `core/src/generator.cpp` (baris 485 sampai 520) tidak memeriksa amputasi sama sekali, sehingga cedera tetap digambar pada bagian yang sudah diamputasi. Tangan dan kaki jatuh ke persegi panjang torso. Posisi titik cedera diundi di dalam persegi panjang bagian tubuh, jadi ukuran persegi panjang memengaruhi piksel cedera.

Konsekuensinya:

1. Refactor harus mempertahankan kedua perilaku itu apa adanya, dan mencatatnya sebagai perilaku yang diketahui. Refactor tidak memperbaikinya secara diam-diam.
2. Rumusan yang benar: `BodyGeometry` adalah sumber data geometri bersama, dan kompatibilitas dibuktikan lewat pengujian cedera dan amputasi.
3. Aturan keberadaan bagian tubuh, termasuk kaskade lengan ke tangan dan kaki ke telapak, dapat diselesaikan satu kali. Tahap gambar dan tahap cedera hanya membacanya. Pembuktiannya tetap lewat tes.

## 3. Cakupan hasil acuan

Isi `tests/golden/matriks-acuan.txt` saat ini, satu baris per kombinasi (benih, animasi, arah, waktu, skenario, sidik digital 64 bit):

| Kelompok | Jumlah baris | Kondisi uji |
| --- | --- | --- |
| `base` | 7.200 | Benih 1 sampai 100, 6 keadaan animasi, 4 arah, 3 nilai waktu |
| `inj_0` sampai `inj_5` | 60 | Satu jenis cedera per benih (benih 1 sampai 10), hanya pada Torso, keparahan 0,5, Idle, South, waktu 0,0 |
| `amp_0` sampai `amp_9` | 100 | Satu amputasi per bagian tubuh (benih 1 sampai 10), Idle, South, waktu 0,0 |

Yang tidak tercakup:

- Cedera pada kepala, lengan, atau kaki, dan cedera pada arah selain South.
- Kombinasi cedera dan amputasi, serta beberapa amputasi sekaligus.
- Profil batas. Pembangkit hanya menghasilkan tinggi 0,2 sampai 0,8, massa dan otot 0,1 sampai 0,9, serta rasio 0,45 sampai 0,55.

Soal metrik: sidik yang sama berarti piksel identik, jadi hasil acuan sudah setara dengan pembandingan tingkat piksel untuk kasus yang tercakup. Kekurangannya ada pada cakupan dan pada kemampuan menunjukkan letak selisih.

**Usulan:** pertahankan implementasi lama sebagai pembanding referensi sementara, yaitu salinan fungsi dari commit sebelum refactor di target tes terpisah. Bandingkan piksel demi piksel pada himpunan besar yang mencakup profil batas, semua bagian tubuh × cedera × amputasi, dan semua arah. Hapus pembanding itu setelah refactor selesai.

## 4. Kebijakan versi

Pernyataan “ketiganya dinaikkan” terlalu absolut. Putuskan per desain, dan wajibkan tes profil lama serta data serialisasi lama. Kode menunjukkan tiap versi punya kekakuan berbeda:

| Versi | Nilai sekarang | Perilaku di kode | Implikasi |
| --- | --- | --- | --- |
| Antarmuka C (`NPC_API_VERSION`) | 3 | `struct_size` diperiksa dengan kesetaraan ketat (`!= sizeof(...)`) di `c_api.cpp` | Menambah satu field ke struct mengubah kontrak bagi pemanggil lama, walau nomor versi tidak naik. Perlu kebijakan: melonggarkan menjadi ukuran minimal yang dikenal dengan field baru hanya di akhir, atau menaikkan versi. |
| Skema profil (`PROFILE_SCHEMA_VERSION`) | 1 | Validator menolak setiap `schema_version` yang tidak sama dengan 1 | Menaikkan skema tanpa kode migrasi membuat semua profil lama ditolak. |
| Serializer (`SERIALIZER_FORMAT_VERSION`, `PROFILE_FORMAT_VERSION`) | 2 dan 3 | Pembaca memeriksa nomor versi format | Menambah field yang disimpan hampir pasti mengubah format. Nilai turunan internal yang tidak disimpan tidak mengubahnya. |

Perbedaan ini relevan bagi desain simpangan parameter. Bila simpangan disimpan di profil (perlu untuk determinisme dan genom), ketiga versi terkait. Bila tidak disimpan, tidak.

## 5. Kriteria transisi

Rumus yang ada memberi dua contoh konkret:

- `waist_w = 6 + round(mass · 4,5 + (1 − musc) · 1,0)` turun saat otot naik, sedangkan `shoulder_w` naik. Kemonotonan karena itu tidak berlaku umum dan harus dinyatakan per pasangan (parameter, wilayah) dalam sebuah tabel.
- Suku `if (mass > 0,65f)` di baris 297 menambah sampai 2,5 piksel sekaligus. Kriteria “maksimal 1 piksel per 0,01” akan gagal tepat di ambang itu, dan itu memang tujuannya.

Rumusan yang lebih presisi: pada kisi nilai parameter tertentu, uji nilai tepat sebelum, pada, dan sesudah setiap ambang, untuk setiap wilayah yang tercantum. Metriknya adalah selisih lebar dalam piksel per langkah kisi. Kemonotonan hanya diwajibkan untuk pasangan yang memang dirancang monoton.

## 6. Catatan desain

- `compute_geometry()` sebaiknya kecil: hanya puluhan nilai dasar yang dipakai lintas tahap. Lebar per baris torso, misalnya, dihitung lewat fungsi dari nilai dasar itu dan tidak disimpan satu per satu, supaya tidak ada nilai turunan yang bisa tidak konsisten.
- Penggambaran tetap berada di fungsi `draw_*` per bagian tubuh. Kode lama tidak cukup hanya dipindahkan menjadi fungsi baru yang sama besar.
- Nama dan batas fungsi ditentukan setelah audit kode.

## 7. Tindak lanjut

Revisi dokumen rencana v0.3.3 dengan koreksi-koreksi di atas.
