TEMPLATE = subdirs

SUBDIRS = \
          app \
          commandline \
          tests

# where to find the sub projects - give the folders
app.subdir = src/app
commandline.subdir = src/commandline
tests.subdir = src/tests # relative paths

# what subproject depends on others
commandline.depends = app
tests.depends = app

DISTFILES += \
    README.md \
    wc3reforged/common.j \
    wc3reforged/common.ai \
    wc3reforged/Blizzard.j \
    pjass/pjass.exe
