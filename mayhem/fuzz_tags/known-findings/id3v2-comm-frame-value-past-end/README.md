`junk_load_comm_frame` (src/junklib.c:3668) splits a decoded COMM frame into description and value. When the decoded
text has no NUL separator inside its `outsize` bytes, the scan stops on the terminator at `descr[outsize]` and the code
then does `value++`, so `value` points one byte past the terminator, into the unused tail of the `malloc` buffer that
`convstr_id3v2` returned (it is not zeroed). `strlen (value)` then reads uninitialized bytes until it leaves the
allocation (ASan: heap-buffer-overflow READ in `strlen`, "0 bytes after" the `sz*4+1` buffer from `convstr_id3v2`).
This is the one `junk_load_comm_frame` defect of the fuzz_tags run, and the same defect `fuzz_mp3` reports.
Impact: heap over-read driven by one COMM frame in an ID3v2 tag (`strlen` over uninitialized memory, then a copy of
it into the `comment` VLA).
Upstream fix: remember the decoded length and only step over a separator that exists,
`int descr_len = outsize;` before the scan and `if (value - descr < descr_len) { value++; }` after `*value = 0;`.
All 5 saved crashers of this class (`0c0770ba`, `778ffb2b`, `7c965dc6`, `f032f319`, `fae37675`) replay clean with it.
Replay: `/mayhem/fuzz_tags-standalone input`.
