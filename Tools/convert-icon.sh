#!/usr/bin/bash
# convert-icon :: image to RGBA base64 encoded string utility
# Copyright 2022 Kallistisoft
# MIT License -- https://opensource.org/licenses/MIT

SELF=$(basename $0);

# show: help usage info
if [[ -z "$1" || "$1" == '--help' ]]; then
	echo "USAGE: $SELF FILENAME"
	exit 1
fi

# test: input file exists
if [[ ! -e "$1" ]]; then
	echo "$SELF: unable to open file [$1]"
	exit 1
fi

# test: imagemagick convert utility is installed
if [[ -z $(which convert) ]]; then
	echo "$SELF: unable to find the imagemagick 'convert' utility..."
	exit 1
fi

# test: base64 encode utility is installed
if [[ -z $(which base64) ]]; then
	echo "$SELF: unable to find the 'base64' utility..."
	exit 1
fi

# convert: using imagemagick
convert $1 -channel RGBA -background black -alpha remove -alpha off $1.rgba

# test: output file created and convert to base64
if [[ -e "$1.rgba" ]]; then
	base64 $1.rgba -w0 > $1.base64
	echo "$SELF: converted [$1] to base64 string [$1.base64]"
	rm $1.rgba
	exit 0
fi

# error: problem with imagemagick conversion
echo "$SELF: error converting input file [$1]"
exit 1


