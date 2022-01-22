TEMPLATE = subdirs

SUBDIRS = \
          testparser \
          testscanner

# where to find the sub projects - give the folders
testparser.subdir = testparser
testscanner.subdir = testscanner # relative paths
