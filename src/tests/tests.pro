TEMPLATE = subdirs

SUBDIRS = \
          testparser \
          testscanner \
          testhighlightinfo \
          testmainwindow

# where to find the sub projects - give the folders
testparser.subdir = testparser
testscanner.subdir = testscanner # relative paths
testhighlightinfo.subdir = testhighlightinfo
testmainwindow.subdir = testmainwindow
