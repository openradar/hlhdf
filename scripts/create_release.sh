#!/bin/sh
###########################################################################
# Copyright (C) 2009 Swedish Meteorological and Hydrological Institute, SMHI,
#
# This file is part of HLHDF.
#
# HLHDF is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# HLHDF is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with HLHDF.  If not, see <http://www.gnu.org/licenses/>.
###########################################################################
#
# Script for generating a release of HLHDF. It will perform some basic
# tests to verify that the release at least builds, tests ok before tagging
# and generating a distribution tar-ball.
#
# The configuration part is a bit rudimentary at the time and requires that
# a file called .release_conf exists in the HLHDF source root directory
# with a line containing the ./configure directive except the --prefix part
# that is automatically generated by this script
#
# @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
# @date 2009-06-25
###########################################################################
ask_yes_no_question() {
  echo -n "$1"
  CONTVALUE=$2
  DEFAULTVALUE=$3
  read YESNO
  if [ $? -ne 0 ]; then
    exit 255;
  fi
  
  if [ "x$YESNO" = "x" ]; then
    YESNO=$DEFAULTVALUE
  fi
  if [ "x$YESNO" != "x$CONTVALUE" ]; then
    exit 255
  fi
}

create_new_version() {
  BVERSION=`echo $1 | sed -e"s/\(v\)\([0-9]*\).\([0-9]*\).\([0-9]*\)\([^\$]*\)/\2.\3.\4/g"`
  VER_MAJOR=`echo $BVERSION | cut -d "." -f1`
  VER_MINOR=`echo $BVERSION | cut -d "." -f2`
  VER_UPDATE=`echo $BVERSION | cut -d "." -f3`

  NEW_UPDATE_VER=`expr $VER_UPDATE + 1`

  echo "v$VER_MAJOR.$VER_MINOR.$NEW_UPDATE_VER"
}

CURRENT_DIR=`pwd`
#SCRIPTPATH=`dirname "$(readlink -f $0)"`

SCRFILE=`python -c "import os;print os.path.abspath(\"$0\")"`
SCRIPTPATH=`dirname "$SCRFILE"`

cd "$SCRIPTPATH/.."

git status

echo ""
echo "Ensure that no pending changes exists in the git status, if there are"
echo "abort this script and commit the pending changes"
ask_yes_no_question "Continue? [yes]: " yes yes

CURRENT_VERSION=`git describe`
if [ $? -ne 0 ]; then
  echo "No 'git tag -a <version> -m <message>' has been run"
  exit 255
fi

TMP_VERSION=`create_new_version "$CURRENT_VERSION"`

echo -n "Current version is $CURRENT_VERSION, will create [$TMP_VERSION]: "
read NEW_VERSION
if [ $? -ne 0 ]; then
  exit 255
fi
if [ "x$NEW_VERSION" = "x" ]; then
  NEW_VERSION=$TMP_VERSION
fi

echo -n "Enter tag message for version $NEW_VERSION: "
read TAG_MESSAGE
if [ $? -ne 0 ]; then
  exit 255
fi
if [ "x$TAG_MESSAGE" = "x" ]; then
  echo "You must specify a reason for tagging this release"
  exit 255
fi

CONFIGURECMD=./configure
if [ -f .release_conf ]; then
  CONFIGURECMD=`cat .release_conf`
else
  echo "No configuration parameters setup, will only use ./configure"
  echo "If you want some other ./configure parameters, edit the file .release_conf in the root directory of HLHDF"
  echo "with only one line containing the full ./configure command."
  echo "NOTE! Do not specify --prefix since that is automatically generated by this script"
fi

echo "Generating tar-ball"
\rm -fr "/tmp/hlhdf-$NEW_VERSION/"
\rm -f "/tmp/hlhdf-$NEW_VERSION.tgz"
git archive --format=tar --prefix="hlhdf-$NEW_VERSION/" HEAD | (cd /tmp/ && tar xf -)
if [ $? -ne 0 ]; then
  echo "Failed to copy source files"
  exit 255
fi
cat << EOF > "/tmp/hlhdf-$NEW_VERSION/.version"
$NEW_VERSION
EOF

if [ $? -ne 0 ]; then
  echo "Failed to create version file"
  exit 255
fi

#
# CREATE A RELEASE SUMMARY TO KEEP TRACK ON CHANGES
#
if [ -f /tmp/hlhdf-$NEW_VERSION/RELEASE ]; then
  echo "RELEASE has been added to the source tree, this file should automatically be generated from the revision history"
  exit 255
fi

NLINES=`git tag | wc -l | awk '{print $1}'`
LASTLINE=`expr $NLINES - 1`
LASTTAG=`git tag | sed "1,$LASTLINE d"`
COMMITLOG=`git log $LASTTAG.. --pretty=format:"%s"`
RHISTORY=`git tag -n10`

cat << EOF > "/tmp/hlhdf-$NEW_VERSION/RELEASE"
News for $NEW_VERSION: $TAG_MESSAGE

$COMMITLOG

Previous releases:
$RHISTORY
EOF

if [ $? -ne 0 ]; then
  echo "Failed to create RELEASE file"
  exit 255
fi

cd /tmp/
tar -cvzf "/tmp/hlhdf-$NEW_VERSION.tgz" "hlhdf-$NEW_VERSION/"
if [ $? -ne 0 ]; then
  echo "Failed to create tar-ball"
  exit 255
fi

echo "Running build and installation tests...."
$SCRIPTPATH/test_build_and_install.sh "/tmp/hlhdf-$NEW_VERSION.tgz" "$NEW_VERSION"
if [ $? -ne 0 ]; then
  echo "Build and installation tests failed"
  exit 255
fi

echo "Build and installation tests successful."

echo "Copying deliverable (hlhdf-$NEW_VERSION.tgz) to $CURRENT_DIR"
cp "/tmp/hlhdf-$NEW_VERSION.tgz" "$CURRENT_DIR"

echo "RELEASE: $NEW_VERSION"
echo "MESSAGE: $TAG_MESSAGE"
echo -n "Do you want to create a tag in the git branch? [yes]: "
YESNO=yes
read YESNO
if [ "x$YESNO" == "xyes" ]; then
  git tag -a "$NEW_VERSION" -m "$TAG_MESSAGE"
fi

echo "Release created successfully"
