`junk_id3v2_read_full` (src/junklib.c:4361) loops `while (readptr - tag <= size - 4 && *readptr)` with `size` a
`uint32_t`. For a tag of 1 to 3 bytes `size - 4` wraps to about 4 billion, so the loop runs and `memcpy (frameid,
readptr, 4)` reads past the end of the `size`-byte tag buffer (ASan: 4-byte heap-buffer-overflow READ, "2 bytes
after" a 1-byte region). This is a different site and a different bad value from `id3v2-frame-flags-overread`
(4403): the fix for one does not fix the other (checked by reverting each fix alone: only that cause's crashers
fail again).
Impact: heap over-read of up to 4 bytes from any ID3v2 header declaring a tag size under 4.
Upstream fix (one line): `while (readptr - tag <= (int64_t)size - 4 && *readptr) {`.
All 5 saved crashers of this class (`4f52fbef`, `96e73244`, `bcd77e21`, `ca44f5e5`, `e48bc79c`) replay clean with it.
Replay: `/mayhem/fuzz_tags-standalone input`.
