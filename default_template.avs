# Default Avisynth script
# For syntax see https://github.com/labyrinthofdreams/templet
# Available variables: source_path (string), avs_plugins (string),
# deinterlace (boolean), resize (boolean), resize.width/resize.height (int), 
# crop (boolean), crop.left/crop.top/crop.right/crop.bottom (int)
SetMemoryMax(128)

PathToVideo = "{$source_path}"
AvisynthPluginsDir = "{$avs_plugins}"
LoadPlugin(AvisynthPluginsDir + "/ffms2.dll")
LoadPlugin(AvisynthPluginsDir + "/TIVTC.dll")
LoadPlugin(AvisynthPluginsDir + "/yadifmod.dll")
LoadPlugin(AvisynthPluginsDir + "/nnedi3.dll")
Import(AvisynthPluginsDir + "/FFMS2.avsi")

# Load video
V = FFVideoSource(PathToVideo, -1, false)
A = FFAudioSource(PathToVideo, -1, false)

AudioDub(V, A)

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
