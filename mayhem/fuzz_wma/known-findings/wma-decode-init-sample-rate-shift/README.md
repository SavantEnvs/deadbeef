`wma_decode_init` (plugins/wma/libwma/wmadeci.c:459) computes `itofix32(s->sample_rate>>1)`, which is
`(sample_rate>>1) << 16` on an `int`; any header sample rate of 65536 Hz or more overflows it (UBSan: `left shift
of 1114113 by 16 places`). The sample rate is a header field, so it is attacker-controlled.
The line above it, `high_freq*2` (458), overflows the same way for every 44.1 kHz and 48 kHz stream, which is why the
synthesized ASF seeds use 22.05 kHz; Mayhem has not reported that site.
Impact: undefined behavior (wraps in practice), giving a garbage `high_band_start`.
Upstream fix: do both computations unsigned, `(fixed32)((uint32_t)(s->sample_rate>>1) << PRECISION)` and
`(fixed32)((uint32_t)high_freq*2u)`, which keeps today's wrapped values without the UB.
Replay: `/mayhem/fuzz_wma-standalone input`.
