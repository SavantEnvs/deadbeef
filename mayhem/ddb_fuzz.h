#ifndef DDB_FUZZ_H
#define DDB_FUZZ_H

#include <stddef.h>
#include <stdint.h>

// One-time core bring-up, mirroring Tests/gtest-runner.cpp (conf, logger, tf, playlist, compiled-in plugins).
// All directories point into a private scratch dir; nothing is read from or written to $HOME.
void ddb_fuzz_init (void);

// Overwrite the scratch file `name` with the input and return its full path.
const char *ddb_fuzz_write (const char *name, const uint8_t *data, size_t size);

#endif
