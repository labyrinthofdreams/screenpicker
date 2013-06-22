SetMemoryMax(128)

PathToVideo = "{$source_path}"
AvisynthPluginsDir = "{$avs_plugins}"
LoadPlugin(AvisynthPluginsDir + "/DGDecode.dll")
LoadPlugin(AvisynthPluginsDir + "/TIVTC.dll")
LoadPlugin(AvisynthPluginsDir + "/yadifmod.dll")
LoadPlugin(AvisynthPluginsDir + "/nnedi3.dll")

MPEG2Source(PathToVideo)

{% if ivtc %}
TFM().TDecimate()
{% endif %}
{% if deinterlace %}
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
