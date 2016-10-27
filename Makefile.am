SUBDIRS = src

dist_doc_DATA = README

man_MANS = doc/calcelestial.1

if DEBUG
    AM_CFLAGS = -g -Wall -Werror -Wno-uninitialized -O0 -fno-omit-frame-pointer
else
    AM_CFLAGS = -O3
endif

TEST_OPTS = -p sun -m rise -t 1990-03-20 -q Baden,Switzerland

test: src/calcelestial
	[ "$$(src/calcelestial ${TEST_OPTS} -l -f %H:%M:%S)" = "06:30:53" ]
	[ "$$(src/calcelestial ${TEST_OPTS} -l -f §A:§O)"    = "47.473:8.306" ]
	[ "$$(src/calcelestial ${TEST_OPTS} -l -f %J)"       = "2447970.730" ]
	[ "$$(src/calcelestial ${TEST_OPTS} -l -f %Z)"       = "CET" ]
	[ "$$(src/calcelestial ${TEST_OPTS} -z America/New_York -f %H:%M:%S)" == "00:30:53" ]