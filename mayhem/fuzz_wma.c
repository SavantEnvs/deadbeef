// WMA/ASF decoder plugin (plugins/wma: ASF header parser + fixed-point WMA decoder). The plugin is not
// in src/moduleconf.h, so its decoder is driven directly through the loaded API.
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <deadbeef/deadbeef.h>
#include "ddb_fuzz.h"
#include "playlist.h"
#include "plugins.h"

DB_plugin_t *wma_load (DB_functions_t *api);

static DB_decoder_t *dec;

int
LLVMFuzzerInitialize (int *argc, char ***argv) {
    ddb_fuzz_init ();
    dec = (DB_decoder_t *)wma_load (plug_get_api ());
    return 0;
}

int
LLVMFuzzerTestOneInput (const uint8_t *data, size_t size) {
    if (size > 512 * 1024) {
        return 0;
    }
    const char *path = ddb_fuzz_write ("input.wma", data, size);

    playlist_t *plt = plt_alloc ("fuzz");
    DB_playItem_t *it = dec->insert ((ddb_playlist_t *)plt, NULL, path);
    if (it) {
        DB_fileinfo_t *fi = dec->open (0);
        if (fi) {
            if (dec->init (fi, it) == 0) {
                static char buf[16384];
                for (int i = 0; i < 64; i++) {
                    if (dec->read (fi, buf, sizeof (buf)) <= 0) {
                        break;
                    }
                }
                dec->seek_sample (fi, 4410);
                dec->read (fi, buf, sizeof (buf));
            }
            dec->free (fi);
        }
        dec->read_metadata (it);
    }
    plt_unref (plt);
    return 0;
}
