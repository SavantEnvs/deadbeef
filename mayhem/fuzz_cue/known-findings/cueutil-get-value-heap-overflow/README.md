`pl_get_value_from_cue` (src/cueutil.c:198) reads one byte past the end of the cuesheet buffer
(`READ of size 1`, reached from `pl_cue_get_field_value` / `plt_load_cuesheet_from_buffer`).
ASan: heap-buffer-overflow. Replay: `/mayhem/fuzz_cue-standalone input`.
