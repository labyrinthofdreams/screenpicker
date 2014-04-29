ScreenPicker - Generate and grab screenshots from video files

Application with Avisynth backend for grabbing individual frames from common video formats.

<img src="http://i.imgur.com/PAY74rS.png">

Features:

- Frame accurate frame traversal for all common containers and video formats, 
including DVD and Blu-ray, and others supported by Avisynth
- Manually go to next or previous frame or jump to any frame via a slider
- Save any number of frames by clicking "Grab" on a selected frame or by right-clicking
a frame in the Screenshots tab and selecting save, and view them under the "Saved" tab
- Save individual frames to disk by clicking "Save" on a selected frame
- Generate any number of screenshots by specifying a frame step and the number of screenshots
- Crop, resize, and IVTC/Deinterlace without scripting via video settings window
- Create Avisynth scripts manually in the application via the built-in Avisynth Script Editor
and view results in the application after editing

Requirements:

- C++11 compiler (requires GCC >=4.8.2)
- Qt >=5.0
- avs2yuv (avs2yuv-0.24bm2) (included)
- templet (included as a submodule)
- Avisynth 2.5.8 + plug-ins (DGDecode.dll, ffms2.dll, nnedi3.dll, TIVTC.dll, yadifmod.dll)
- avinfo.exe

Installation:

- git submodule init
- git submodule update
- Open screenpicker.pro in Qt Creator. Build->Build Project.
- Put avinfo.exe and avisynth.dll in the executable directory
- Create directory "avisynth" in the executable directory and put the avisynth plug-ins there
