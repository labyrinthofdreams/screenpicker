SetMemoryMax(128)

PathToVideo = "{$SOURCE_PATH}"
AvisynthPluginsDir = "{$AVS_PLUGINS}"
LoadPlugin(AvisynthPluginsDir + "/DGDecode.dll")
LoadPlugin(AvisynthPluginsDir + "/ffms2.dll")
LoadPlugin(AvisynthPluginsDir + "/TIVTC.dll")
Import(AvisynthPluginsDir + "/FFMS2.avsi")

MPEG2Source(PathToVideo)

{% if IVTC %}
TFM().TDecimate()
{% endif %}

#Spline36Resize([RESIZE_WIDTH],[RESIZE_HEIGHT])

#Crop([CROP_LEFT], [CROP_TOP], [CROP_RIGHT], [CROP_BOTTOM])

ConvertToRGB32(matrix="Rec601")
