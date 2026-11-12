Same root cause as `fuzz_tags`'s `utf16-odd-length-overread` (`ConvertUTF16toUTF8` (ConvertUTF.c:226)); the cause, impact and one-line fix are in
`../../../fuzz_tags/known-findings/utf16-odd-length-overread/README.md`. This `input` is an MP3 file whose ID3v2 tag hits it through
`cmp3_insert` -> `junk_id3v2_read`. Saved crashers of this class in run 3: `9e29e62b`; all replay clean with that fix.
Replay: `/mayhem/fuzz_mp3-standalone input`.
