`junk_id3v2_read_full` (src/junklib.c:4347) reads the 4-byte extended-header size
`uint32_t sz = (readptr[3] << 0) | (readptr[2] << 7) | (readptr[1] << 14) | (readptr[0] << 21);` as soon as the
header's extended-header flag is set, before anything checks that the tag holds 4 bytes. The saved input is an ID3v2.3
header with flags `0xd0` and a tag size of 3, so `readptr[3]` reads one byte past the 3-byte `malloc` (ASan:
heap-buffer-overflow READ of size 1, reached from `junk_id3v2_read` 4627; `fuzz_mp3` reaches it the same way through
`cmp3_insert`). The check right after it (`size < sz`) cannot help, because it runs on the value that was already read.
This is a third bound error in the same function, with a different site and a different bad value from
`id3v2-small-tag-size-underflow` (4361: `size - 4` wraps) and `id3v2-frame-flags-overread` (4403): each fix alone leaves
the other crashers failing.
Impact: heap over-read of up to 4 bytes from any ID3v2 header that sets the extended-header flag and declares a tag
size of 1 to 3.
Upstream fix (3 lines, in the `if (extheader) {` block, before the read): `if (size < 4) { goto error; }`.
The saved crasher (`33d5ab85`) replays clean with it; reverting only this fix makes it fail again at 4347.
Replay: `/mayhem/fuzz_tags-standalone input` (also `/mayhem/fuzz_mp3-standalone input`).
