`asf_utf16LEdecode` (plugins/wma/asfheader.c:357) decodes a UTF-16 surrogate pair as
`... | ((utf16[3] - 0xDC) << 8)` without checking that `utf16[3]` is a low surrogate. When the second code unit
is not in 0xDC..0xDF the subtraction is negative and the left shift is undefined (UBSan: `left shift of negative
value -50`). 8 Mayhem defects of run 3 (7 in run 2) are this one cause; they differ only in the caller, `asf_parse_header` at
582/588/596/617/665/669/686/797, one per metadata string it decodes.
Impact: undefined behavior, and a garbage code point passed to `utf8encode`.
Upstream fix (one line): `if (n < 4 || utf16[3] < 0xDC || utf16[3] >= 0xE0) {` before the `ucs =` expression, so an
unpaired high surrogate ends the string like a truncated one. With it every saved crasher of this class replays clean.
Replay: `/mayhem/fuzz_wma-standalone input`.
