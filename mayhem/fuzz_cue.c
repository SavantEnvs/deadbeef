// Cuesheet parser (src/cueutil.c) in embedded-cuesheet mode, as used for CUESHEET tags inside audio
// files. The FILE entries are never resolved against the host file system in this mode.
#include <dirent.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ddb_fuzz.h"
#include "playlist.h"
#include "cueutil.h"

int
LLVMFuzzerInitialize (int *argc, char ***argv) {
    ddb_fuzz_init ();
    return 0;
}

int
LLVMFuzzerTestOneInput (const uint8_t *data, size_t size) {
    if (size > 64 * 1024) {
        return 0;
    }
    uint8_t *buf = malloc (size + 1);
    memcpy (buf, data, size);
    buf[size] = 0;

    playlist_t *plt = plt_alloc ("fuzz");
    playItem_t *origin = pl_item_alloc_init ("/music/album.flac", "stdflac");
    plt_load_cuesheet_from_buffer (plt, NULL, "/music/album.flac", origin, (int64_t)44100 * 3600, 44100, buf, (int)size, NULL, NULL, 0);
    pl_item_unref (origin);
    plt_unref (plt);
    free (buf);
    return 0;
}
