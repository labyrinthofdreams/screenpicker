ScreenPicker - Generate and grab screenshots from video files

<img src="http://i.imgur.com/PAY74rS.png">

Requirements:

- C++11 compiler
- Qt 5.0 (might work with newer)
- avs2yuv (avs2yuv-0.24bm2)
- cpptempl (http://ginstrom.com/scribbles/2010/10/30/cpptempl-a-template-language-for-c/)
- boost 1.51.0
- Avisynth + plug-ins (DGDecode.dll, ffms2.dll, nnedi3.dll, TIVTC.dll, yadifmod.dll)
- avinfo.exe

Installation:

- Change paths to libraries in the .pro file
- Compile
- Put avinfo.exe and avisynth.dll in the app directory
- Create directory "avisynth" in the app directory and put the avisynth plug-ins there
