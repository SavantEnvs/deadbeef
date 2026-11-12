`get_long_le` (plugins/wma/libasf/asf.c:48) returns `p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24)`, where
`p[3]` is promoted to `int`, so any top byte of 128 or more shifts into the sign bit (UBSan: `left shift of 241 by 24
places cannot be represented in type 'int'`). Any 32-bit field read with the high bit set triggers it. Run 3 has 3
defects, one per call path: `asf_get_timestamp` from `get_asf_metadata` 884 (`wmaplug_insert`), `asf_get_timestamp`
from `asf_seek` 446 (`wmaplug_seek_sample64`), and `asf_read_packet` 146 (`wmaplug_read`).
Impact: undefined behavior on an ordinary 32-bit field (it wraps in practice).
Upstream fix (one line): `((unsigned long)p[3] << 24)`.
Replay: `/mayhem/fuzz_wma-standalone input`.
