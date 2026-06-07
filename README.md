# 🚧 AKTİF GELİŞTİRME SÜRECİNDEDİR 🚧

Bu proje, ESP32 üzerinde VGA çıkışıyla 3D render işlemi yaparak Synthwave/Retrowave tarzı grafikler elde etmeyi amaçlayan bir motor çalışmasıdır.

## Synthwave-3D-FabGL

FabGL kütüphanesini temel alan bu proje, PS/2 klavye ve VGA monitör destekli bir donanım üzerinde gerçek zamanlı 3D model (DeLorean, Porsche vb.) çizimi yapmaktadır. Çizim teknikleri arasında tel kafes (wireframe), düz renk doldurma (flat shading) ve "Painter's Algorithm" kullanılarak Z-depth (derinlik) tabanlı yüzey sıralaması bulunmaktadır.

### Özellikler
- **320x200 75Hz VGA Çıkışı:** Çift tamponlama (double buffering) kullanılarak ekran yırtılmaları engellenmiştir.
- **Dinamik 3D Izgara (Grid):** Synthwave temasının vazgeçilmezi olan, sonsuza doğru akan ufuk çizgisi ve ızgara animasyonu.
- **Ressam Algoritması (Painter's Algorithm):** 3D modellerin yüzeyleri derinliğe (Z-depth) göre arkadan öne doğru sıralanarak ekrana basılır. Bu sayede donanımsal Z-buffer ihtiyacı ortadan kalkar.
- **Optimizasyonlar:** ESP32'nin RAM limitlerini aşmamak ve parça bazlı (fragmentation) hataları önlemek adına dinamik bellek tahsisi (`malloc`) minimuma indirilmiş, statik bellek yapıları tercih edilmiştir.

### Kontroller (PS/2 Klavye)
- **Ok Tuşları:** Aracı sağa ve sola kaydırır.
- **[0-2] Tuşları:** Modeller (Arabalar) arasında geçiş yapar.
- **[Q-P] Tuşları:** Çeşitli neon renk temaları arasında geçiş yapar.
- **[N/M] Tuşları:** Kamerayı (modeli) manuel olarak döndürür (Rotate).

### Donanım İhtiyacı
- ESP32 (VGA ve PS/2 destekli kartlar - Olimex ESP32-SBC-FabGL uyumlu)
- VGA Monitör
- PS/2 Klavye

---
*Not: Bu repo sadece ESP32 üzerinde koşan güncel kodları barındırmaktadır. 3D modellerin (obj/glb) işlenip C++ başlık dosyalarına dönüştürülme süreçlerinde kullanılan Python araçları farklı bir dizinde tutulmaktadır.*
