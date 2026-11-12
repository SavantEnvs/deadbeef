`MULT31` (plugins/wma/libwma/codeclib_misc.h:67) is `return MULT32(x,y)<<1;`, a left shift of a signed value that is
negative for half of all products (UBSan: `left shift of negative value -533039`, reached from `XNPROD31` 171,
`ff_imdct_half` mdct.c:156, `ff_imdct_calc` 480, `wma_decode_block` wmadeci.c:1295). It is not in run 4's defect list
because the bit-reader shift in `wma-bit-reader-signed-shift` (get_bits 420) stops the saved input first; it surfaced
when that one was fixed, so it is the next layer for the same input, not a separate input. It is the first of the
fixed-point decoder's shift idioms (the rest of `codeclib_misc.h` and the MDCT/FFT code use the same pattern): the
saved input only reaches this one, and others may remain.
Impact: undefined behavior in the IMDCT of any decoded WMA block (it wraps in practice).
Upstream fix (one line): `return (int32_t)((uint32_t)MULT32(x,y)<<1);`.
The saved input (`043efe1a`, the same file as `wma-bit-reader-signed-shift`) replays clean with that row's fixes and this
one; reverting only this fix makes it fail again at `MULT31`.
Replay: `/mayhem/fuzz_wma-standalone input`.
