`asf_parse_header` (plugins/wma/libasf/asfheader.c) loops forever on a header object whose size makes the
SKIP_BYTES/lseek advance stall. Not a crash: the exec runs until the Mayhemfile `timeout:`.
Replay: `timeout 10 /mayhem/fuzz_wma-standalone input`.
