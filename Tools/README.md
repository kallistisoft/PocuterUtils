# PocuterUtil :: Tools -- Pocuter Development Tools

This is a collection of tools for aiding in the development of applications for the Pocuter ecosystem

***

## convert-icon.sh -- Icon Conversion Utility Script
This script simplifies the process of converting an icon image file to a base64 encoded string suitable  for inclusion in an application metatadat INI file.

This script requires both the [ImageMagick](https://imagemagick.org/index.php) image toolkit and the GNU base64 binary to be installed and available viable the path.

**Usage:**<br/>convert-icon.sh FILENAME

This will produce a file ending in '.base64' in the current directory

***

## pocuter-deploy -- Pocuter Application Deployment Tool
This is a command line tool for compiling, packaging, and uploading a Pocuter application to a 'Code Upload Server'.

[Please see the README file for this tool for more information](/Apps/CodeUploader/tools/)
