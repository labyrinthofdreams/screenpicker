SetMemoryMax(128)

PathToVideo = "{$SOURCE_PATH}"
AvisynthPluginsDir = "{$AVS_PLUGINS}"
LoadPlugin(AvisynthPluginsDir + "/DGDecode.dll")
LoadPlugin(AvisynthPluginsDir + "/yadifmod.dll")
LoadPlugin(AvisynthPluginsDir + "/nnedi3.dll")

MPEG2Source(PathToVideo)

{% if IVTC %}
TFM().TDecimate()
{% endif %}
{% if deinterlace %}
YadifMod(edeint=nnedi3())
{% endif %}

#Spline36Resize([RESIZE_WIDTH],[RESIZE_HEIGHT])

#Crop([CROP_LEFT], [CROP_TOP], [CROP_RIGHT], [CROP_BOTTOM])

ConvertToRGB32(matrix="Rec601")
