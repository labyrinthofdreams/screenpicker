# Default Avisynth script
PathToVideo = "%1"
AvisynthPluginsDir = "%2"
LoadPlugin(AvisynthPluginsDir + "/DGDecode.dll")
LoadPlugin(AvisynthPluginsDir + "/ffms2.dll")
Import(AvisynthPluginsDir + "/FFMS2.avsi")

# Load video
# MPEG2Source(PathToVideo)
FFVideoSource(PathToVideo)

# Our application currently only understands RGB32
# http://avisynth.org/mediawiki/Convert
ConvertToRGB32()
