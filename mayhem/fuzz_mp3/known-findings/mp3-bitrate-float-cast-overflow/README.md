`cmp3_set_extra_properties` (plugins/mp3/mp3.c:130) computes `int bitrate = mp3info->datasize * 8 / dur / 1000;` with
`dur` a `double` item duration. When the duration is 0 (a stream with a data size and a computed duration of zero)
the quotient is `inf`, and the conversion to `int` is undefined (UBSan: `inf is outside the range of representable
values of type 'int'`; Mayhem classes it as "Numeric Truncation Error"). This is the one defect of this site; it is
specific to the mp3 plugin and is not reached by `fuzz_tags`.
Impact: undefined behavior (a garbage `:BITRATE` property) on a crafted MP3 file; in practice `INT_MIN`.
Upstream fix: `double br = mp3info->datasize * 8 / dur / 1000; int bitrate = (br >= 0 && br < INT_MAX) ? (int)br : 0;`.
All 5 saved crashers of this class (`22e0d70f`, `451d5f35`, `64f66b6f`, `9156809a`, `dab59261`) replay clean with it.
Replay: `/mayhem/fuzz_mp3-standalone input`.
