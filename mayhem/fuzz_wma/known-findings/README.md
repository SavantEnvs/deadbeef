# fuzz_wma: triage of the Mayhem defects

The 300s run 4 (`3f411bf`) reports 20 defects (run 3: 19, run 2: 15, run 1: 9), plus one hang found locally. By first
application frame they are 11 root causes (the run-3 ten plus the bit reader's `get_bits` 420), plus the hang, and a
twelfth that the bit-reader fix exposes for the same input (`MULT31`, below). None is in harness code. Each cause has a
directory here with `input` and a README. The 72 crashing inputs of run 4 (they include those of earlier runs) map one to
one onto a cause below; all of them fail on the committed build (the 6 `wmaplug_read` inputs only in the libFuzzer binary,
which has the 2 GB limit; the 5 `stdio_seek` inputs only with `/tmp` on tmpfs) and all replay clean once the one-line
fixes in the READMEs are applied (built on a scratch copy, standalone with `/tmp` on tmpfs and in a single libFuzzer
process; upstream is untouched; the combined diff is `mayhem/known-findings.patch`, `git apply` it on a clone to
reproduce). Inputs for the same cause beyond the one saved here are listed by hash prefix in its README.

| Root cause | Defects (run 4) | Crashers | Slug |
|---|---|---|---|
| `asf_utf16LEdecode` shifts a negative value for an unpaired high surrogate (asfheader.c:357) | 8 (callers `asf_parse_header` 582/588/596/617/665/669/686/797) | 34 | `asf-utf16-unpaired-surrogate-shift` |
| `get_long_le` shifts into the sign bit (asf.c:48) | 3 (via `asf_get_timestamp` x2, `asf_read_packet`) | 12 | `asf-get-long-le-shift` |
| `wmaplug_init` `1 << i` for 32 or more channels (wma_plugin.c:160) | 1 | 3 | `wmaplug-init-channelmask-shift` |
| `wma_decode_init` `itofix32` overflows for sample rate >= 65536 (wmadeci.c:459) | 1 | 2 | `wma-decode-init-sample-rate-shift` |
| `wma_decode_init` `4 * b` overflows for sample rate about 536M or more (wmadeci.c:440); same header field as the row above, different expression and fix site | 1 | 1 | `wma-decode-init-band-position-overflow` |
| `wmaplug_insert` divides by header `packet_size`/`numpackets` of 0 (wma_plugin.c:428/430) | 1 | 5 | `wmaplug-insert-zero-packet-size-division` |
| `wmaplug_insert` duration product overflows `int64_t` (wma_plugin.c:433) | 1 | 1 | `wmaplug-insert-duration-overflow` |
| `pl_format_time` float-to-int and `mindur * 60` overflow for a huge duration (playlist.c:3169) | 1 | 2 | `pl-format-time-huge-duration` |
| `stdio_seek` `offs + offset` overflows `int64_t` (vfs_stdio.c:173); reproduces on tmpfs `/tmp` only | 1 | 5 | `stdio-seek-offset-overflow` |
| `wmaplug_read` `calloc (1, packet_size)` with a header-chosen size up to 4 GB (wma_plugin.c:215) | 1 | 6 | `wmaplug-read-packet-size-alloc` |
| bit reader cache: `AV_RB32` shifts a signed byte into the sign bit and the `int` cache shifts again (`get_bits` ffmpeg_get_bits.h:420, via `wma_decode_superframe_init` 1375) | 1 | 1 | `wma-bit-reader-signed-shift` |
| `MULT31` shifts a negative product left (codeclib_misc.h:67, via `ff_imdct_half`); not reported by Mayhem, reached by the same input once the row above is fixed | 0 | 0 (shares the input above) | `wma-mult31-negative-shift` |
| `asf_parse_header` hang (local replay only) | 0 | 0 | `asf-parse-header-hang` |

`wmaplug-insert-duration-overflow` and `pl-format-time-huge-duration` share an origin (a bogus header duration) but have
different fix sites, so they are listed separately; likewise the two `wma_decode_init` rows share the header sample rate.
The `wma_decode_init` 440 input (`b78c0693`) also reaches 459 once 440 is fixed, so it needs both fixes.

### What this table does not cover

The table maps every crasher of run 4 and nothing more. Fixing the bit reader let `043efe1a` run on into the fixed-point
decoder, where it hit `MULT31`; that decoder (`codeclib_misc.h`, `mdct.c`, `fft-ffmpeg.c`, `wmadeci.c`) is Rockbox/FFmpeg
fixed-point code that leans on signed shifts and 32-bit products, so more causes of this kind are likely to remain and a
later Mayhem run can report one. A 5-minute local libFuzzer run (4 workers) of the scratch build with every fix applied
found two such sites that are not in a Mayhem run yet and are not fixed here: `wma_decode_init` (wmadeci.c:305,
`s->nb_channels * s->sample_rate` overflows `int` for a header with a huge channel count and rate; it needs
`(int64_t)`), and `asf_seek` (libasf/asf.c:421, division by a zero `packet_size`; seen once in libFuzzer, it did not
reproduce from the saved file alone, so it is not triaged). No count of remaining causes is known.
