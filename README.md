# FFmpegImpl
FFmpeg decoding &amp; encoding

## 1. Quickstart
### 1.1 Dependency
(1) [FFmpeg](https://github.com/FFmpeg/FFmpeg). Specifically, `AVCODEC, AVFORMAT, AVUTIL, AVDEVICE, SWSCALE` libraries need to be compiled.<br>
```
git clone https://github.com/FFmpeg/FFmpeg
cd FFmpeg && ./configure --enable-avcodec --enable-avformat --enable-avutil --enable-avdevice --enable-swscale 
make -j8
make install
```
(2) [OpenCV](https://github.com/opencv/opencv), you can compile or install via source/pip/brew whatever you prefer<br>

### 1.2 Compiling FFmpegImpl
```
git clone https://github.com/babyformula/FFmpegImpl
cd FFmpegImpl && mkdir build
```
Then use Cmake or Cmake GUI to produce the project in the IDE you prefer.(Xcode project recommended)

## 2. Usage
To be continued.
