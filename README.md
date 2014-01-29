ScreenPicker - Generate and grab screenshots from video files

<img src="http://i.imgur.com/PAY74rS.png">

Requirements:

- C++11 compiler (tested with GCC 4.8.1 on Windows)
- Qt 5.0 or newer
- avs2yuv (avs2yuv-0.24bm2) (included)
- cpptempl (http://ginstrom.com/scribbles/2010/10/30/cpptempl-a-template-language-for-c/) (included)
- Avisynth + plug-ins (DGDecode.dll, ffms2.dll, nnedi3.dll, TIVTC.dll, yadifmod.dll)
- avinfo.exe

Installation:

- Open screenpicker.pro in Qt Creator. Build->Build Project.
- Put avinfo.exe and avisynth.dll in the executable directory
- Create directory "avisynth" in the executable directory and put the avisynth plug-ins there
