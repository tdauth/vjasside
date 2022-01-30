TEMPLATE = subdirs

SUBDIRS = \
          testpjass \
          testparser \
          testscanner \
          testhighlightinfo \
          testmainwindow

# where to find the sub projects - give the folders
testpjass.subdir = testpjass
testparser.subdir = testparser
testscanner.subdir = testscanner # relative paths
testhighlightinfo.subdir = testhighlightinfo
testmainwindow.subdir = testmainwindow
