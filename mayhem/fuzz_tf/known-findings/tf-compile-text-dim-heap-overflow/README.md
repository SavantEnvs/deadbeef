`tf_compile` (src/tf.c) sizes its output buffer as `calloc (len * 3, 1)`, but several constructs emit more than
3 output bytes per source byte, so the compiler writes past the end of the heap buffer. The single byte `<`
(a dim marker with no operand) is the smallest case: `tf_compile_text_dim` emits a 7-byte header
(`0 5 <amount> <len32>`) for one source byte, and ASan reports a heap-buffer-overflow write at src/tf.c:4158.
`[` (ifdef, 6-byte header), `%field%` and `$func(` headers expand the same way, which is why Mayhem reports the
same defect from `tf_compile_ifdef` (4068, 4069), `tf_compile_field` (4009), `tf_compile_func` (3902) and
`tf_compile_plain` (4235, the plain-byte copy that runs after the output pointer is already past the buffer).
All seven heap-overflow defects of the first 300s run are this one root cause.

Impact: heap overflow write in the title-format compiler; the script is user-controlled (config, plugin
settings, tf playlist columns).
Upstream fix (one line): `uint8_t *code = calloc (len * 16 + 64, 1);` in `tf_compile`; with it every
saved crasher of this class replays clean.
Replay: `/mayhem/fuzz_tf-standalone input`.
Run 4 adds `tf_compile_func` 3920 (`*(c->o++) = (char)i`, crasher `717764e9`), another `$func(` header written past the same
buffer; it replays clean with this fix and fails again with only this fix reverted.
