#!/usr/bin/env bash
#
# mayhem/build.sh — builds, inside the commit image (as `mayhem`, in /mayhem):
#   1. DeaDBeeF's core + mp3/m3u/wma plugins sanitized ($SANITIZER_FLAGS, DWARF 3, fuzzer-no-link), linked into
#      one libFuzzer binary per harness and a `-standalone` reproducer per harness;
#   2. upstream's googletest suite (Tests/*Tests.cpp, as Tests.mk builds it) with the project's normal flags,
#      for mayhem/test.sh.
# Upstream builds with autotools/Xcode; this reproduces its Tests.mk source list directly because the player's
# GUI/plugin stack (gtk, ffmpeg, ...) is not needed to reach the parsers. Air-gapped: every dependency is an
# apt -dev package or lives under /opt/toolchains/deps (see mayhem/Dockerfile).
set -euo pipefail

[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
: "${SRC:=/mayhem}"
: "${STANDALONE_FUZZ_MAIN:=/opt/mayhem/StandaloneFuzzTargetMain.c}"
# The plugin API casts between struct types (ddb_playItem_t / playItem_t, DB_* / ddb_*) at every call, so
# -fsanitize=function aborts every input on the first API call; every other UBSan check stays on and halting.
: "${UBSAN_RELAX=-fno-sanitize=function}"
export SANITIZER_FLAGS DEBUG_FLAGS CC CXX LIB_FUZZING_ENGINE MAYHEM_JOBS COVERAGE_FLAGS UBSAN_RELAX

cd "$SRC"

DEPS=/opt/toolchains/deps
LD="$DEPS/libdispatch-6.0.3"
MP4P="$(echo "$DEPS"/mp4p-*/src)"
GT=/usr/src/googletest/googletest
OUT="$SRC/mayhem/build"
rm -rf "$OUT"
mkdir -p "$OUT/fuzz" "$OUT/test" "$OUT/harness"

INC="-I$MP4P/include -I$LD/include -Ishared -I. -Isrc -Iinclude -Isrc/ConvertUTF -Isrc/scriptable -Iplugins/coreaudio -Iplugins/wma -Imayhem"
DEFS='-fblocks -fcommon -D_GNU_SOURCE -DHAVE_LOG2=1 -DDOCDIR="" -DPREFIX="" -DLIBDIR="" -DVERSION="" -DUSE_LIBMAD -DUSE_LIBMPG123 -DXCTEST -DGOOGLETEST_STATIC'
export FUZZ_C="-w -std=gnu11 -O1 $SANITIZER_FLAGS $DEBUG_FLAGS -fsanitize=fuzzer-no-link $UBSAN_RELAX $DEFS $INC"
export TEST_C="-w -std=c99 -O2 $COVERAGE_FLAGS $DEFS $INC"
export TEST_CXX="-w -std=c++17 -O2 $COVERAGE_FLAGS $DEFS $INC -I$GT -I$GT/include -I$GT/../googlemock/include"

compile_one() { # <kind: fuzz|test> <source> <object>
  case "$1:$2" in
    # src/tf.c stores uint16_t arg lengths into an unaligned byte stream (FIXME upstream); benign on x86, floods otherwise
    fuzz:src/tf.c) $CC $FUZZ_C -fno-sanitize=alignment -c "$2" -o "$3" ;;
    fuzz:*.c) $CC $FUZZ_C -c "$2" -o "$3" ;;
    test:*.c) $CC $TEST_C -c "$2" -o "$3" ;;
    test:*.cc|test:*.cpp) $CXX $TEST_CXX -c "$2" -o "$3" ;;
  esac
}
export -f compile_one

compile_list() { # <kind> ; sources on stdin -> $OUT/<kind>/<flattened path>.o
  local kind="$1" f
  while read -r f; do
    [ -n "$f" ] || continue
    echo "$f $OUT/$kind/$(echo "$f" | tr / _).o"
  done | xargs -P "$MAYHEM_JOBS" -L1 bash -c 'compile_one '"$kind"' "$1" "$2"' _
}

core_sources() {
  ls "$MP4P"/src/*.c external/wcwidth/*.c \
     shared/*.c shared/scriptable/*.c shared/undo/*.c shared/filereader/*.c \
     src/*.c src/ConvertUTF/*.c src/md5/*.c src/scriptable/*.c src/undo/*.c \
     plugins/m3u/*.c plugins/mp3/*.c | grep -v '^src/main\.c$'
}

# ---- 1. sanitized fuzz build -------------------------------------------------------------------
{ core_sources
  echo Tests/testbootstrap.c
  echo plugins/wma/wma_plugin.c; echo plugins/wma/asfheader.c; echo plugins/wma/libasf/asf.c
  for f in wmafixed wmadeci fft-ffmpeg mdct_lookup mdct ffmpeg_bitstream; do echo "plugins/wma/libwma/$f.c"; done
  echo mayhem/ddb_fuzz.c
} | compile_list fuzz

LIBS="-Wl,--no-warnings -L$LD/lib -lmad -lmpg123 -ljansson -ldispatch -lBlocksRuntime -lpthread -lm -ldl"
HARNESS_FLAGS="$SANITIZER_FLAGS $DEBUG_FLAGS -O1 $UBSAN_RELAX $DEFS $INC -std=gnu11"

$CXX $SANITIZER_FLAGS $DEBUG_FLAGS -c mayhem/lsan_off.cc -o "$OUT/harness/lsan_off.o"
$CC $SANITIZER_FLAGS $DEBUG_FLAGS -c "$STANDALONE_FUZZ_MAIN" -o "$OUT/harness/standalone_main.o"

for h in fuzz_tf fuzz_tags fuzz_mp3 fuzz_wma fuzz_cue; do
  $CC $HARNESS_FLAGS -fsanitize=fuzzer-no-link -c "mayhem/$h.c" -o "$OUT/harness/$h.o"
  $CXX $SANITIZER_FLAGS $DEBUG_FLAGS $LIB_FUZZING_ENGINE "$OUT/harness/$h.o" "$OUT"/fuzz/*.o \
       "$OUT/harness/lsan_off.o" $LIBS -o "$SRC/$h"
  $CXX $SANITIZER_FLAGS $DEBUG_FLAGS -fsanitize=fuzzer-no-link "$OUT/harness/$h.o" "$OUT/harness/standalone_main.o" \
       "$OUT"/fuzz/*.o "$OUT/harness/lsan_off.o" $LIBS -o "$SRC/$h-standalone"
done

# ---- 2. upstream test suite, normal flags ----------------------------------------------------
{ ls "$MP4P"/src/*.c external/wcwidth/*.c plugins/gtkui/layoutmanager.c \
     plugins/m3u/*.c plugins/mp3/*.c plugins/nullout/*.c plugins/shellexec/*.c plugins/vfs_curl/*.c \
     shared/*.c shared/analyzer/*.c shared/scriptable/*.c shared/undo/*.c shared/filereader/*.c \
     src/*.c src/ConvertUTF/*.c src/md5/*.c src/scriptable/*.c src/undo/*.c Tests/*.c | grep -v '^src/main\.c$'
  ls Tests/gtest-runner.cpp Tests/*Tests.cpp | sort -u
  echo "$GT/src/gtest-all.cc"
} | compile_list test

$CXX "$OUT"/test/*.o -L"$LD/lib" -lmad -lmpg123 -lcurl -ldispatch -lpthread -lBlocksRuntime -lm -ljansson -ldl \
     $COVERAGE_FLAGS -o "$OUT/test/runtests"
mkdir -p "$OUT/test/Tests"
cp -r Tests/TestData Tests/PresetManagerData "$OUT/test/Tests/"
