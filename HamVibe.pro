TEMPLATE = subdirs
SUBDIRS += app tests

app.file = HamVibe_app.pro
tests.file = tests/tests.pro
tests.depends = app
