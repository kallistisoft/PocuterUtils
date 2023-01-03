//
// Copyright 2023 Kallistisoft
// GNU GPL-3 https://www.gnu.org/licenses/gpl-3.0.txt
/*
* [PocuterUtils]/Libs/Keyboard/Keyboard.h
* 
* PocuterUtils::Keyboard -- Pocuter Utility Class for Flexible Keyboard Input
*
* See README.md file for details, examples, and usage guide
*/

#ifndef _POCUTERUTIL_KEYBOARD_H_
#define _POCUTERUTIL_KEYBOARD_H_

#include <Pocuter.h>
#include <cstring>

// local: max length constraints
#define KEYSET_WIDTH_MAX    12
#define KEYSET_STRING_MAX   255
#define KEYSET_BLINK_LEN    250

// local: special format keys
#define KBD_CHAR_SPACE   ' '
#define KBD_CHAR_DELETE  '\r'
#define KBD_CHAR_RETURN  '\n'

// global: character set primitive strings
#define CHARSET_NONE     "\r\n"
#define CHARSET_UPPER    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define CHARSET_LOWER    "abcdefghijklmnopqrstuvwxyz"
#define CHARSET_NUMERIC  "0123456789"
#define CHARSET_SYMBOLS  ",./\\;:[]!@#$%^&*()_+<>?'\"`{}|~-="
#define CHARSET_SPACE    " "

// global: character set complex strings
#define CHARSET_IPADDR   "." CHARSET_NUMERIC
#define CHARSET_FLOAT	 "." CHARSET_NUMERIC
#define CHARSET_HOSTNAME CHARSET_LOWER CHARSET_NUMERIC "-."
#define CHARSET_HEX      CHARSET_NUMERIC "ABCDEF"

// global: key set primitive bit-flags
#define KEYSET_CUSTOM    0x0000 /**< keyset flag: user-defined */
#define KEYSET_UPPER     0x0001 /**< keyset flag: upper-case letters */
#define KEYSET_LOWER     0x0002 /**< keyset flag: lower-case letters */
#define KEYSET_NUMERIC   0x0004 /**< keyset flag: numerals */
#define KEYSET_SYMBOLS   0x0008 /**< keyset flag: symbols */
#define KEYSET_SPACE     0x0010 /**< keyset flag: space character */
#define KEYSET_HEX       0x0020 /**< keyset flag: hexadecimal */
#define KEYSET_FLOAT     0x0040 /**< keyset flag: floating-point */
#define KEYSET_NEGATIVE  0x0080 /**< keyset flag: negative number meta-flag */
#define KEYSET_HOSTNAME  0x0100 /**< keyset flag: hostname */
#define KEYSET_IPADDR    0x0200 /**< keyset flag: ip-address */

// global: key set complex bit-flags
#define KEYSET_ALPHA            KEYSET_UPPER | KEYSET_LOWER   /**< keyset flag: upper+lower letters */
#define KEYSET_ALPHA_NUMERIC    KEYSET_ALPHA | KEYSET_NUMERIC /**< keyset flag: upper+lower+numeric characters */
#define KEYSET_FULL             KEYSET_ALPHA_NUMERIC | KEYSET_SYMBOLS | KEYSET_SPACE /**< keyset flag: upper+lower+numeric+symbols+space characters */

// local: button input macros
#define ACTION_HOLD_CLICK_A (getInput(BUTTON_A) & HOLD)
#define ACTION_HOLD_CLICK_B (getInput(BUTTON_B) & HOLD)
#define ACTION_HOLD_CLICK_C (getInput(BUTTON_C) & HOLD)

// local: color formatting macros
#define COLOR_BRIGHTER(c)   (c | 0x00808080 )
#define COLOR_DARKER(c)     ((c >> 1) & 0x00FEFEFE)

// local: color formatting macros
#define _KBD_RESET_CURSOR() { this->cursor = strlen(this->charset) - 1; }


// ------------------------------------------------------------------------------------------------
//
//	Use the 'PocuterUtil' namespace
//
namespace PocuterUtil {
/**
* @brief PocuterUtil::Keyboard -- Pocuter Utility Class for Flexible Keyboard Input
*  
* @note See README.md file for details, examples, and usage guide
*/
// ------------------------------------------------------------------------------------------------
class Keyboard {	
	private:
		Pocuter *pocuter;

		char label[17];
		uint16_t keyset;

		char text[ KEYSET_STRING_MAX + 1 ];
		uint maxlen;

		char charset[ KEYSET_STRING_MAX + 3 ];
		int cursor;

		bool scrolling;

		unsigned long lastFrame;
		unsigned long interval;
		bool blinking;
		
	public:
		bool active;      /**< flag: keyboard 'in-use' */
		bool autoupdate;  /**< flag: auto-update display */

		Keyboard( Pocuter *pocuter, char *label, uint keyset, uint maxlen );
		
		void custom( char *charset );

		void clear();
		void set( char *newtext );
		char* get();
		
		bool getchar();
};
/**
 * @brief Keyboard Constructor
 * 
 * @param pocuter pointer to Pocuter system object
 * @param label keyboard label text
 * @param keyset character set bit flags
 * @param maxlen maximum length of keyboard text, must be <= 255
*/
Keyboard::Keyboard( Pocuter *pocuter, char *label, uint keyset = KEYSET_FULL, uint maxlen = 0 ) {
	
	// save reference to pocuter object
	this->pocuter = pocuter;

	// zero buffer memory
	memset( this->label,   0, 17 );
	memset( this->text,    0, KEYSET_STRING_MAX + 1);
	memset( this->charset, 0, KEYSET_STRING_MAX + 3);

	// copy label string text
	strncpy( this->label, label, 16 );
	
	// validate max text length
	if( maxlen > KEYSET_STRING_MAX ) maxlen = KEYSET_STRING_MAX;

	// init state variables
	this->active = false;
	this->maxlen = maxlen ? maxlen : KEYSET_STRING_MAX;
	this->cursor = 0;
	this->autoupdate = true;
	this->scrolling = false;
	this->lastFrame = micros();
	this->interval = 0;
	this->blinking = false;

	// save keyset code
	this->keyset = keyset;

	// custom keyset only assign end-control characters
	if( keyset == KEYSET_CUSTOM ) {
		strcpy( this->charset, CHARSET_NONE);
		return;
	}

	// assign special ip address charset (return)
	if( keyset & KEYSET_IPADDR ) {
		strcpy( this->charset, CHARSET_IPADDR CHARSET_NONE );
		_KBD_RESET_CURSOR();
		this->maxlen = 15;
		return;
	}

	// assign special hostname charset (return)
	if( keyset & KEYSET_HOSTNAME ) {
		strcpy( this->charset, CHARSET_HOSTNAME CHARSET_NONE );
		_KBD_RESET_CURSOR();
		return;
	}

	// prepend negative sign to numeric types (fall-through)
	if( keyset & KEYSET_NEGATIVE ) {
		strcpy( this->charset, "-" );
	}

	// assign special hexadecimal charset (return)
	if( keyset & KEYSET_HEX ) {
		strcat( this->charset, CHARSET_HEX CHARSET_NONE );
		_KBD_RESET_CURSOR();
		return;
	}

	// assign special float charset (return)
	if( keyset & KEYSET_FLOAT ) {
		strcat( this->charset, CHARSET_FLOAT CHARSET_NONE );
		_KBD_RESET_CURSOR();
		return;
	}

	// compose character set from keyset bit flags
	if( keyset & KEYSET_UPPER )
		strcat( this->charset, CHARSET_UPPER );

	if( keyset & KEYSET_LOWER )
		strcat( this->charset, CHARSET_LOWER );

	if( keyset & KEYSET_NUMERIC )
		strcat( this->charset, CHARSET_NUMERIC );

	if( keyset & KEYSET_SYMBOLS )
		strcat( this->charset, CHARSET_SYMBOLS );

	if( keyset & KEYSET_SPACE )
		strcat( this->charset, " " );


	// append end-control characters to charset
	strcat( this->charset, CHARSET_NONE);

	// set default cursor position to '[OK]' 
	_KBD_RESET_CURSOR();
}


/**
 * @brief set user-defined character set
 * 
 * @note resets keyboard cursor position
 * 
 * @param charset string containing allowed keyboard characters
 * 
*/
void Keyboard::custom( char *charset ) {
	this->keyset = KEYSET_CUSTOM;
	memset( this->charset, 0, KEYSET_STRING_MAX + 3 );
	strncpy( this->charset, charset, KEYSET_STRING_MAX );
	strcat( this->charset, CHARSET_NONE );
	_KBD_RESET_CURSOR();
}


/**
 * @brief clear keyboard text buffer
 * 
 * @note resets keyboard cursor position
*/
void Keyboard::clear() {
	this->set((char*)"");
	_KBD_RESET_CURSOR();
}


/**
 * @brief set keyboard text buffer
 * 
 * @note resets keyboard cursor position
*/
void Keyboard::set( char *newtext ) {
	memset( this->text, 0, KEYSET_STRING_MAX + 1 );
	strncpy( this->text, newtext, this->maxlen );
	_KBD_RESET_CURSOR();
}


/**
 * @brief get keyboard text buffer
 * 
 * @return pointer to keyboard text
*/
char* Keyboard::get() {
	return this->text;
}


/**
 * @brief display keyboard and handle user input
 * 
 * @note the calling application is responsible for updating button state by calling: updateInput()
 * @note by default the display will auto-update, set the autoupdate member to false to disable this behaviour
 * 
 * @return boolean flag indicating if text buffer has changed
*/
bool Keyboard::getchar() {

	// calculate blinking effect
	this->interval += (micros() - lastFrame) / 1000.0;
	if( this->interval > KEYSET_BLINK_LEN ) {
		this->blinking = !this->blinking;
		this->interval = 0;
	}
	this->lastFrame = micros();

	// set default text changed flag value
	char changed = false;

	// set keyboard active flag
	this->active = true;

	// get system color and make highlight color
	UG_COLOR systemColor = pocuterSettings.systemColor;
	UG_COLOR accentColor = COLOR_BRIGHTER( systemColor );
	UG_COLOR darkerColor = COLOR_DARKER( systemColor );

	// get display size
	uint16_t sizeX;
	uint16_t sizeY;
	pocuter->Display->getDisplaySize(sizeX, sizeY);

	// clear screen
	UGUI* gui = pocuter->ugui;	
	gui->UG_FillFrame(0, 0, sizeX, sizeY, C_BLACK);
	gui->UG_FontSelect(&FONT_POCUTER_5X7);

	// draw keyboard label
	gui->UG_SetForecolor( accentColor );	
	gui->UG_PutStringSingleLine(0, 0, this->label);

	// draw keyboard text
	gui->UG_SetForecolor( systemColor );	
	uint textlen = strlen( this->text );
	uint textpos = 0;
	if( textlen ) {
		// text fits within screen width
		if( textlen <= KEYSET_WIDTH_MAX ) {
			gui->UG_PutStringSingleLine(0, 32, this->text);
			textpos = gui->UG_StringWidth( this->text );
		} else {
			// truncate/scroll text to the left
			char textfrag[KEYSET_WIDTH_MAX + 1];
			uint start = textlen - KEYSET_WIDTH_MAX;
			strncpy(textfrag, this->text + start, KEYSET_WIDTH_MAX);
			textfrag[KEYSET_WIDTH_MAX] = '\0';
			gui->UG_PutStringSingleLine(0, 32, textfrag);
			textpos = gui->UG_StringWidth( textfrag );
		}
	}

	// current cursor key state
	int setlen = strlen( this->charset );
	char letter[5];	
	char curkey;

	// maxlen reached only allow DELETE and RETURN
	if( textlen == this->maxlen ) {
		setlen = 2;
		if( this->cursor % 2 ) {
			gui->UG_SetForecolor( systemColor );
			gui->UG_PutStringSingleLine( textpos + 2, 24, "[<]" );
			gui->UG_SetForecolor( accentColor );
			gui->UG_PutStringSingleLine( textpos + 2, 32, "[OK]" );
			curkey = '\n';
		} else {
			gui->UG_SetForecolor( accentColor );
			gui->UG_PutStringSingleLine( textpos + 2, 32, "[<]" );
			gui->UG_SetForecolor( systemColor );
			gui->UG_PutStringSingleLine( textpos + 2, 40, "[OK]" );
			curkey = '\r';
		}
	}

	// draw keyboard character set
	else {
		int index = this->cursor - 2;
		if( index < 0 ) index = setlen + index;
		for( int i=0; i < 5; i++ ) {
			
			// get keyset character
			char key = this->charset[ (index + i) % setlen ];

			// format keyboard 'action' characters 
			if( key == KBD_CHAR_RETURN ) { strcpy(letter,"[OK]"); }
			else if( key == KBD_CHAR_SPACE ) { strcpy(letter,"[ ]"); }
			else if( key == KBD_CHAR_DELETE ) { strcpy(letter,"[<]"); }
			// format 'special mode' keyboard characters
			else if( key == '-' && (this->keyset & (KEYSET_NEGATIVE | KEYSET_HOSTNAME)) ) { strcpy(letter,"[-]"); }
			else if( key == '.' && (this->keyset & (KEYSET_FLOAT | KEYSET_HOSTNAME | KEYSET_IPADDR))) { strcpy(letter,"[.]"); }
			// alpha-numeric key, no formatting
			else {
				letter[0] = key;
				letter[1] = '\0';
			}

			// store selected keyboard character 
			if( i == 2 ) curkey = key;

			// set highlight/blink color and draw character to screen
			gui->UG_SetForecolor( i == 2 ? (this->blinking ? accentColor : darkerColor) : systemColor );
			gui->UG_PutStringSingleLine( textpos + 2, 14+(i*9), letter );
		}
	}

	// cache: current scrolling hold state
	bool curScrolling = this->scrolling;

	// button: UP EVENT
	if( ACTION_SINGLE_CLICK_A || ACTION_HOLD_CLICK_A ) {
		this->scrolling = ACTION_HOLD_CLICK_A;
		if( !(!this->scrolling && curScrolling) )
			this->cursor = (setlen + (this->cursor-1)) % setlen;
	}

	// button: DOWN EVENT
	if( ACTION_SINGLE_CLICK_B || ACTION_HOLD_CLICK_B ) {
		this->scrolling = ACTION_HOLD_CLICK_B;
		if( !(!this->scrolling && curScrolling) )
			this->cursor = ++this->cursor % setlen;
	}

	// button: SELECT EVENT
	if( ACTION_SINGLE_CLICK_C || (ACTION_HOLD_CLICK_C && curkey == KBD_CHAR_DELETE )) {

		// key: RETURN EVENT
		 if( curkey == KBD_CHAR_RETURN ) { 			
			changed = true;

			// test: HOSTNAME doesn't end in '-' char
			if( (this->keyset & KEYSET_HOSTNAME) && textlen && this->text[textlen-1] == '-' ) {
				// -- remove trailing character --
				this->text[ --textlen ] = '\0';
			} 
			
			// else: end active keyboard state
			else {
				this->active = false;
			}
		}

		// key: DELETE EVENT
		else if( curkey == KBD_CHAR_DELETE ) { 
			if( textlen ) {
				this->scrolling = ACTION_HOLD_CLICK_C;
				if( !(!this->scrolling && curScrolling) ) {

					// adjust cursor postion if at max len (keep current key)
					if( textlen == this->maxlen )
						this->cursor = strlen( this->charset ) - 2;

					// -- remove trailing character --
					this->text[ --textlen ] = '\0';
					changed = true;
				}
			}
		}

		// key: APPEND EVENT
		else if( textlen < this->maxlen ){
			this->text[ textlen ] = curkey;
			this->text[ ++textlen ] = '\0';
			changed = true;

			// adjust cursor if max length (set to delete)
			if( textlen == this->maxlen )
				this->cursor = 0;

			// mode: numeric sub-type negative number handling (fall-through)
			if( this->keyset & KEYSET_NEGATIVE ) {
				// restrict usage to one '-' char at beginning
				if( curkey == '-' ) {
					if( textlen != 1 ) {
						// -- remove trailing character --
						this->text[ --textlen ] = '\0';
						changed = false;					
					}
				}
			}

			// mode: floating point number formatting
			if( this->keyset & KEYSET_FLOAT ) {

				// count decimal points
				uint octcount = 0;
				for(uint i=0; i < textlen; i++)
					if( this->text[i] == '.' )
						octcount++;
				
				// validate use of '.' char
				if( curkey == '.' ) {
					// restrict usage to one '.' char
					if( octcount > 1 ) {
						// -- remove trailing character --
						this->text[ --textlen ] = '\0';
						changed = false;					
					}

					// zero prefix decimal input
					if( textlen == 1 ) {
						this->text[0] = '0';
						this->text[1] = '.';
						this->text[2] = '\0';
						textlen = 2;
					}

					// zero prefix negative decimal input
					else if(textlen == 2 && this->text[0] == '-') {
						this->text[0] = '-';
						this->text[1] = '0';
						this->text[2] = '.';
						this->text[3] = '\0';
						textlen = 3;						
					}
				}
			}

			// mode: hostname/domain name formatting
			else if( this->keyset & KEYSET_HOSTNAME ) {
				// rule: restrict cant start with '-' or '.' character
				if( textlen == 1 && (curkey == '-' || curkey == '.') ) {
					// -- remove trailing character --
					this->text[ --textlen ] = '\0';
					changed = false;
				}
				// rule: restrict segment cant start with '-' character
				if( textlen >= 2 && curkey == '-' && this->text[textlen-2] == '.' ) {
					// -- remove trailing character --
					this->text[ --textlen ] = '\0';
					changed = false;
				}				
				// rule: no doubles '.' characters
				if( textlen >= 2 && curkey == '.' && this->text[textlen-2] == '.' ) {
					// -- remove trailing character --
					this->text[ --textlen ] = '\0';
					changed = false;					
				}
			}

			// mode: ip address mode auto-formatting
			else if( this->keyset & KEYSET_IPADDR ) {
				
				// test last three chars are digits
				bool triplet =  isdigit(this->text[textlen - 1]) && 
								isdigit(this->text[textlen - 2]) && 
								isdigit(this->text[textlen - 3]);

				// count octet markers
				uint octcount = 0;
				for(uint i=0; i < textlen; i++)
					if( this->text[i] == '.' )
						octcount++;

				// validate use of '.' char
				if( curkey == '.' ) {
					if( ( textlen == 1 )									// prevent leading '.' char
					|| ( textlen >= 2 && this->text[ textlen - 2 ] == '.' ) // prevent double '.' chars
					|| ( octcount > 3 )										// prevent more than four octets
					) {
						// -- remove trailing character --
						this->text[ --textlen ] = '\0';
						changed = false;
					}
				}
				
				// auto-detect new address octet
				else if( octcount < 3 ) {
					if( textlen >= 3 && triplet ) {
						this->text[ textlen ] = '.';
						this->text[ ++textlen ] = '\0';
					}
				}

				// validate last octet length <= 3
				else if( triplet && isdigit(this->text[textlen - 4]) ){
					// -- remove trailing character --
					this->text[ --textlen ] = '\0';
					changed = false;
				}
			}
		}
	}

	// update screen
	if( this->autoupdate )
		pocuter->Display->updateScreen();

	// return text changed flag
	return changed;
}

/*
	Close the 'PocuterUtil' namespace
*/
};

// undefine internal macros and constants
#undef KEYSET_WIDTH_MAX
#undef KEYSET_STRING_MAX
#undef KEYSET_BLINK_LEN

#undef KBD_CHAR_SPACE
#undef KBD_CHAR_DELETE
#undef KBD_CHAR_RETURN

#undef CHARSET_NONE

#undef ACTION_HOLD_CLICK_A
#undef ACTION_HOLD_CLICK_B
#undef ACTION_HOLD_CLICK_C

#undef COLOR_BRIGHTER
#undef COLOR_DARKER

#undef _KBD_RESET_CURSOR

#endif // _POCUTERUTIL_KEYBOARD_H_