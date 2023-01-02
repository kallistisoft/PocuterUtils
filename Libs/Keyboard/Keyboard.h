#ifndef KEYBOARD_H
#define KEYBOARD_H

/*
	FIDDLE: play with color settings
	FIDDLE: play with font size??
	- - - - - - - - - -
	FIXED: Text length overflow bug
	FIXED: Label length overflow bug
	- - - - - - - - - -
	DONE: Fast delete	
	DONE: Fast scroll up/down
	DONE: IP Address auto-formatting
	DONE: Max length available keyset change	
	DONE: Refactor CHARSET_ to CHARSET to improve autofill	
	DONE: Implement blinking char selection
	- - - - - - - - - -
	TODO: cancel button (maybe??)
	TODO: left/right edit keys (maybe??)
	^^^^^^^^
	creates a bunch of edge cases :/
*/


#include <Pocuter.h>
#include <cstring>

#define KEYSET_WIDTH_MAX	12
#define KEYSET_STRING_MAX	255
#define KEYSET_BLINK_LEN	250

#define CHARSET_SPACE	'\t'
#define CHARSET_DELETE	'\r'
#define CHARSET_ENTER	'\n'

#define CHARSET_NONE	"\r\n"
#define CHARSET_UPPER	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define CHARSET_LOWER	"abcdefghijklmnopqrstuvwxyz"
#define CHARSET_NUMERIC	"0123456789"
#define CHARSET_SYMBOLS	",./\\;:[]!@#$%^&*()_+<>?`'\"{}|~-="
#define CHARSET_IPADDR	"." CHARSET_NUMERIC
#define CHARSET_HEX		CHARSET_NUMERIC "ABCDEF"

#define KEYSET_CUSTOM	0x00
#define KEYSET_UPPER	0x01
#define KEYSET_LOWER	0x02
#define KEYSET_NUMERIC	0x04
#define KEYSET_SYMBOLS	0x08
#define KEYSET_SPACE	0x10
#define KEYSET_HEX		0x20
#define KEYSET_IPADDR	0x80

#define KEYSET_ALPHA			KEYSET_UPPER | KEYSET_LOWER
#define KEYSET_ALPHA_NUMERIC	KEYSET_ALPHA | KEYSET_NUMERIC
#define KEYSET_FULL				KEYSET_ALPHA_NUMERIC | KEYSET_SYMBOLS | KEYSET_SPACE

#define ACTION_HOLD_CLICK_A (getInput(BUTTON_A) & HOLD)
#define ACTION_HOLD_CLICK_B (getInput(BUTTON_B) & HOLD)
#define ACTION_HOLD_CLICK_C (getInput(BUTTON_C) & HOLD)

#define COLOR_BRIGHTER(c)	(c | 0x00808080 )
#define COLOR_DARKER(c)		((c >> 1) & 0x00FEFEFE)

/*
	Use the 'Pocuter' namespace
*/
namespace Pocuter {

/* ------------------------------------------------------------------------------------------------
	Pocuter::Keyboard() -- Flexible keyboard utility object class
   ------------------------------------------------------------------------------------------------ */
class Keyboard {
	
	private:
		Pocuter *pocuter;

		char label[17];
		uint keyset;

		char text[ KEYSET_STRING_MAX + 1 ];
		uint maxlen;

		char charset[ KEYSET_STRING_MAX + 3 ];
		int cursor;

		bool scrolling;

		unsigned long lastFrame;
		unsigned long interval;
		bool blinking;
		
	public:
		bool active;

		Keyboard( Pocuter *pocuter, char *label, uint keyset, uint maxlen );
		
		void custom( char *charset );

		void clear();
		void set( char *newtext );
		char* get();
		
		bool getchar();
};

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

	// assign special ip address charset
	if( keyset & KEYSET_IPADDR ) {
		strcpy( this->charset, CHARSET_IPADDR CHARSET_NONE );
		this->maxlen = 15;
		return;
	} 

	// assign special hexadecimal charset
	if( keyset & KEYSET_HEX ) {
		strcpy( this->charset, CHARSET_HEX CHARSET_NONE );
		return;
	}

	// build composable character set from keyset bit flags
	if( keyset & KEYSET_UPPER )
		strcat( this->charset, CHARSET_UPPER );

	if( keyset & KEYSET_LOWER )
		strcat( this->charset, CHARSET_LOWER );

	if( keyset & KEYSET_NUMERIC )
		strcat( this->charset, CHARSET_NUMERIC );

	if( keyset & KEYSET_SYMBOLS )
		strcat( this->charset, CHARSET_SYMBOLS );

	if( keyset & KEYSET_SPACE )
		strcat( this->charset, "\t" );


	// append end-control characters to charset
	strcat( this->charset, CHARSET_NONE);
}

void Keyboard::custom( char *charset ) {
	this->keyset = KEYSET_CUSTOM;
	memset( this->charset, 0, KEYSET_STRING_MAX + 3 );
	strncpy( this->charset, charset, KEYSET_STRING_MAX );
	strcat( this->charset, CHARSET_NONE );
	this->cursor = 0;
}

void Keyboard::clear() {
	this->set((char*)"");
}

void Keyboard::set( char *newtext ) {
	memset( this->text, 0, KEYSET_STRING_MAX + 1 );
	strncpy( this->text, newtext, this->maxlen );
	this->cursor = 0;
}

char* Keyboard::get() {
	return this->text;
}

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
		if( textlen <= KEYSET_WIDTH_MAX ) {
			gui->UG_PutStringSingleLine(0, 32, this->text);
			textpos = gui->UG_StringWidth( this->text );
		} else {
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

	// maxlen reached only allow delete and enter
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
			
			char key = this->charset[ (index + i) % setlen ];

			if( key == CHARSET_ENTER ) { strcpy(letter,"[OK]"); }
			else if( key == CHARSET_SPACE ) { strcpy(letter,"[ ]"); }
			else if( key == CHARSET_DELETE ) { strcpy(letter,"[<]"); }
			else if( key == '.' && this->keyset == KEYSET_IPADDR ) { strcpy(letter,"[.]"); }
			else {
				letter[0] = key;
				letter[1] = '\0';
			}

			if( i == 2 ) curkey = key;

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
	if( ACTION_SINGLE_CLICK_C || (ACTION_HOLD_CLICK_C && curkey == CHARSET_DELETE )) {
		// key: SPACE
		if( curkey == CHARSET_SPACE ) { 
			strcat(this->text," "); 
			changed = true;
		}
		// key: DELETE
		else if( curkey == CHARSET_DELETE ) { 
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
		// key: ENTER
		else if( curkey == CHARSET_ENTER ) { 
			this->active = false;
			changed = true;
		}
		// key: APPEND
		else if( textlen < this->maxlen ){
			this->text[ textlen ] = curkey;
			this->text[ ++textlen ] = '\0';
			changed = true;

			// adjust cursor if max length (set to delete)
			if( textlen == this->maxlen )
				this->cursor = 0;

			// ip address mode auto-formatting
			if( this->keyset == KEYSET_IPADDR ) {
				
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
					if( ( textlen == 1 )									 // prevent leading '.' char
					 || ( textlen >= 2 && this->text[ textlen - 2 ] == '.' ) // prevent double '.' chars					
					 || ( octcount > 3 )									 // prevent more than four octets
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

				// vaildate last octet length <= 3
				else if( triplet && isdigit(this->text[textlen - 4]) ){
					// -- remove trailing character --
					this->text[ --textlen ] = '\0';
					changed = false;
				}
			}
		}
	}

	// update screen
    pocuter->Display->updateScreen();

	// return text changed flag
	return changed;
}

/*
	Close the 'Pocuter' namespace
*/
};

#endif
