`wmaplug_insert` (plugins/wma/wma_plugin.c:428) divides by `wfx.packet_size` and, on the next line, by
`wfx.numpackets`, both read from the File Properties object of the header, with no check for zero (UBSan: `division
by zero`). Two saved inputs (`3bbc3f02`, `d4605cb2`) hit it; this is the one `Divide By Zero` defect.
Impact: crash on a malformed file; a playlist insert of such a file aborts the player.
Upstream fix: `if (wfx.packet_size == 0 || wfx.numpackets == 0) { deadbeef->pl_item_unref (it); return NULL; }`
before the division.
Replay: `/mayhem/fuzz_wma-standalone input`.
