`u8_nextchar` (src/utf8.c:238) trusts its input to be valid UTF-8 ("a valid UTF8 sequence is expected, no
validity or 0 checks are performed"). A lead byte followed by 6 or more continuation bytes makes `sz` reach 7+,
and `offsetsFromUTF8[sz-1]` indexes past the 6-entry table (UBSan: `index N out of bounds for type
'const uint32_t[6]'`). Callers then trust the returned length too: `tf_caps_impl` (src/tf.c:~815, 832) passes
`size` to `u8_toupper`/`u8_tolower`, which write it into `char temp[5]`, so a 6-byte run is a stack-buffer
overflow (ASan, `input-stack-overflow`).
Every Mayhem defect whose first frame is `u8_nextchar` (8 in run 1, 12 in run 2, reached from `tf_func_progress_impl` via
`u8_strncpy`, `tf_func_right` via `u8_strlen`, `tf_func_upper`, `tf_func_lower`, `tf_func_abbr`,
`tf_func_trim`, `tf_caps_impl`) and the 2 `u8_toupper`/`u8_tolower` stack overflows are this one root cause: invalid UTF-8
inside a script string literal or a tag value reaching a string function.

Impact: out-of-bounds table read (garbage length) and, in `$caps`, a stack buffer overflow write.
Upstream fix: bound the loop in `u8_nextchar`, `while (s[*i] && !isutf(s[*i]) && sz < 4);`, so the length
is never more than 4; with it every saved crasher of this class replays clean.
Replay: `/mayhem/fuzz_tf-standalone input` (UBSan report) and `input-stack-overflow` (ASan report).
