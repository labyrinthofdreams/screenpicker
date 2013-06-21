# Default Avisynth script
# http://avisynth.org/mediawiki/Internal_functions/Control_functions
SetMemoryMax(128)

PathToVideo = "{$source_path}"
AvisynthPluginsDir = "{$avs_plugins}"
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
Spline36Resize({$resize.width},{$resize.height})
{% endif %}

{% if crop %}
# Crop
Crop({$crop.left},{$crop.top},-{$crop.right},-{$crop.bottom})
{% endif %}

ConvertToRGB32(matrix="Rec601")
