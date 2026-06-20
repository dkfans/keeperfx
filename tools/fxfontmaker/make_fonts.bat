
rem  unifont doesn't have 12px directly, so make it a combo of a rescaled unifont, 
rem  and a proper wenquanyi 12px font
rem  12px doesn't have regional variants, as at that size differences would be barely noticable anyways
python rescale_unifont_hex.py unifont-17.0.04.hex unifont12.hex
python bdf_to_hex.py wenquanyi_9pt.bdf wenquanyi.hex
python merge_hex.py unifont12.hex wenquanyi.hex merged12.hex

python unifont_hex_to_binary.py unifont-17.0.04.hex    font16.fxfont     16
python unifont_hex_to_binary.py unifont_jp-17.0.04.hex font16_JPN.fxfont 16
python unifont_hex_to_binary.py unifont_t-17.0.04.hex  font16_CHT.fxfont 16
python unifont_hex_to_binary.py merged12.hex           font12.fxfont     12
pause