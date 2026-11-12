`tf_func_pad_impl` (src/tf.c:1237) accepts `argc == 1` (`if (argc < 1 || argc > 3)`) but then unconditionally
reads `arglens[1]` (the length argument, line 1254), which is past the end of the argument-length array for
`$pad(x)` and `$pad_right(x)`. ASan reports a 2-byte `dynamic-stack-buffer-overflow` READ at src/tf.c:1254.
This is the single `tf_func_pad_impl` defect in the Mayhem run.

Impact: stack out-of-bounds read of attacker-influenced size used as an expression length.
Upstream fix (one line): `if (argc < 2 || argc > 3) {`; with it every saved crasher of this class replays clean.
Replay: `/mayhem/fuzz_tf-standalone input`.
