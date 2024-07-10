TEMPLATE = subdirs

SUBDIRS += src      \
           tests

src.file = src/studio.pro
tests.depends = src
