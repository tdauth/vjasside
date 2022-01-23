TEMPLATE = subdirs

SUBDIRS = \
          testparser \
          testscanner \
          testcodeelementholder \
          testmainwindow

# where to find the sub projects - give the folders
testparser.subdir = testparser
testscanner.subdir = testscanner # relative paths
testcodeelementholder.subdir = testcodeelementholder
testmainwindow.subdir = testmainwindow
