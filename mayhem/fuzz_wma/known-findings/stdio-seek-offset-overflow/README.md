`stdio_seek` (src/vfs_stdio.c:173) turns a `SEEK_CUR` into an absolute seek with `offs + offset`, with no overflow
check. `asf_parse_header` skips an object with `SKIP_BYTES(fd, size - ...)`, and the object size is an
attacker-controlled 64-bit field, so the sum overflows `int64_t` (UBSan: `signed integer overflow: ... + ...
cannot be represented in type 'int64_t'`).
The first huge seek has to succeed for `offs` to be large. `lseek64` accepts an offset near 2^63 on tmpfs but fails
with EINVAL on a container's overlayfs `/tmp` (what the local replay used), so the crash reproduces only where the scratch directory is on tmpfs, as it is in
Mayhem. The 5 saved inputs (`03ff8154`, `34866a55`, `4ee4beda`, `7550d7d2`, `7cb76e07`) replay clean on a plain
overlay `/tmp`; this is the one `stdio_seek` defect. The harness does not depend on state: only the filesystem does.
Impact: undefined behavior on a crafted file.
Upstream fix: `if (__builtin_add_overflow (offs, offset, &offset)) { return -1; }`.
Replay: `docker run --tmpfs /tmp ... /mayhem/fuzz_wma-standalone input` (or any host with `/tmp` on tmpfs).
