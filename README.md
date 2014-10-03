plaYUVer
========

plaYUVer is an open-source video player with the following features:
- Support for libavformat and libavcodec;
- Support for wide range of raw video formats;
- Support for OpenCV imag/video processing library
- Advanced frame zoom with easy to use pan function
- Usefull information in status bar, *e.g.*, pixel information
- Usefull sidebars with stream and frame information, *e.g.*, frame histogram
- Constant growing libs for stream and frame processing (PlaYUVerStream PlaYUVerFrame)
- Advanced API for frame/video processing algorithms
- Frame level quality measurement API based on the PlaYUVerFrame class


Developers
----------------------------------------------------------------

- Joao Carreira     (jfmcarreira@gmail.com)
- Lui�s Lucas        (luisfrlucas@gmail.com) 


Builds
----------------------------------------------------------------
Download update build for windows and linux from our <a href="https://sourceforge.net/projects/playuver/">SourceForge project page</a> 


Build/Install using cmake
----------------------------------------------------------------

1. The following are required for compilation:
  - an ordinary C++ development environment (g++, make, C++ libraries and headers, ...)
  - cmake
  - a Qt development environment (libraries, headers, qmake, ...): only QtCore, QtGui and QtWidgets are required, at least in version 4

2. Dependencies:
  - QT
  - OpenCv (optional)
  - FFmpeg (optional)

3. Run the following commands: (this will create an executable named playuver) 
```
mkdir build
cd build
cmake -DUSE_FFMPEG=ON -DUSE_OPENCV=ON ..  (for FFmpeg and OpenCV support)
make
make install 
```
