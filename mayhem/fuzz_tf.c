// Title-formatting script compiler + evaluator (src/tf.c): compile arbitrary script text, evaluate it
// against a track with metadata, and run the legacy-format importer.
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ddb_fuzz.h"
#include "playlist.h"
#include "tf.h"

int
LLVMFuzzerInitialize (int *argc, char ***argv) {
    ddb_fuzz_init ();
    return 0;
}

int
LLVMFuzzerTestOneInput (const uint8_t *data, size_t size) {
    if (size > 4096) {
        return 0;
    }
    char *script = malloc (size + 1);
    memcpy (script, data, size);
    script[size] = 0;

    playItem_t *it = pl_item_alloc_init ("/music/Artist/Album/01 Title.flac", "stdflac");
    pl_add_meta (it, "artist", "Artist");
    pl_add_meta (it, "album", "Album");
    pl_add_meta (it, "title", "Title");
    pl_add_meta (it, "track", "1/12");
    pl_add_meta (it, "year", "2001");
    pl_add_meta (it, "genre", "Rock");

    char *code = tf_compile (script);
    if (code) {
        ddb_tf_context_t ctx;
        memset (&ctx, 0, sizeof (ctx));
        ctx._size = sizeof (ctx);
        ctx.it = (DB_playItem_t *)it;
        char out[2048];
        tf_eval (&ctx, code, out, sizeof (out));
        tf_free (code);
    }

    char legacy[2048];
    tf_import_legacy (script, legacy, sizeof (legacy));

    pl_item_unref (it);
    free (script);
    return 0;
}
