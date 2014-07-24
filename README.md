ScreenPicker
===

A special purpose application for generating screenshots and GIFs from common video formats, including DVDs and Blu-rays. 

It works by having an Avisynth backend that provides the application with frame accurate
frame traversal letting the user navigate a video either frame by frame, jumping to any desired frame,
or by generating thumbnails from a given range making it easier to navigate the video.  

As an ease-of-use application it provides functionality to crop and resize videos, as well as deinterlacing and
inverse telecining DVDs. For advanced users there is an Avisynth script window that provides custom 
Avisynth scripting.

Its other features include saving any desired frames to disk by saving them directly, or queueing them
to be saved later by either "grabbing" the current image or by right-clicking the desired thumbnails.


<img src="http://i.imgur.com/BIb9MDB.png">

Features:

- Frame accurate frame traversal for all common containers and video formats, 
including DVDs and Blu-rays, and others supported by Avisynth
- Navigate any video manually either frame by frame, jumping to any desired frame,
or by generating thumbnails from a given range
- Save any desired frames or queue them to be saved later by "grabbing" the current image by right-clicking any desired thumbnail
- Crop and resize videos without scripting
- Deinterlace and inverse telecine DVDs and Blu-rays without scripting
- For advanced users write custom Avisynth scripts
- Create GIF image sequences from video source

Requirements for building:  
- C++11 compiler (recommended GCC >=4.8.2)
- Qt >=5.0
- avs2yuv (avs2yuv-0.24bm2) (included)
- templet (included as a submodule)

Requirements for running:  
- Avisynth 2.5.8 + plug-ins (DGDecode.dll, ffms2.dll, nnedi3.dll, TIVTC.dll, yadifmod.dll)
- avinfo.exe
- ImageMagick + gifsicle for GIFs
- DGIndex for DVDs and Blu-rays

Installation:

- git submodule init
- git submodule update
- Open screenpicker.pro in Qt Creator. Build->Build Project.
- Put avinfo.exe and avisynth.dll in the executable directory
- Create directory "avisynth" in the executable directory and put the avisynth plug-ins there

FAQ
==========
