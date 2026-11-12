# fuzz_tf: triage of the Mayhem defects

The four 300s Mayhem runs report 18 (run 1), 26 (run 2), 29 (run 3) and 30 defects (run 4, `3f411bf`). They are five
distinct upstream root causes, not 30 independent bugs; none is in harness code, and the harness feeds `tf_compile` a
NUL-terminated copy of the input, as the real callers do. The 111 crashing inputs of run 4 (they include those of earlier
runs) all fail on the committed build and all replay clean once the five one-line fixes below are applied (checked on a
scratch copy, standalone and in a single libFuzzer process; upstream is untouched; the combined diff is
`mayhem/known-findings.patch`, `git apply` it on a clone to reproduce). Reverting one fix at a time makes that row's
crashers fail again (checked in earlier rounds: the compile-buffer fix brought back all the compile crashers, the `numargs`
fix 2; in this round for `tf_compile_func` 3920), so the rows are independent. This covers the crashers of run 4 only.

Run 4 by root cause: 14 `u8_nextchar` family (12 `u8_nextchar` 238, 1 `u8_toupper`, 1 `u8_tolower`), 13 `tf_compile`
buffer overflows (`tf_compile_plain` 4235 x3, `tf_compile_field` 4009 x2 and 3985 x2, `tf_compile_ifdef` 4068/4069,
`tf_compile_text_dim` 4108 and 4158, `tf_compile_func` 3902 and 3920), 1 `$pad`, 1 `$crlf`, 1 `tf_eval_int`.
The 3 stack-buffer overflows of run 4 are `u8_toupper` and `u8_tolower` (the u8 row) and `tf_import_legacy` (the
`$crlf` row).

| Root cause | Defects | Crashers | Slug |
|---|---|---|---|
| `tf_compile` output buffer `calloc (len * 3, 1)` is too small (header bytes per source byte exceed 3) | 13 in run 4 (12 in run 3, 10 in run 2, 7 in run 1): `tf_compile_text_dim` 4108 and 4158, `tf_compile_ifdef` 4068/4069, `tf_compile_field` 4009 x2 and 3985 x2, `tf_compile_func` 3902 and 3920, `tf_compile_plain` 4235 x3 | 40 | `tf-compile-text-dim-heap-overflow` |
| `u8_nextchar` trusts valid UTF-8 (`offsetsFromUTF8[6]` indexed past the end; 5+ byte lengths overflow `temp[5]` in `u8_toupper`/`u8_tolower`) | 12 array-index (`u8_nextchar` 238) + 2 stack overflows (`u8_toupper`, `u8_tolower`) in run 4 (the same in run 3 and run 2, 8 + 2 in run 1) | 61 | `tf-u8-nextchar-invalid-utf8` |
| `$pad`/`$pad_right` accept one argument but read `arglens[1]` | 1 (`tf_func_pad_impl` 1254) | 7 | `tf-pad-one-argument-oob-read` |
| `tf_import_legacy` `$crlf()` expansion writes 8 bytes after an `outsize < 7` guard | 1 (`tf_import_legacy` 4296, a stack overflow) | 1 | `tf-import-legacy-crlf-off-by-one` |
| `tf_eval_int` stores the argument count in a `char`; a count of 128 or more makes `numargs * sizeof (uint16_t)` negative | 1 (`tf_eval_int` 3040, `__asan_memcpy` negative size, CWE-131) | 2 | `tf-eval-int-signed-numargs` |

`tf_compile_text_dim` 4108 (new in run 3) is the same output-buffer cause as 4158: 4108 is the second header byte
(`*(c->o++) = 5;`) of a dim block nested in another dim block (`tf_compile_text_dim` 4146 -> `tf_compile_plain`
4230 -> `tf_compile_text_dim`), written past the same 9-byte `calloc` from `tf_compile` 4251. The `len * 16 + 64`
fix alone removes it (reverting only that fix brings back the compile crashers, including this one).
`tf_compile_func` 3920 (new in run 4, `*(c->o++) = (char)i`, crasher `717764e9`) is the same output-buffer cause as 3902: both
write a `$func(` header into the same undersized `calloc`. It replays clean with the `len * 16 + 64` fix alone, and
reverting only that fix makes it fail again at tf.c:3920.
`tf_eval_int` 3040 is not a consequence of the compile overflow: its
bytecode buffer is intact (the report points inside the 309-byte `tf_compile` allocation) and the 2 crashers still
fail with the compile fix applied and only the `numargs` fix reverted.
