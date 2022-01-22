TEMPLATE = subdirs

SUBDIRS = \
          app \
          tests

# where to find the sub projects - give the folders
app.subdir = src/app
tests.subdir = src/tests # relative paths

# what subproject depends on others
tests.depends = app

DISTFILES += \
    README.md
