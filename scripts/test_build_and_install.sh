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
# Script performing build and installation tests.
#
# The configuration part is a bit rudimentary at the time and requires that
# a file called .test_conf exists in the HLHDF source root directory
# with a line containing the ./configure directive except the --prefix part
# that is automatically generated by this script
#
# @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
# @date 2009-08-19
###########################################################################

###
# Verifies that a directory exists
#
# Arguments:
#  1 - Comment, usually what test it is
#  2 - Dirname, the directory to check
#
# Returns: 0 on success, otherwise 255
verify_directory() {
  COMMENT=$1
  DIRNAME=$2
  if [ ! -d "$2" ]; then
    echo "Test [$1] failed: No directory named '$2'"
    return 255
  fi
  return 0
}

###
# Verifies that a file exists
#
# Arguments:
#  1 - Comment, usually what test it is
#  2 - Filename, the file to check
#
# Returns: 0 on success, otherwise 255
verify_file() {
  if [ ! -f "$2" ]; then
    echo "Test [$1] failed: No file named '$2'"
    return 255
  fi
  return 0
}

###
# Verifies that a symbolic link exists and
# what directory it points at.
#
# Arguments:
#  1 - Comment, usually what test it is
#  2 - Linkname, the link to check
#  3 - The targets name
#
# Returns: 0 on success, otherwise 255
verify_symbolic_link() {
  if [ ! -h "$2" ]; then
    echo "Test [$1] failed: $2 is not a symbolic link"
    return 255
  fi
  STR=`readlink "$2" | grep -e "$3\$"`
  if [ "x$STR" = "x" ]; then
    echo "Test [$1] failed: $2 does not point at $3"
    return 255
  fi
  return 0
}

###
# Verifies that the source structure is according to
# some set rules. It will check for hlhdf-<version>
# and some underlying files. This means that
# directory should be changed to the source catalogue
# before executing this function
#
# Arguments:
#  1 - Comment, usually what test it is
#  2 - Version, the source version
#
# Returns: 0 on success, otherwise 255
verify_source_structure() {
  COMMENT=$1
  VERSION=$2
  verify_directory "$1" "hlhdf-$2" || return 255
  verify_file "$1" "hlhdf-$2/INSTALL" || return 255
  verify_file "$1" "hlhdf-$2/LICENSE" || return 255
  verify_file "$1" "hlhdf-$2/README" || return 255
  verify_file "$1" "hlhdf-$2/.version" || return 255
  VERNBR=`cat hlhdf-$2/.version`
  if [ "x$2" != "x$VERNBR" ]; then
    echo "Test [$1] failed: Version numbers does not match with hlhdf-$2/.version"
    return 255
  fi
  return 0
}

###
# Removes the specified file
#
# Arguments:
#  1 - Filename, the file to remove
#
# Returns: Always returns 0
remove_file() {
  \rm -f "$1"
}

###
# Removes the specified directory
#
# Arguments:
#  1 - Dirname, the directory to remove
#
# Returns: Always returns 0
remove_directory() {
  \rm -fr "$1"
}

###
# Changes to the specified directory
#
# Arguments:
#  1 - Dirname, the directory to change to
#
# Returns: Returns 0 on success, otherwise 255
change_directory() {
  cd "$1"
  if [ $? -ne 0 ]; then
    echo "Failed to change directory to $1"
    return 255
  fi
  return 0
}

###
# Removes the specified directory and creates it again
#
# Arguments:
#  1 - Dirname, the directory to remove and create
#
# Returns: Returns 0 on success, otherwise 255
remove_and_create_directory() {
  remove_directory "$1"
  mkdir "$1"
  if [ $? -ne 0 ]; then
    echo "Failed to create directory $1"
    return 255
  fi
  return 0
}

###
# Removes the specified file and creates it again
#
# Arguments:
#  1 - Filename, the file to remove and create
#
# Returns: Returns 0 on success, otherwise 255
remove_and_create_file() {
  remove_file "$1"
  touch "$1"
  if [ $? -ne 0 ]; then
    echo "Failed to create file $1"
    return 255
  fi
  return 0
}

###
# Creates a fake installation directory structure
#
# Arguments:
#  1 - Comment, typically the test case name
#  2 - The installation directory that should be faked
#  3 - The version to create
#
# Returns: Returns 0 on success, otherwise 255
create_fake_directory_structure() {
  remove_and_create_directory "$2" || return 255
  remove_and_create_directory "$2/$3" || return 255    
  remove_and_create_directory "$2/$3/bin" || return 255
  remove_and_create_directory "$2/$3/include" || return 255
  remove_and_create_directory "$2/$3/lib" || return 255
  remove_and_create_directory "$2/$3/mkf" || return 255
  remove_and_create_file "$2/hlhdf.pth" || return 255
  cat << EOF > "$2/.version"
$3
EOF
  if [ $? -ne 0 ]; then
    echo "Test [$1]: Could not create faked .version file"
    return 255
  fi
  return 0
}

###
# Creates a fake directory structure for a HLHDF installation
# prior to 0.7.5.
#
# Arguments:
#  1 - Installdir, the directory that should be faked
#
# Returns: Returns 0 on success, otherwise 255
create_fake_old_directory_structure() {
  remove_and_create_directory "$1" || return 255
  remove_and_create_directory "$1/bin" || return 255
  remove_and_create_directory "$1/include" || return 255
  remove_and_create_directory "$1/lib" || return 255
  remove_and_create_directory "$1/mkf" || return 255
  remove_and_create_file "$1/hlhdf.pth" || return 255
  
  return 0  
}

###
# Executes the provided command
#
# Arguments:
#  1 - Comment, typically the test case name
#  2 - Command, The command that should be executed
#
# Returns: Returns 0 on success, otherwise 255
execute_command() {
  $2
  if [ $? -ne 0 ]; then
    echo "Test [$1] failed: Could not execute command '$2'"
    return 255
  fi
  return 0
}

###
# Verifies the HLHDF installation directory structure.
# Before calling this function, the user has to change
# the directory to the base installation directory
# since it will check for <version> and underlying
# catalogues.
#
# Arguments:
#  1 - Comment, typically the test case name
#  2 - Version, The installed version
#
# Returns: Returns 0 on success, otherwise 255
verify_hlhdf_directory_structure() {
  verify_directory "$1" "$2" || return 255
  verify_directory "$1" "$2/bin" || return 255
  verify_directory "$1" "$2/include" || return 255
  verify_directory "$1" "$2/lib" || return 255
  verify_directory "$1" "$2/mkf" || return 255
  
  return 0
}

###
# Verifies the contents of a version file. I.e. checks
# the contents of the file with the provided version.
#
# Arguments:
#  1 - Comment, typically the test case name
#  2 - Filename, the name of the file, e.g. .version
#  3 - Version, The version number to check for
#
# Returns: Returns 0 on success, otherwise 255
verify_version_file_contents() {
  TMPVER=`cat $2`
  if [ "x$TMPVER" != "x$3" ]; then
    echo "Test [$1] failed: $2 reads $TMPVER but should be $3"
    return 255
  fi
  return 0
}

###
# Verifies that the installation structure is correct and
# that the relevant files and symbolic links has been created.
#
# Arguments:
#  1 - Comment, typically the test case name
#  2 - installdir, the installation base directory
#  3 - Version, the version of the installation
#
# Returns: Returns 0 on success, otherwise 255
verify_installation() {
  change_directory "$2" || return 255
  
  verify_hlhdf_directory_structure "$1" "$3" || return 255

  verify_file "$1" "hlhdf.pth" || return 255

  verify_file "$1" ".version" || return 255

  verify_version_file_contents "$1" ".version" "$3" || return 255

  return 0
}

###
# Verifies that the pre 0.7.5 structure has been left intact
# but moved to a folder called pre-0.7.
#
# Arguments:
#  1 - Comment, typically the test case name
#  2 - Installdir, the installation base directory
#
# Returns: Returns 0 on success, otherwise 255
verify_pre075_installation() {
  change_directory "$2" || return 255
  
  verify_hlhdf_directory_structure "$1" "pre-0.7" || return 255

  return 0
}  

###
# Prepares a tarball for building by removing old
# build directory, extracting the tar ball and
# verify that the source contains the nessecary files.
#
# Arguments:
#  1 - Comment, typically the test case name
#  2 - Builddir, the directory where the tarball should be extracted
#  3 - Tarball, the actual tarball
#  4 - Version, the version that should be built
#
# Returns: Returns 0 on success, otherwise 255
prepare_build() {
  remove_and_create_directory "$2" || return 255

  change_directory "$2" || return 255

  tar -xvzf "$3"
  
  # Verify source files and directory structure
  verify_source_structure "$1" "$4" || return 255
  
  return 0
}

###
# Runs the configure, make, make test and make install sequence
# as one operation.
#
# Arguments:
#  1 - Comment, typically the test case name
#  2 - Builddir, the directory where the tarball should be extracted
#  3 - Installdir, the directory to install the deliverables in
#  4 - Version, the version that should be built and installed
#  5 - Configcmd, the configure command
#
# Returns: Returns 0 on success, otherwise 255
run_config_make_sequence() {
  change_directory "$2/hlhdf-$4" || return 255

  $5 --prefix="$3"
  if [ $? -ne 0 ]; then
    echo "Test [$1] failed: Failed to configure with command $5 --prefix=$3"
    return 255
  fi
  
  execute_command "$1" "make" || return 255

  execute_command "$1" "make test" || return 255

  execute_command "$1" "make install" || return 255

  return 0  
}

###
# Executes one installation test, i.e. verifies deliverable
# that configure, make, make test, make install works as expected.
# It also verifies that the structure of the installed software
# is correct, etc. etc.
#
# Arguments:
#  1 - Comment, typically the test case name
#  2 - Tarball, the deliverable
#  3 - Version, the version that should be built and installed
#  4 - Builddir, the directory where the software should be built in
#  5 - Installdir, the directory where the software should be installed in
#  6 - Configcmd, the configure command (excluding the prefix)
#
# Returns: Returns 0 on success, otherwise 255
execute_one_install_test() {
  # Setup
  prepare_build "$1" "$4" "$2" "$3" || return 255

  # Configure, make, make test and make install
  remove_directory "$5"
  run_config_make_sequence "$1" "$4" "$5" "$3" "$6" || return 255

  # Verify that structure is correct on installation
  verify_installation "$1" "$5" "$3" || return 255
  
  remove_directory "$4"
  remove_directory "$5"

  return 0
}

###
# More or less like execute_upgrade_test with the exception
# that it verifies that it is possible to upgrade.
#
# Arguments:
#  1 - Comment, typically the test case name
#  2 - Tarball, the deliverable
#  3 - Version, the version that should be built and installed
#  4 - Builddir, the directory where the software should be built in
#  5 - Installdir, the directory where the software should be installed in
#  6 - Configcmd, the configure command (excluding the prefix)
#
# Returns: Returns 0 on success, otherwise 255
execute_upgrade_test() {
  # Setup
  prepare_build "$1" "$4" "$2" "$3" || return 255
  
  create_fake_directory_structure "$1" "$5" "v0.7.4"
  
  # Configure, make, make test and make install
  run_config_make_sequence "$1" "$4" "$5" "$3" "$6" || return 255

  # Verify that structure is correct on installation
  verify_installation "$1" "$5" "$3" || return 255

  # Verify that the old structure still exists
  change_directory "$5" || return 255
  
  verify_hlhdf_directory_structure "$1" "v0.7.4" || return 255
  
  verify_version_file_contents "$1" ".rollback" "$3" || return 255
  
  return 0  
}

###
# More or less like execute_upgrade_test with the exception
# that it verifies that it is possible to upgrade from a
# old HLHDF release.
#
# Arguments:
#  1 - Comment, typically the test case name
#  2 - Tarball, the deliverable
#  3 - Version, the version that should be built and installed
#  4 - Builddir, the directory where the software should be built in
#  5 - Installdir, the directory where the software should be installed in
#  6 - Configcmd, the configure command (excluding the prefix)
#
# Returns: Returns 0 on success, otherwise 255
execute_pre075_upgrade_test() {  
  # Setup
  prepare_build "$1" "$4" "$2" "$3" || return 255
  
  create_fake_old_directory_structure "$5"
  
  # Configure, make, make test and make install
  run_config_make_sequence "$1" "$4" "$5" "$3" "$6" || return 255

  # Verify that structure is correct on installation
  verify_installation "$1" "$5" "$3" || return 255

  # Also verify that the old hlhdf release has been translated into a
  # pre-0.7 directory
  verify_pre075_installation "$1" "$5" || return 255
  
  return 0
}

SCRFILE=`python -c "import os;print os.path.abspath(\"$0\")"`
SCRIPTPATH=`dirname "$SCRFILE"`
cd "$SCRIPTPATH/.."

CONFIGURECMD=./configure
if [ -f .test_conf ]; then
  CONFIGURECMD=`cat .test_conf`
else
  echo "No configuration parameters setup, will only use ./configure"
  echo "If you want some other ./configure parameters, edit the file .test_conf in the root directory of HLHDF"
  echo "with only one line containing the full ./configure command."
  echo "NOTE! Do not specify --prefix since that is automatically generated by this script"
fi

TB=$1
VER=$2
if [ ! -f "$TB" ]; then
  echo "No '$TB' file found"
  exit 255
fi
if [ "x" = "x$VER" ]; then
  echo "No version specified"
  exit 255
fi

echo "Startinng build and installation tests"
echo "Log messages will be stored in /tmp/hlhdf_buildandinstall.log"

remove_and_create_file "/tmp/hlhdf_buildandinstall.log" || exit 255

echo "Running test with no whitespaces in either build or install directory"
execute_one_install_test "Standard" "$TB" "$VER" "/tmp/hlhdfbuild" "/tmp/hlhdfinstall" "$CONFIGURECMD" 2>&1 >> /tmp/hlhdf_buildandinstall.log || exit 255

echo "Running test where whitespaces are in the build directory"
execute_one_install_test "Whitespace in build" "$TB" "$VER" "/tmp/hlhdf build" "/tmp/hlhdfinstall" "$CONFIGURECMD" 2>&1 >> /tmp/hlhdf_buildandinstall.log || exit 255

echo "Running test where whitespaces are in the install directory"
execute_one_install_test "Whitespace in install" "$TB" "$VER" "/tmp/hlhdfbuild" "/tmp/hlhdf install" "$CONFIGURECMD" 2>&1 >> /tmp/hlhdf_buildandinstall.log || exit 255

echo "Running test where whitespaces are in both build and install directory"
execute_one_install_test "Whitespace in both build and install" "$TB" "$VER" "/tmp/hlhdf build" "/tmp/hlhdf install" "$CONFIGURECMD" 2>&1 >> /tmp/hlhdf_buildandinstall.log || exit 255

echo "Running upgrade test from old version (pre 0.7.5 to a newer)"
execute_pre075_upgrade_test "Upgrade test from pre0.7.5 to newer" "$TB" "$VER" "/tmp/hlhdfbuild" "/tmp/hlhdfinstall" "$CONFIGURECMD" 2>&1 >> /tmp/hlhdf_buildandinstall.log || exit 255

echo "Running upgrade test from old version"
execute_upgrade_test "Upgrade test" "$TB" "$VER" "/tmp/hlhdfbuild" "/tmp/hlhdfinstall" "$CONFIGURECMD" 2>&1 >> /tmp/hlhdf_buildandinstall.log || exit 255

echo "Tests executed successfully"
exit 0