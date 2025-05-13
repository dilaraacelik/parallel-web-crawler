# Windows'ta Parallel Web Crawler Kurulumu ve Çalıştırılması

Bu dokümanda, Parallel Web Crawler projesini Windows işletim sisteminde nasıl kuracağınız ve çalıştıracağınız adım adım anlatılmaktadır.

## Gereksinimler

- [MSYS2](https://www.msys2.org/) (MinGW GCC derleyicisi ve POSIX uyumlu bir ortam sağlar)
- CMake (İsteğe bağlı)
- Git (İsteğe bağlı)

## Kurulum Adımları

### 1. MSYS2 Kurulumu

1. https://www.msys2.org/ adresinden MSYS2 kurucusunu indirin
2. İndirdiğiniz kurulum dosyasını çalıştırın ve talimatları izleyerek kurun
3. Kurulum tamamlandığında MSYS2 terminali açılacaktır
4. Paketleri güncellemek için bu komutu çalıştırın:
   ```
   pacman -Syu
   ```
5. İstenirse terminali kapatıp yeniden açın ve tekrar güncelleştirmeyi tamamlayın:
   ```
   pacman -Su
   ```

### 2. Gerekli Paketleri Yükleme

MSYS2 MINGW64 terminalini başlat menüsünden açın (MSYS2 MinGW x64) ve aşağıdaki komutu çalıştırın:

```
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-libcurl mingw-w64-x86_64-cmake git
```

### 3. Projeyi Derleme

#### Makefile Kullanarak

1. MSYS2 MINGW64 terminalini açın
2. Projenin dizinine gidin:
   ```
   cd /c/Users/Kullanici_Adiniz/Desktop/parallel-web-crawler
   ```
3. Projeyi temizleyin ve derleyin:
   ```
   mingw32-make clean
   mingw32-make
   ```

#### CMake Kullanarak (Alternatif)

1. MSYS2 MINGW64 terminalini açın
2. Projenin dizinine gidin
3. Bir build dizini oluşturun ve içine girin:
   ```
   mkdir build
   cd build
   ```
4. CMake ile projeyi yapılandırın:
   ```
   cmake -G "MSYS Makefiles" ..
   ```
5. Projeyi derleyin:
   ```
   mingw32-make
   ```

### 4. Programı Çalıştırma

MSYS2 MINGW64 terminali kullanarak şu komutu çalıştırın:

```
./bin/parallel_crawler.exe -i data/urls.txt -o data/results.csv -t 4
```

Komut satırı parametreleri:
- `-i`: Giriş dosyası (taranacak URL'lerin listesi)
- `-o`: Çıkış dosyası (sonuçların kaydedileceği CSV dosyası)
- `-t`: Kullanılacak thread sayısı

## Otomatik Kurulum

Daha kolay kurulum için `windows_setup.bat` dosyasını çalıştırabilirsiniz. Bu batch dosyası MSYS2'nin kurulu olduğunu kontrol eder ve gerekli paketleri yükler.

1. Projenin kök dizinindeyken `windows_setup.bat` dosyasını çift tıklayarak veya komut istemcisinden çalıştırın
2. Komut dosyası gerekli kontrolleri yapacak ve kurulum adımlarını gerçekleştirecektir
3. Kurulum tamamlandığında, derleme ve çalıştırma komutları görüntülenecektir

## Sorun Giderme

1. **"pthread.h: No such file or directory" hatası**:
   - MSYS2 terminalinden pthread paketini yüklediğinizden emin olun:
     ```
     pacman -S mingw-w64-x86_64-winpthreads-git
     ```

2. **"curl/curl.h: No such file or directory" hatası**:
   - MSYS2 terminalinden libcurl paketini yüklediğinizden emin olun:
     ```
     pacman -S mingw-w64-x86_64-libcurl
     ```

3. **"g++: command not found" hatası**:
   - MSYS2 MINGW64 terminalini kullandığınızdan emin olun (MSYS2 MSYS terminalini değil)
   - GCC derleyicisinin yüklü olduğunu kontrol edin:
     ```
     pacman -S mingw-w64-x86_64-gcc
     ``` 