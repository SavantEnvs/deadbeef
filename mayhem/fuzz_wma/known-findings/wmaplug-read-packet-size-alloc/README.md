`wmaplug_read` (plugins/wma/wma_plugin.c:215) does `calloc (1, info->wfx.packet_size)` for every packet.
`packet_size` is a 32-bit field from the header's File Properties object and is never bounded, so the header alone
chooses an allocation of up to 4 GB. libFuzzer reports it as `out-of-memory (malloc(4278191104))`, which Mayhem files
as `Memory Allocation with Excessive Size Value`; 6 saved inputs (`77283be1`, `a5beae5b`, `b14530e1`, `c2294620`,
`c38232b8`, `e2831e54`) hit it, all one defect.
This is an attacker-controlled size, not a leak flood: each input allocates one huge block once and frees it.
LeakSanitizer is off (`mayhem/lsan_off.cc`), so leaks are not what is being reported. On a real host the call
either returns NULL (the code handles that) or commits gigabytes lazily.
Impact: memory-exhaustion denial of service from a 1.7 KB file.
Upstream fix: reject an implausible packet size when the header is parsed, `if (wfx->packet_size > 16 * 1024 * 1024)
return ASF_ERROR_INVALID_OBJECT;` after `read_uint32le (fd, &wfx->packet_size)`.
Replay: `/mayhem/fuzz_wma input` (the libFuzzer binary; the `-standalone` build has no allocation limit).
