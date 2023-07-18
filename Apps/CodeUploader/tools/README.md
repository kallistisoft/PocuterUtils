# PocuterUtil :: pocuter-deploy -- Pocuter Application Deployment Tool
**This command line tool compiles, packages, and uploads a Pocuter application to a ['Code Upload Server'](../).**

***NOTE: The 'pocuter-deploy' tool has been integrated into the [Official Pocuter GitHub Repository](https://github.com/pocuter/pocuter-deploy), all future updates will posted to that repository!***


## Tool Features
- Options for compiling, packaging, and uploading an application
- Capable of using both the windows and java app convertion programs
- Automatically reads/writes application metadata using the INI file
- Uses the standard python library - no extra python packages required
- Environment variables for persisting commonly used options

## Software Requirements
- [Pocuter app converter](https://github.com/pocuter)
- [arduino-cli](https://arduino.github.io/arduino-cli/latest/installation)
- Python3
- curl

## Tool Usage
This tool is designed to be run from the root folder of an Arduino project from a Linux, WSL, or MacOS terminal. In this folder it expects there to be a Pocuter application metadata file having the same name as the folder and ending in ***'.ini'***

This tool is used by being called with one of the four command modes: [***build***](#build-command), [***package***](#package-command), [***upload***](#upload-command), and [***deploy***](#deploy-command).

## Build Command:
The ***build command*** compiles the ***'.ino'*** application source code using the ***arduino-cli*** tool. The files are compiled in a temporary folder and the resulting binary is copied into the current folder. This command has the same effect as using the 'Sketch -> Export Compiled Binary' option from the Arduino GUI program.

The ***build command*** accepts an optional ***'--flags='*** argument which is passed to ***arduino-cli***. This option can be used for example to force a --clean build or to have ***arduino-cli*** produce --verbose output. Multiple --flags arguments can be used.

### Examples:
```Shell
    # simple build command
    pocuter-deploy build

    # build command, clean build folder and verbose compilation
    pocuter-deploy build --flags --clean --flags --verbose
```


## Package Command:
The ***package command*** uses the selected app conversion program to convert the arduino binary into a packaged pocuter application. By default the tool uses the windows (***appconverter.exe***) program, but this can be changed by using the ***'--converter='*** option or by setting the ***POCUTER_DEPLOY_PACKAGER*** environment variable.

### Application Metadata:
The package will be built using the application ID and version number specified with the ***'--id='*** and ***'--version='*** options respectively. **These values will be written to the app's metadata INI file!**

If no ***'--id='*** or ***'--version='*** options are given the tool will attempt to read these values from the app's metadata INI file.

The embedding of these values into the INI file means that these values only need to be updated when needed and can be tracked via version control systems like GitHub.

### Specifying the Application Converter to Use
The app conversion program can be changed from the default (***appconverter.exe***) either by setting the environment variable ***POCUTER_DEPLOY_PACKAGER*** or by passing the ***'--converter='*** option. If a file ending in ***'.jar'*** is given then the tool will run that file using the java executable. Otherwise, the tool will test to see if the given executable is in the path or in the current directory.

### Examples:
```Shell
    # run packager and set application id and version
    pocuter-deploy package --id 101234 --version 1.0.0

    # run packager and update version number
    pocuter-deploy package --version 1.1.0

    # run packager using currently stored metadata
    pocuter-deploy package

    # run packager using the java app converter
    pocuter-deploy package --converter=./PocuterAppConverter.jar
```

## Upload Command
The ***upload command*** uploads a packaged Pocuter application to a 'Code Upload Server' at the specified ip address or hostname. If no address is given then the tool will use the value of the ***POCUTER_DEPLOY_ADDRESS*** environment variable.

The ***upload command*** has a single optional argument flag ***'--yes'*** that bypasses the upload confirmation prompt.

### Examples:
```Shell
    # upload packaged app to server
    pocuter-deploy upload 192.168.1.100

    # upload packaged app to server, skip confirmation
    pocuter-deploy upload --yes 192.168.1.100

    # upload packaged app to server, skip confirmation, use address variable
    pocuter-deploy upload --yes
```

## Deploy Command
The ***deploy command*** executes all three commands in order: ***build***, ***package***, ***upload*** and accepts all of the optional arguments of those commands.

***NOTE**: This command always returns successfully as long as it can talk to the server. The fact that it returns a successfull error code does not mean that the application upload was successful - it is necessary to read the status prompt returned by the server to determine if a validation error occurred!*

### Examples:
```Shell
    # deploy app to server and update app version number
    pocuter-deploy deploy --version 1.2.0 192.168.1.100

    # deploy app to server using existing metadata
    pocuter-deploy deploy 19.168.1.100

    # deploy app to server using existing metadata and address from environment
    pocuter-deploy deploy --yes
```

***
## Environment Variables
There are two environment variables that can be set to automatically define often repeated options. These variables are **POCUTER_DEPLOY_PACKAGER** to set the location of the app coversion program, and **POCUTER_DEPLOY_ADDRESS** to set the address or hostname of the 'Code Upload Server'. These variables can persist between sessions by adding them to your shell initialization script (~/.bashrc, ~/.zshrc, etc..):

### Examples:
```Shell
    # set the location of the package converter
    export POCUTER_DEPLOY_PACKAGER=~/PocuterAppConverter.jar

    # set the location of the code upload server
    export POCUTER_DEPLOY_ADDRESS=192.168.1.100
```

***

## Running From a Windows CMD Prompt
This tool is capable of running from a Windows CMD or Powershell instance, but to do so requires that Python3 and [GNU curl](https://curl.se/windows/) be present in the path. Although this can be done, it isn't recommended, please use [WSL](https://ubuntu.com/wsl) if you are running Windows.

***

## Complete Usage Text
```
Pocuter Code Deployment Tool (pocuter-deploy).

Usage: 
pocuter-deploy COMMAND [--help] [options] [ip-address]

Options:
  -h, --help            show this help message and exit


Available Commands:
  build       Compile source code using arduino-cli
  package     Package binary image using appconverter.exe
  upload      Upload packaged application to 'Code Upload' server
  deploy      Compile, package, and upload application

  use the --help option with a command for more information...


  Build command options:
    Compile arduino source code with optional flags. The flag argument can
    be specified multiple times.

    -f BUILD_FLAG, --flags=BUILD_FLAG
                        compile flag to pass arduino-cli

  Package command options:
    If --appid or --version are supplied they will be *written* to the
    metadata file, otherwise these values are *read* from the metadata
    file. If the environment variable POCUTER_DEPLOY_PACKAGER exists it
    will be used for packaging the app.

    -i APPID, --id=APPID
                        application ID number [> 100000]
    -v VERSION, --version=VERSION
                        application version string in the form [x.y.z]
    -c PACKAGER, --converter=PACKAGER
                        path to app converter .exe or .jar file...
                        (appconverter.exe)

  Upload command options:
    Upload a packaged pocuter application from the ./apps/ folder to the
    code upload server at the given ip address or hostname. If the address
    is omitted it will be read from the environment variable:
    POCUTER_DEPLOY_ADDRESS

    -y, --yes           skip upload confirmation prompt

  Deploy command options:
    The deploy command accepts all of the previous options...


Usage Notes:
  This tool is designed to be run from the root of an arduino sketch folder.
  It expects to find a file ending in '.ino' and a file ending in '.ini' both
  having the same name as the folder.
```