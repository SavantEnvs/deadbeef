// Tag readers in src/junklib.c: ID3v1, ID3v2 (2.2/2.3/2.4, full + convert), APEv2, plus charset
// detection/recoding of the raw bytes.
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <deadbeef/deadbeef.h>
#include "ddb_fuzz.h"
#include "junklib.h"
#include "playlist.h"
#include "vfs.h"

extern DB_functions_t *deadbeef;

int
LLVMFuzzerInitialize (int *argc, char ***argv) {
    ddb_fuzz_init ();
    return 0;
}

static void
read_tags (const char *path) {
    playItem_t *it = pl_item_alloc_init (path, "stdmpg");

    DB_FILE *fp = deadbeef->fopen (path);
    if (fp) {
        junk_id3v2_read (it, fp);
        deadbeef->fclose (fp);
    }

    fp = deadbeef->fopen (path);
    if (fp) {
        DB_id3v2_tag_t tag24;
        memset (&tag24, 0, sizeof (tag24));
        junk_id3v2_read_full (it, &tag24, fp);
        junk_id3v2_free (&tag24);
        deadbeef->fclose (fp);
    }

    fp = deadbeef->fopen (path);
    if (fp) {
        junk_id3v1_read (it, fp);
        deadbeef->fclose (fp);
    }

    fp = deadbeef->fopen (path);
    if (fp) {
        junk_apev2_read (it, fp);
        deadbeef->fclose (fp);
    }

    fp = deadbeef->fopen (path);
    if (fp) {
        DB_apev2_tag_t ape;
        memset (&ape, 0, sizeof (ape));
        junk_apev2_read_full (it, &ape, fp);
        junk_apev2_free (&ape);
        deadbeef->fclose (fp);
    }

    pl_item_unref (it);
}

int
LLVMFuzzerTestOneInput (const uint8_t *data, size_t size) {
    if (size > 256 * 1024) {
        return 0;
    }
    read_tags (ddb_fuzz_write ("tags.mp3", data, size));

    if (size <= 4096) {
        char *s = malloc (size + 1);
        memcpy (s, data, size);
        s[size] = 0;
        const char *cs = junk_detect_charset (s);
        char out[8192];
        junk_recode (s, (int)size, out, sizeof (out), cs ? cs : "iso8859-1");
        junk_recode (s, (int)size, out, sizeof (out), "cp1251");
        junk_recode (s, (int)size, out, sizeof (out), "shift-jis");
        free (s);
    }
    return 0;
}
