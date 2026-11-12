# fuzz_mp3: triage of the Mayhem defects

The 300s run 4 (`3f411bf`) reports 5 defects (run 3: 4). Four are the same root causes as four of `fuzz_tags`'s seven (same
first application frame and line, same input shape: an ID3v2 tag in front of the audio, read by `junk_id3v2_read` from
`cmp3_insert`); the fifth is specific to the mp3 plugin. None is in harness code. The 16 crashing inputs of run 4 all fail
on the committed build and all replay clean once the one-line fixes below are applied (checked on a scratch copy,
standalone and in a single libFuzzer process; upstream is untouched; the combined diff is
`mayhem/known-findings.patch`, `git apply` it on a clone to reproduce). This covers the crashers of run 4 only.

| Root cause | Defects | Crashers | Slug | Same as `fuzz_tags` |
|---|---|---|---|---|
| `junk_load_comm_frame` steps one byte past the description terminator (junklib.c:3668) | 1 | 3 | `id3v2-comm-frame-value-past-end` | yes, same defect |
| `ddb_iconv` hands `ConvertUTF16toUTF8` an odd byte length (ConvertUTF.c:226) | 1 | 2 | `utf16-odd-length-overread` | yes, same defect |
| `junk_id3v2_read_full` ID3v2.3 frame size `readptr[0] << 24` (junklib.c:4388) | 1 | 5 | `id3v23-frame-size-shift` | yes, same defect |
| `junk_id3v2_read_full` `size - 4` wraps for a tag under 4 bytes (junklib.c:4361); new in run 4 | 1 | 1 | `id3v2-small-tag-size-underflow` (in `fuzz_tags`) | yes, same defect |
| `cmp3_set_extra_properties` `int` conversion of an infinite bitrate (mp3.c:130) | 1 | 5 | `mp3-bitrate-float-cast-overflow` | no, mp3 plugin only |

`fuzz_tags` has three more causes that `fuzz_mp3` did not report in run 4 (the `junk_id3v2_read_full` 4403 bound,
`junklib_id3v2_sync_frame` and the `junk_id3v2_read_full` 4347 extended-header size); the mp3 harness reaches the same
code, so they are expected to appear. 4347 is confirmed reachable: the `fuzz_tags` reproducer
(`../../fuzz_tags/known-findings/id3v2-extheader-size-overread/input`) fails in `fuzz_mp3-standalone` at junklib.c:4347
from `cmp3_insert` (mp3.c:644), and it replays clean with the `fuzz_tags` fix.
