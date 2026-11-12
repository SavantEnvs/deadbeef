`pl_format_time` (src/playlist.c:3169) splits a float duration with `(int)floor (t / 3600)`. For a very large
duration the float-to-int cast is out of range, `hourdur` is garbage and `mindur * 60` overflows
(UBSan: `signed integer overflow: 60043380 * 60`). It is reached from `wmaplug_insert` through
`plt_set_item_duration` (the call at playlist.c:3026) with a duration computed from header fields, but the bug is in
the core formatter and would be reachable from any plugin that reports an absurd duration.
Impact: undefined behavior; the string shown for the duration is garbage.
Upstream fix (one line): clamp first, `t = roundf (fminf (t, 359999.f));`.
Replay: `/mayhem/fuzz_wma-standalone input`.
