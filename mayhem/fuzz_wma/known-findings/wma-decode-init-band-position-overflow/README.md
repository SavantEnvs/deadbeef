`wma_decode_init` (plugins/wma/libwma/wmadeci.c:440) computes the critical-band edge as
`pos = ((block_len * 2 * a) + (b << 1)) / (4 * b);` with `b = s->sample_rate`, a header field. For a sample rate of
about 536 million Hz or more `4 * b` overflows `int` (UBSan: `signed integer overflow: 4 * 578814014 cannot be
represented in type 'int'`; reached from `wmaplug_init` 142). It shares an origin with `wma-decode-init-sample-rate-shift`
(459): both come from the unvalidated header sample rate, in different expressions of the same function, and Mayhem
reports them as two defects. They are two fix sites, not one: this saved input (rate 578814014) hits 440 first, and
with only 440 fixed it goes on to hit 459, so both fixes are needed for it.
Impact: undefined behavior (wraps in practice) giving a garbage `exponent_bands` table.
Upstream fix: widen the arithmetic,
`pos = (int)((((int64_t)block_len * 2 * a) + ((int64_t)b << 1)) / (4 * (int64_t)b));` (an alternative that covers both
sites is to reject a header sample rate above 65535 in `wma_decode_init`).
The saved crasher (`b78c0693`) replays clean with both this and the 459 fix.
Replay: `/mayhem/fuzz_wma-standalone input`.
