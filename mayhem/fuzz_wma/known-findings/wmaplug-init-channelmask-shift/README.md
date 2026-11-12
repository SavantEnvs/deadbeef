`wmaplug_init` (plugins/wma/wma_plugin.c:160) builds the channel mask with `1 << i` for `i < fmt.channels`, and
`channels` is a 16-bit field of the stream header. From 32 channels on the shift is undefined (UBSan: `left shift
of 1 by 31 places`; bit 31 already overflows an `int`).
Impact: undefined behavior; the mask itself only has 32 bits, so channels past 31 can never be represented.
Upstream fix (one line): `for (int i = 0; i < _info->fmt.channels && i < 32; i++) { ... |= 1u << i;`.
Replay: `/mayhem/fuzz_wma-standalone input`.
