#!/bin/bash
# Syntax: build-package.sh version

# Before running this script, tag a new version:
# $ git tag v0.1.3
# $ git push origin tags/v0.1.3

VERSION="$1"
DISTRIBUTION="trusty"

URL_REPO=https://github.com/KeyviDev/keyvi
PACKAGE_NAME=keyvi

# Setting umask
UMASK=$(umask)
umask 0022

# this is where our build script is.
WORKDIR=$(echo $0 | sed 's,[^/]*$,,; s,/$,,;')
[ -z "$WORKDIR" ] && WORKDIR=$PWD

# this is where the package is going to be built
BUILD_DIR=$WORKDIR/../archives/

# repo dir
THIS_REPO_DIR=$WORKDIR/../

trap "script_cleanup" EXIT


# check local environment for all required apps/tools
function checkEnv() {
	if [ ! -x "/usr/bin/wget" -o ! -x "$(which wget)" ]
	then
		die "Cannot find curl"
	fi

	if [ ! -x "/usr/bin/git" -o ! -x "$(which git)" ]
	then
		die "Cannot find git"
	fi

	if [ ! -x "/usr/bin/gpg" -o ! -x "$(which gpg)" ]
	then
		die "Cannot find gpg"
	fi
}

# this function is called whenever the script exits
# and it performs some cleanup tasks
function script_cleanup() {

	# setting back umask
	umask $UMASK
}

# report error and exit
function die() {
	echo -e "$0: $1"
	exit 2
}

# check for local requirements
checkEnv

[ ! -z "$VERSION" ] || die "Expected a version number as a parameter"

############################
echo "Starting build...."
############################

dch --distribution $DISTRIBUTION -v $VERSION
git add debian/changelog
git commit -m "release version $VERSION"

mkdir -p $BUILD_DIR/$VERSION/build

cd $BUILD_DIR/$VERSION
wget $URL_REPO/archive/v$VERSION.tar.gz -O keyvi_$VERSION.orig.tar.gz

cp -r ../../debian build/
cd build
tar xvfz ../keyvi_$VERSION.orig.tar.gz --strip-components 1
debuild -S

echo "to upload to PPA use dput keyvi {PACKAGE}.changes"
