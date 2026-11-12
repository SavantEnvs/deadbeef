`tf_import_legacy` (src/tf.c:4293-4296) guards the newline expansion with `if (outsize < 7) break;` and then
`strcpy (out, "$crlf()")`, which writes 8 bytes including the NUL. With exactly 7 bytes left it writes one byte
past the output buffer (ASan: stack-buffer-overflow WRITE at src/tf.c:4296). The reproducer is 4 plain bytes
followed by 292 newlines, which leaves `outsize == 7` for the 292nd newline of the harness's 2048-byte buffer.

Impact: 1-byte stack overflow write (NUL) in the legacy-format importer, which every caller feeds a
fixed-size buffer.
Upstream fix (one line): `if (outsize < 8) {` in the `'\n'` branch; with it the saved crasher replays clean.
Replay: `/mayhem/fuzz_tf-standalone input`.
