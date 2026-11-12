// MP3 decoder plugin (plugins/mp3: stream parser, libmad and libmpg123 backends) driven through the
// playlist insert path, then decode + seek.
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <deadbeef/deadbeef.h>
#include "ddb_fuzz.h"
#include "playlist.h"
#include "plugins.h"

extern DB_functions_t *deadbeef;

int
LLVMFuzzerInitialize (int *argc, char ***argv) {
    ddb_fuzz_init ();
    return 0;
}

static void
run (const char *path, int backend) {
    deadbeef->conf_set_int ("mp3.backend", backend);

    playlist_t *plt = plt_alloc ("fuzz");
    DB_playItem_t *it = deadbeef->plt_insert_file2 (0, (ddb_playlist_t *)plt, NULL, path, NULL, NULL, NULL);
    DB_decoder_t *dec = (DB_decoder_t *)deadbeef->plug_get_for_id ("stdmpg");

    if (it && dec) {
        DB_fileinfo_t *fi = dec->open (0);
        if (fi) {
            if (dec->init (fi, it) == 0) {
                static char buf[16384];
                for (int i = 0; i < 64; i++) {
                    if (dec->read (fi, buf, sizeof (buf)) <= 0) {
                        break;
                    }
                }
                dec->seek_sample (fi, 44100 / 2);
                for (int i = 0; i < 8; i++) {
                    if (dec->read (fi, buf, sizeof (buf)) <= 0) {
                        break;
                    }
                }
                dec->seek (fi, 0.1f);
                dec->read (fi, buf, sizeof (buf));
            }
            dec->free (fi);
        }
    }
    plt_unref (plt);
}

int
LLVMFuzzerTestOneInput (const uint8_t *data, size_t size) {
    if (size > 512 * 1024) {
        return 0;
    }
    const char *path = ddb_fuzz_write ("input.mp3", data, size);
    run (path, 0);
    run (path, 1);
    return 0;
}
