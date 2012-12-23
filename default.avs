# Default Avisynth script
PathToVideo = "%1"
AvisynthPluginsDir = "%2"
LoadPlugin(AvisynthPluginsDir + "/DGDecode.dll")
LoadPlugin(AvisynthPluginsDir + "/ffms2.dll")
LoadPlugin(AvisynthPluginsDir + "/TIVTC.dll")
Import(AvisynthPluginsDir + "/FFMS2.avsi")

# Load video
#MPEG2Source(PathToVideo)
FFVideoSource(PathToVideo)

# Do your IVTC / Deinterlacing here..
#TFM().TDecimate()

# PAL DVD 4:3 - 16:15
#Spline36Resize(768,576)
# PAL DVD 16:9 - 64:45
#Spline36Resize(1024, 576)
# NTSC DVD 4:3 - 8:9
#Spline36Resize(720, 540)
# NTSC DVD 16:9 - 32:27
#Spline36Resize(854,480)

# Do your cropping here..
#Crop(0, 0, -0, -0)

# Our application currently only understands RGB32
# http://avisynth.org/mediawiki/Convert
ConvertToRGB32(matrix="Rec601")
