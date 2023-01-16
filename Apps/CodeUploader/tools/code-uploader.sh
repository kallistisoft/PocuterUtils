#!/usr/bin/env bash
# code-uploader :: CLI tool for uploading a Pocuter package file to a 'Code Uploader' server
# Copyright 2022 Kallistisoft
# MIT License -- https://opensource.org/licenses/MIT

#
# TODO: change options processing to getopts and add -y option
#
# Consider using a config file ~/.code-uploader.conf
#   the config file would contain the environment variables
#   use eval to import into the script
#

SELF=$(basename $0);
IPADDR=$1

# define: usage text
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
USAGE=$(cat << EOF

USAGE: $SELF [-y] [IP-ADDRESS]

    This script should be run in your project's root folder.

    It checks for the existance of an ./apps/ folder and
    uploads the detected Pocuter application package to
    the 'Code Upload Server' running at IP-ADDRESS

    The ip address argument can be ommited if the shell
    contains the environemt variable:

        CODE_UPLOADER_IP_ADDRESS
 
    If CODE_UPLOADER_IP_ADDRESS is set then the upload
    confirmation prompt will be skipped.
 

EOF
);

# help: show usage info
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
if [[ "$1" == '--help' || "$1" == '-h' ]]; then
    echo "$USAGE";
    exit 1;
fi

# empty: missing IP address argument - test environment variable
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
if [[ -z "$1" ]]; then
    if [[ -z "$CODE_UPLOADER_IP_ADDRESS" ]]; then
        echo "$USAGE";
        exit 1;
    fi
    IPADDR=$CODE_UPLOADER_IP_ADDRESS
fi




# test: current working directory has an './apps/' sub-folder
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
if [[ ! -d ./apps/ ]]; then
    echo "$SELF: Application package folder './apps/ doesn't exist!"
    exit 1
fi



# get: application sub folder and app id number
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
appid=$(basename ./apps/* 2>/dev/null)

# detect: application id number from './apps/' contents
if [[ -z "$appid" ]]; then
    echo "$SELF: Can't determine the application ID number from the './apps/' folder!"
    echo "$SELF: The './apps/' folder should only contain a single numbered sub-folder!"
fi

# test: application id is numeric
[ -n "$appid" ] && [ "$appid" -eq "$appid" ] 2>/dev/null;
if [ $? -ne 0 ]; then
    echo "$SELF: Can't determine the application ID number from the './apps/' folder!"
    echo "$SELF: The './apps/' subfolder ('$appid') is non-numeric!"
fi



# get: pocuter image file
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
image="./apps/$appid/esp32c3.app"

if [[ ! -e $image ]]; then
    echo "$SELF: Can't find pocuter application image file: '$image'"
    exit 1
fi

# get: image file statistics - ( name, MD5, size )
name=$(strings $image | awk 'NR==3,NR==10{ print }' | grep -i Name | awk -F= '{print $2}')
md5=$(md5sum -b $image | awk '{print $1}') 2>/dev/null
size=$(wc -c < $image)



# test: ping upload server ip address
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
if [[ -z "$(ping -W0.5 -qc1 $IPADDR | grep '1 received')" ]]; then
    echo "$SELF: Unable to ping 'Code Upload Server' server at: $IPADDR"
    exit 1
fi



# display: application upload info
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
echo
echo "Ready to upload application to $IPADDR..."
echo
echo "   ID: $appid"
echo " Name: $name"
echo
echo "  MD5: $md5"
echo " File: $image"
echo " Size: $size bytes"
echo
echo

# confirm: show confirmation dialog
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
if [[ ! -n "$CODE_UPLOADER_IP_ADDRESS" ]]; then
    read -p "Do you wish to upload this program? [Y/n]: " -n 1 -r REPLY
    echo
    if [[ $REPLY =~ ^[Nn]$ ]]; then
        exit 2
    fi
fi

# upload: send request to server
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
res=$(curl -# -F appID=$appid -F appSize=$size -F appMD5=$md5 -F appImage=@$image http://$IPADDR/upload)
echo
echo $res
echo
