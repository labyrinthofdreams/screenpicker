# Default Avisynth script
# http://avisynth.org/mediawiki/Internal_functions/Control_functions
SetMemoryMax(128)

PathToVideo = "{$SOURCE_PATH}"
AvisynthPluginsDir = "{$AVS_PLUGINS}"
LoadPlugin(AvisynthPluginsDir + "/ffms2.dll")
LoadPlugin(AvisynthPluginsDir + "/TIVTC.dll")
LoadPlugin(AvisynthPluginsDir + "/yadifmod.dll")
LoadPlugin(AvisynthPluginsDir + "/nnedi3.dll")
Import(AvisynthPluginsDir + "/FFMS2.avsi")

# Load video
FFVideoSource(PathToVideo)

{% if deinterlace %}
# Deinterlace
YadifMod(edeint=nnedi3())
{% endif %}

{% if resize %}
# Resize
Spline36Resize({$resize.W},{$resize.H})
{% endif %}

{% if crop %}
# Crop
Crop({$crop.LEFT},{$crop.TOP},-{$crop.RIGHT},-{$crop.BOTTOM})
{% endif %}

ConvertToRGB32(matrix="Rec601")
