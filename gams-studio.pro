TEMPLATE = subdirs

SUBDIRS += src      \
#           watchdog \
#           crash    \
           tests

src.file = src/studio.pro
tests.depends = src
crash.file = watchdog/crash/crash.pro
