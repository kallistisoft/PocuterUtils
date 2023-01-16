# PocuterUtil::Keyboard -- Flexible Keyboard Utility Class
- Jump to: [Hardware Input](#hardware-input)
- Jump to: [API Documentation](#pocuterutilkeyboard-class-api)
- Jump to: [Quick Usage Example](#quick-usage-example)
- Jump to: [Advanced Usage Example](/Apps/KeyboardDemo)
***


## Keyboard Features:
- Compossible keyboard character sets (uppercase,lowercase,numeric,symbols,space,...)
- Pre-defined character/input modes for common use cases: decimal, float, hex, ip-address, hostname
- Custom character set; define your own custom character set and ordering
- Ability to bind keyboard to a PocuterConfig settings file for persistance
- User-defined maximum string length with max length indication
- Delete character key; auto-delete by holding select button
- Auto-scroll character set by holding up/down button
- Custom display text color, default is C_LIME


## Pre-Defined Character Set Bit-Flags:
**Primitive alphabetic character sets:**
- **KEYSET_UPPER:** Letter characters **'A-Z'**
- **KEYSET_LOWER:** Letter characters **'a-z'**
- **KEYSET_SYMBOLS:** Symbolic characters **'!@#$%...'**
- **KEYSET_SPACE:** Space character **' '**

**Numeric character sets:**
- **KEYSET_NUMERIC:** Numeric characters **'0-9'**
- **KEYSET_FLOAT:** Numeric characters **'0-9'** and **'.'** symbol
- **KEYSET_HEX:** Hexadecimal characters **'0-9'** and **'A-F'**
- **KEYSET_NEGATIVE:** Meta-flag for enabling negative number input

**Complex alphabetic character sets:**
- **KEYSET_ALPHA:** Combination of upper and lower key sets
- **KEYSET_ALPHA_NUMERIC:** Combination of alpha and numeric key sets
- **KEYSET_FULL:** Combintation of alpha, numeric, symbols, and space key sets

**Internet address character sets:**
- **KEYSET_HOSTNAME:** Combination of lowercase alpha numeric key sets and **'-.'** characters
- **KEYSET_IPADDR:** Combination of numeric key set and **'.'** character

**User-defined character set:**
- **KEYSET_CUSTOM:** Empty keyset; use the ***custom()*** member function to manually set the key set


## Special Input Modes:
**The following keyset flags will trigger special input behaviour:**

- **KEYSET_IPADDR:** Assigns the ip address key set, sets the max string length to 15 characters, and enables auto-formatting of address octets
- **KEYSET_HOSTNAME:**  Assigns lowercase alpha-numeric and **'[-.]'** symbols, enables hostname formatting restrictions

- **KEYSET_NEGATIVE:** Numeric type modifier that adds **'[-]'** MINUS character and enables negative number formatting restrictions
- **KEYSET_FLOAT:**  Assigns the numeric key set and **'[.]'**, restricts usage of **'[.]'** character and **'[0]'** prefixes decimal values


## User-Defined Character Sets:
The available key set can be changed at any time by calling the ***custom()*** member function and passing it a string containing the desired keyboard character set. The keyboard will display the provided characters in the order that they are given and will automatically format the **' '** SPACE character to **'[ ]'**

To aid in creating a user-defined key set there are several ***#defines*** that contain common character strings:

- **CHARSET_UPPER:** String literal of letter characters **'A-Z'**
- **CHARSET_LOWER:** String literal of letter characters **'a-z'**
- **CHARSET_NUMERIC:** String literal of numeric characters **'0-9'**
- **CHARSET_SYMBOLS:** String literal of symbolic characters **'!@#$%...'**
- **CHARSET_HEX:** String literal of hexadecimal characters **'0-9'** and **'A-F'**
- **CHARSET_SPACE:** String literal of the SPACE character **' '**


***
# Hardware Input

**The keyboard input routine *getchar()* uses all three of the hardware buttons on the Pocuter:**

- **BUTTON_A:** Scroll keyboard cursor upwards
- **BUTTON_B:** Scroll keyboard cursor downwards
- **BUTTON_C:** Append keyboard character, delete, return

These buttons can be held to activate auto-scroll and auto-delete behaviors.

It is the calling application's responsibility to call the Pocuter-OS ***updateInput()*** function to update the button states.

The keyboard does not implement any form of cancel event; it is up to the calling application to implement this functionality manually.


***
# PocuterUtil::Keyboard Class API

## Class Definition:
```C
// keyboard 'in-use' flag
bool active;

// auto-update display (default: true)
bool autoupdate;

// display text color (default: C_LIME)
UG_COLOR color;

// instantiate keyboard object
Keyboard( Pocuter *pocuter, char *label, uint keyset, uint maxlen );

// assign custom key set
void custom( char *charset );

// clear/set/get keyboard text buffer
void clear();
void set( char *newtext );
char* get();

// display keyboard and handle user input
bool getchar();
```

##  Class Constructor:
```C
PocuterUtil::Keyboard( 
    /* pocuter system object */
    Pocuter *pocuter,
    
    /* keyboard display label (max length: 16) */
    char *label,
    
    /* key set flags, default is all characters */
    uint keyset = KEYSET_FULL,
    
    /* keyboard text length, default is max: 255 */
    uint maxlen = KEYSET_STRING_MAX
);
```

**Instantiate a new keyboard object by passing the pocuter system object, a display label string, (optional) compossible character set bit-flags, and (optional) maximum keyboard text size.**

**Multiple key set flags can be combined using OR '|' to create a custom keyboard character set; example lowercase letters and the space character would be: (KEYSET_LOWER | KEYSET_SPACE)**

**If you are going to use a user-defined key set it is recommended that you pass the custom key set flag: KEYSET_CUSTOM, unless you also need to invoke one of the builtin special input processing modes: IPADDR, HOSTNAME, FLOAT**

**It is recommend to create a separate keyboard object for each input field in your application. Doing so makes getting/setting the field values much simpler and doesn't require storing the field value in a separate non-volatile variable.**

**Example invocations:**
```C
// create keyboard for entry of floating point number, allow negative values, limit to 12 characters
new PocuterUtil::Keyboard( pocuter, (char*)"Enter float value", KEYSET_FLOAT | KEYSET_NEGATIVE, 12 );

// create keyboard for entry of hexadecimal number, limit to 6 characters
new PocuterUtil::Keyboard( pocuter, (char*)"Enter color code", KEYSET_HEX, 6 );

// create keyboard for entry of ip address
new PocuterUtil::Keyboard( pocuter, (char*)"Enter IP Address", KEYSET_IPADDR );

// create keyboard for entry of upper/lower case text without spaces, limit to 32 characters
new PocuterUtil::Keyboard( pocuter, (char*)"Enter Text", KEYSET_ALPHA, 32 );

// create keyboard for entry of upper/lower case text and allow spaces, limit to 32
new PocuterUtil::Keyboard( pocuter, (char*)"Enter Text", KEYSET_ALPHA | KEYSET_SPACE, 32 );

// create keyboard for entry of lower case text and numbers, no spaces, maximum size (255 chars)
new PocuterUtil::Keyboard( pocuter, (char*)"Enter Text", KEYSET_LOWER | KEYSET_NUMERIC );
```

***
### void custom( char *charset ):  Set user defined keyboard character set
	void custom( char *charset );
User defined character sets can be assigned using the custom() method...
```C
/* Alternating upper/lower keyboard including space character: */
keyboard->custom( (char *)( "AaBbCc..Zz" CHARSET_SPACE ))

/* Equation Editor: numeric with limited symbol characters and no-spaces: */
keyboard->custom( (char *)( CHARSET_NUMERIC "+-/*^()" ))

/* Hexadecimal Fraction: invoke with KEYSET_FLOAT in the constructor to trigger special float input handling */
keyboard->custom( (char *)( CHARSET_HEX "." ))

/* MAC Address: invoke with maxlen=18 and use a user-defined post-processor to perform auto-formatting */
keyboard->custom( (char *)( CHARSET_HEX ":" ))
```
***
### bool active: Keyboard enabled flag
```C
bool active;
```
This member variable tracks the in-use state of the keyboard. It is automatically set to true when calling ***getchar()*** and is set to false when the user selects the **'[OK]'** option from the keyboard.

This variable is fully exposed and is usefull for manually managing keyboard flow control, see [Quick Usage Example](#quick-usage-example) section for flow control implementation.
***
### bool autoupdate: Automatically update display
```C
bool autoupdate = true;
```
This member variable enables automatic updating of the screen when calling ***getchar()***. This variable is true by default. Applications that wish to do post-processing of the display can set this to false and then update the screen manually. See the [Keyboard Demo Application](/Apps/KeyboardDemo) for a usage example.
***
### bool color: Keyboard text color
```C
UG_COLOR color = C_LIME;
```
This member variable holds the keyboard text display color. It is set to the defult system color (C_LIME) on initialization.
***
### bool bind( char *section, char *name, bool autoload): Bind the keyboard to a config setting
```C
void bind( char *section, char *name, bool autoload=false );
```
This function binds the keyboard to a PocuterConfig settings file for data persistance.
This function automatically uses the config file name ***"settings"*** this is maintain compatability with the ***getSetting(...)*** and ***setSetting (...)*** helper functions included with the BaseApp template file.

Note that the binding functions bind()/load()/save() will not work unless the app has been loaded from an SD card as it requires both SD storage and the unique App ID provided by the menu application loader. 

**At this time the binding functions will not work properly with applications flashed directly to ROM!**
***
### bool bind( char *configName, char *section, char *name, bool autoload): Bind the keyboard to a config setting in custom file
```C
void bind( char *configName, char *section, char *name, bool autoload=false );
```
This function binds the keyboard to a custom PocuterConfig settings file for data persistance.
This function is incompatible with the ***getSetting(...)*** and ***setSetting (...)*** helper functions included with the BaseApp template file as they expect the config file name to be "settings".
***
### void clear(): Clear keyboard text
```C
void clear();
```
This function clears the keyboard text string and resets the keyboard selection cursor
***
### void set( char *newtext ): Set keyboard text
```C
void set( char *newtext );
```
This function sets the keyboard text by copying the value from ****newtext*** up to the maximum defined length of the keyboard text string
***
### char* get( ): Get keyboard text
```C
char* get();
```
This function returns a raw pointer to the keyboard text buffer. This pointer can be used to directly modify the contents of the keyboard text buffer. Subsequent calls to ***getchar()*** can/will modify this value. It is the application's responsibility to copy this value to a non-volatile memory location if persistence is required.

**THIS FUNCTION DOES NOT PERFORM NUMERIC TYPE CONVERTION** -- It is the application's responsibility to perform numeric type convertion when calling ***set()*** or ***get()*** respectively
***
### bool getchar(): Process keyboard display and input
```C
bool getchar()
```
**This function displays the keyboard on screen and handles all user input.** **The application MUST call** ***updateInput()*** **before calling this function!**

This function's draw algorithm automatically clears the screen and calls ***pocuter->Display->updateScreen();*** if the ***autoupdate*** member variable is true (default). 

If an application so desires, it can disable the ***autoupdate*** flag and perform additional draw calls afterwards for example:  displaying an icon in the label area, or overwriting the label as part of 'smart' keyboard post processing.

The function returns a boolean flag indicating that the contents of the keyboard text buffer has changed. This flag can be used to trigger post-processing of the keyboard text to implement a user-defined smart keyboard.

If doing custom post-processing be sure to check the value of the ***bool active*** flag as the **'[OK]'** event triggers a truthy return of the ***getchar()*** function.

See the source code of the [Keyboard Demo Application](/Apps/KeyboardDemo) for advanced usage examples.

***
# Quick Usage Example
```C
#include "Keyboard.h"

PocuterUtil::Keyboard *keyboard;

void setup() {
    // initialize pocuter system object
    pocuter = new Pocuter();
    
    // create new keyboard using pocuter object
    //  set custom label text
    //  combine key set flags == upper+lower case and space character
    //  use default length (MAX: 255)
    keyboard = new PocuterUtil::Keyboard( pocuter, (char*)"This is a label", KEYSET_ALPHA | KEYSET_SPACE );
    
    // bind keyboard to the default configuration file (optional)
    keyboard->bind( (char*) "my_config_section", (char*) "my_config_item" );

    // load saved value otherwise set default value
    if( !keyboard->load() ) keyboard->set( (char*) "This is a default value" );

    ...
}

void loop() {
    // UPDATE BUTTON INPUT STATE -- MUST BE CALLED BY THE APPLICATION
    updateInput();
    
    // process keyboard display and input
    if( keyboard->active ) {

        // get next keyboard character
        if( keyboard->getchar() ) {

            // test if user selected '[OK]'
            if( !keyboard->active ) {
                //
                // perform keyboard closing actions
                //
                
                ...

                // save keyboard value to bound config setting
                keyboard->save();
            }
        }
        return;
    }
    
    if( /* trigger event for activating keyboard */ ) {
        keyboard->active = true;
    }
    
    ...
    
    // display keyboard text value
    gui->UG_PutStringSingleLine(0, 0, keyboard->get() );
    pocuter->Display->updateScreen();
}
```

***
# Advanced Usage Example
See the [Keyboard Demo Application](/Apps/KeyboardDemo) for advanced usage examples -- How to bind a keyboard to a configuration setting, multiple keyboards, and 'smart' keyboard examples.
