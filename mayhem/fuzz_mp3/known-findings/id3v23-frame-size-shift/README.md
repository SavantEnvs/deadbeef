Same root cause as `fuzz_tags`'s `id3v23-frame-size-shift` (`junk_id3v2_read_full` (junklib.c:4388)); the cause, impact and one-line fix are in
`../../../fuzz_tags/known-findings/id3v23-frame-size-shift/README.md`. This `input` is an MP3 file whose ID3v2 tag hits it through
`cmp3_insert` -> `junk_id3v2_read`. Saved crashers of this class in run 3: `10d364c4`, `3fe7ec0d`, `7adc9fa4`, `8c4b936d`, `f62f5a3b`; all replay clean with that fix.
Replay: `/mayhem/fuzz_mp3-standalone input`.
