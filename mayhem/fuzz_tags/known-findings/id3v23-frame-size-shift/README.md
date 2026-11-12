`junk_id3v2_read_full` (src/junklib.c:4388) builds the ID3v2.3 frame size as
`(readptr[3] << 0) | (readptr[2] << 8) | (readptr[1] << 16) | (readptr[0] << 24)`; `readptr[0]` is promoted to `int`,
so a top byte of 128 or more shifts into the sign bit (UBSan: `left shift of 255 by 24 places cannot be represented in
type 'int'`; Mayhem classes it as "Incorrect Calculation"). The 2.4 branch (7-bit fields, `<< 21`) is fine. The same
defect is reported by `fuzz_mp3` (5 of its 12 crashers).
Impact: undefined behavior on an ordinary 32-bit field (it wraps in practice, and the next check rejects the size).
Upstream fix (one line): `((uint32_t)readptr[0] << 24)`.
All 5 saved crashers of this class (`4d048805`, `5e8c63d6`, `76eaefe0`, `788aee27`, `ba74a383`) replay clean with it.
Replay: `/mayhem/fuzz_tags-standalone input`.
