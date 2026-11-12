`wmaplug_insert` (plugins/wma/wma_plugin.c:433) computes `i_length / 1000 * wfx.rate` in `int64_t`, where `i_length`
comes from the header's play duration, packet count and preroll. A large or negative header value overflows the
product (UBSan: `signed integer overflow: -2251799813685248 * 22050`).
This is the same input family as `pl-format-time-huge-duration`: a bogus header duration. They are separate
defects because they have separate fix sites; guarding the product here also stops that duration from reaching
`pl_format_time` for this plugin.
Impact: undefined behavior and a nonsense item duration.
Upstream fix: `if (__builtin_mul_overflow (i_length / 1000, (int64_t)wfx.rate, &totalsamples)) { unref; return NULL; }`.
Replay: `/mayhem/fuzz_wma-standalone input`.
