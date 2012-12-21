ifdef SKIP_LOWLEVEL_INIT
# SDRAM version
TEXT_BASE = 0x80000000
else
# ROM version
TEXT_BASE = 0x9f000000
endif


