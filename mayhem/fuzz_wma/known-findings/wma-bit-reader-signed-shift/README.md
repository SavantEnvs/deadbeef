The WMA bit reader (plugins/wma/libwma/ffmpeg_get_bits.h, the `ALT_BITSTREAM_READER` branch) loads its 32-bit cache
with `AV_RB32` and shifts it left by the bit offset, and every later read (`get_bits`, `get_bits1`, `get_vlc2`, ...)
shifts the cache again. The cache is a signed `int` and `AV_RB32` (plugins/wma/libwma/ffmpeg_intreadwrite.h) builds its
value from `byte[0] << 24`, which is a signed shift. Mayhem reports the first one it meets, `get_bits` at
ffmpeg_get_bits.h:420 (class `Incorrect Calculation`, UBSan `left shift of 1391601253 by 4 places cannot be
represented in type 'int'`), reached from `wma_decode_superframe_init` (wmadeci.c:1375) from `wmaplug_read`
(wma_plugin.c:228). It is application code, not the harness. The same macro is expanded at other lines of the header
(`get_vlc2` 668 is the next one the saved input reaches), so those are the same cause.
Needed together for the saved input: `AV_RB32` shifts `(uint32_t)byte[0]` (`left shift of 242 by 24 places`), and the
cache is declared `unsigned int` in `OPEN_READER` (without it `SKIP_CACHE` fails at `get_vlc2` 668). The cast in
`UPDATE_CACHE` is then unnecessary because `AV_RB32` is already unsigned.
Impact: undefined behavior on ordinary stream bytes (it wraps in practice).
Upstream fix (two lines): in ffmpeg_intreadwrite.h `(((uint32_t)((const uint8_t*)(x))[0] << 24) |`, and in
ffmpeg_get_bits.h `unsigned int name##_cache __attribute__((unused)) = 0;\`.
The saved input is `043efe1a`; it goes on to hit `wma-mult31-negative-shift` once these are fixed, so it needs both
rows' fixes to replay clean.
Replay: `/mayhem/fuzz_wma-standalone input`.
