`junk_id3v2_read_full` (src/junklib.c:4403) reads the two frame flag bytes after the frame size
(`flags1 = readptr[0]`, `flags2 = readptr[1]`). The preceding bounds check, `readptr - tag >= size - sz`, does not
count those 2 bytes (and `size - sz` wraps when `sz > size`), so a frame header that ends exactly at the end of the tag
makes `readptr[1]` read one byte past the buffer (ASan: 1-byte heap-buffer-overflow READ, "0 bytes after" the
tag buffer from line 4335). This is the one defect of that line in the fuzz_tags run. It is not the cause of
`id3v2-small-tag-size-underflow` (4361): reverting only this fix makes only this crasher fail again.
Impact: 1-byte heap over-read for a frame header at the very end of an ID3v2 tag.
Upstream fix (one line): `if (readptr - tag + 2 + (int64_t)sz > size) {` for that check (the one in the ID3v2.3/2.4
branch, before `flags1`).
The saved crasher (`e60bf97f`) replays clean with it.
Replay: `/mayhem/fuzz_tags-standalone input`.
