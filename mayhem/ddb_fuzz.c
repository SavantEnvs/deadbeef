#include <limits.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ddb_fuzz.h"
#include "conf.h"
#include "logger.h"
#include "metacache.h"
#include "playlist.h"
#include "playmodes.h"
#include "plugins.h"
#include "tf.h"

extern char dbplugindir[PATH_MAX];
extern char dbconfdir[PATH_MAX];
extern char confdir[PATH_MAX];
extern char dbinstalldir[PATH_MAX];
extern char dbdocdir[PATH_MAX];
extern char dbpixmapdir[PATH_MAX];
extern char dbcachedir[PATH_MAX];
extern char dbresourcedir[PATH_MAX];
extern char dbstatedir[PATH_MAX];
extern char sys_install_path[PATH_MAX];

static char scratch[PATH_MAX];
static char file_path[PATH_MAX];
static int initialized;

static void
cleanup (void) {
    if (file_path[0]) {
        unlink (file_path);
    }
    rmdir (scratch);
}

static void
set_dir (char *dst, const char *sub) {
    snprintf (dst, PATH_MAX, "%s/%s", scratch, sub);
    mkdir (dst, 0700);
}

void
ddb_fuzz_init (void) {
    if (initialized) {
        return;
    }
    initialized = 1;

    const char *tmp = getenv ("TMPDIR");
    snprintf (scratch, sizeof (scratch), "%s/ddbfuzz.XXXXXX", tmp && *tmp ? tmp : "/tmp");
    if (!mkdtemp (scratch)) {
        perror ("mkdtemp");
        exit (1);
    }
    atexit (cleanup);

    setenv ("HOME", scratch, 1);
    setenv ("XDG_LOCAL_HOME", scratch, 1);
    set_dir (dbplugindir, "plugins");
    set_dir (dbconfdir, "conf");
    set_dir (confdir, "confroot");
    set_dir (dbinstalldir, "install");
    set_dir (dbdocdir, "doc");
    set_dir (dbpixmapdir, "pixmaps");
    set_dir (dbcachedir, "cache");
    set_dir (dbresourcedir, "res");
    set_dir (dbstatedir, "state");
    set_dir (sys_install_path, "sys");

    metacache_init ();
    ddb_logger_init ();
    conf_init ();
    conf_enable_saving (0);
    tf_init ();
    streamer_playmodes_init ();
    pl_init ();
    if (plug_load_all ()) {
        fprintf (stderr, "plug_load_all failed\n");
        exit (1);
    }
}

const char *
ddb_fuzz_write (const char *name, const uint8_t *data, size_t size) {
    if (!file_path[0]) {
        snprintf (file_path, sizeof (file_path), "%s/%s", scratch, name);
    }
    FILE *fp = fopen (file_path, "wb");
    if (!fp) {
        perror ("fopen scratch");
        exit (1);
    }
    if (size) {
        fwrite (data, 1, size, fp);
    }
    fclose (fp);
    return file_path;
}
