`junklib_id3v2_sync_frame` (src/junklib.c:3612) removes ID3v2 unsynchronisation (`FF 00` becomes `FF`) in place. It
looks at the byte after an `FF` only if `synced_size - written >= 2`, but `data` advances with `consumed`, which runs
ahead of `written` after the first `FF 00`, so for an `FF` as the last byte of the frame `data[1]` is read one byte past
the frame (ASan: 1-byte heap-buffer-overflow READ, "0 bytes after" the 460-byte tag buffer). This is the one
`junklib_id3v2_sync_frame` defect of the fuzz_tags run.
Impact: 1-byte heap over-read for an unsynchronised frame ending in `FF`.
Upstream fix (one line): `synced_size - consumed >= 2` in place of `synced_size - written >= 2`.
The saved crasher (`f220ab76`) replays clean with it.
Replay: `/mayhem/fuzz_tags-standalone input`.
