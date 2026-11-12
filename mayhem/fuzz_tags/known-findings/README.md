# fuzz_tags: triage of the Mayhem defects

The 300s run 4 (`3f411bf`) reports 7 defects (run 3: 6). They are 7 distinct application sites, none in harness code;
each has a directory here with `input` and a README. `src/junklib.c` and `src/ConvertUTF/ConvertUTF.c` are upstream's tag
readers, reached from `junk_id3v2_read`. The 27 crashing inputs of run 4 map one to one onto the rows below; all 27 fail
on the committed build and all 27 replay clean once the one-line fixes below are applied together (checked on a scratch
copy, as standalone reproducers and in a single libFuzzer process; upstream is untouched; the combined diff is
`mayhem/known-findings.patch`, `git apply` it on a clone to reproduce). Reverting any one fix makes that row's crashers
fail again, so the rows are independent. This covers the crashers of run 4 only: a later run can report a cause that is
not in this table (rounds 2, 3 and 4 each did).

| Root cause | Defects | Crashers | Slug |
|---|---|---|---|
| `junk_load_comm_frame` steps one byte past the description terminator and `strlen`s uninitialized heap (junklib.c:3668) | 1 | 5 | `id3v2-comm-frame-value-past-end` |
| `ddb_iconv` hands `ConvertUTF16toUTF8` an odd byte length; the last 16-bit read overruns (ConvertUTF.c:226) | 1 | 5 | `utf16-odd-length-overread` |
| `junk_id3v2_read_full` ID3v2.3 frame size `readptr[0] << 24` shifts into the sign bit (junklib.c:4388) | 1 | 5 | `id3v23-frame-size-shift` |
| `junk_id3v2_read_full` `size - 4` wraps for a tag under 4 bytes (junklib.c:4361) | 1 | 5 | `id3v2-small-tag-size-underflow` |
| `junk_id3v2_read_full` frame flags `readptr[1]` read past the tag (junklib.c:4403) | 1 | 1 | `id3v2-frame-flags-overread` |
| `junklib_id3v2_sync_frame` reads `data[1]` past the frame (junklib.c:3612) | 1 | 5 | `id3v2-unsync-frame-overread` |
| `junk_id3v2_read_full` reads the 4-byte extended-header size before checking the tag holds 4 bytes (junklib.c:4347) | 1 | 1 | `id3v2-extheader-size-overread` |

`junk_id3v2_read_full` 4347, 4361 and 4403 are three bound errors in the same function but not one cause: they have
different bad values and different fix sites (the ext-header read, the loop condition, the frame flags), and the fix for
one alone leaves the others' crashers failing.
`fuzz_mp3` runs the same readers through the mp3 plugin; which of its defects are these is in its README
(`../../fuzz_mp3/known-findings/README.md`).
