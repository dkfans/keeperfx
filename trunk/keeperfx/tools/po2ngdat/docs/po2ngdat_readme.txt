KeeperFX translations converter
------------------------------

This tool converts .po and .pot files, containing english strings
and their translations into, .dat format native to the game engine.

Command line options:
--verbose, -v
  Increases verbosity level (displays more details about program execution).
--output <file>,  -o <file>
  Sets output file name. The generated file will be written under that name.
--encfile <file>, -e <file>
  Sets encoding specification file. This is a text file which contains character encodings,
  ie. char_encoding_tbl_eu.txt.
--ref <string>, -r <string>
  Sets reference string. Only text messages with matching reference string are placed in DAT file.

Examples:
po2ngdat -o gtext_eng.dat -e char_encoding_tbl_eu.txt gtext_eng.pot
po2ngdat -o gtext_fre.dat -e char_encoding_tbl_eu.txt gtext_fre.po
po2ngdat -o gtext_jpn.dat -e char_encoding_tbl_jp.txt gtext_jpn.po
