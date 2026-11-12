`ddb_iconv` (src/junklib.c:573) converts UTF-16LE/UCS-2LE text with
`ConvertUTF16toUTF8 (&in, in + inlen, ...)`. The converter reads whole 16-bit units while `source < sourceEnd`, so an
odd `inlen` (an ID3v2 text frame whose payload has an odd number of bytes after the BOM) makes its last read take one
byte past the end of the input (ASan: 2-byte heap-buffer-overflow READ at src/ConvertUTF/ConvertUTF.c:226, in the
buffer from `_get_aligned_copy`). This is the one `ConvertUTF16toUTF8` defect of the fuzz_tags run, and the same defect
`fuzz_mp3` reports; `ConvertUTF.c` itself is correct for an even length.
Impact: 1-byte heap over-read per odd-length UTF-16 string in a tag.
Upstream fix (one line): `inlen &= ~1;` at the top of the `UTF-16LE`/`UCS-2LE` branch of `ddb_iconv`.
All 5 saved crashers of this class (`21c75f1a`, `a5a9b68d`, `aed5708d`, `dc914daf`, `f7384cf6`) replay clean with it.
Note: the `UTF-16BE` branch below it swaps bytes in pairs with the same assumption (`temp[i+1]` for odd `inlen`); Mayhem
has not reported it.
Replay: `/mayhem/fuzz_tags-standalone input`.
