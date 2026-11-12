`tf_eval_int` (src/tf.c:3040) reads a function call's argument count as `char numargs = *code;`. The compiler stores
the count as one byte, so a call with 128 to 255 arguments (the saved input is `$upper(` followed by a run of commas, 130 arguments) gives a negative
`char`, and `memcpy (arglens, code+1, numargs * sizeof (uint16_t))` gets a negative size (ASan:
`negative-size-param (size=-252)` in `__asan_memcpy`, from `tf_eval` 313; Mayhem classes it as CWE-131 "Incorrect
Calculation of Buffer Size"). The `alloca (numargs * sizeof (uint16_t))` above it is already sized from the same
negative value. The bytecode itself is intact: this is not caused by the `tf_compile` overflow.
Impact: crash (and a huge `alloca`) evaluating a user-controlled script with many arguments.
Upstream fix (one line): `unsigned char numargs = *code;`.
Both saved crashers of this class (`285cd4f8`, `a2ad35f2`) replay clean with it, and fail again with only this fix
reverted while the compile-buffer fix stays applied.
Replay: `/mayhem/fuzz_tf-standalone input`.
