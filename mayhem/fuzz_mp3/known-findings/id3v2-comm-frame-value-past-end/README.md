Same root cause as `fuzz_tags`'s `id3v2-comm-frame-value-past-end` (`junk_load_comm_frame` (junklib.c:3668)); the cause, impact and one-line fix are in
`../../../fuzz_tags/known-findings/id3v2-comm-frame-value-past-end/README.md`. This `input` is an MP3 file whose ID3v2 tag hits it through
`cmp3_insert` -> `junk_id3v2_read`. Saved crashers of this class in run 3: `ca492d5e`; all replay clean with that fix.
Replay: `/mayhem/fuzz_mp3-standalone input`.
