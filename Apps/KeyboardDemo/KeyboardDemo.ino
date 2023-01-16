// Copyright 2023 Kallistisoft
// GNU GPL-3 https://www.gnu.org/licenses/gpl-3.0.txt

#include "settings.h"
#include "system.h"

#include "Keyboard.h"

long lastFrame;

// keyboard object pointers
PocuterUtil::Keyboard *keyboard_color;
PocuterUtil::Keyboard *keyboard_text;
PocuterUtil::Keyboard *keyboard_ip;

// initialize color swatch for color code 'smart' keyboard
UG_COLOR keyboard_color_swatch = C_BLACK;
char keyboard_color_string[7];

// buffers for last entered keyboard text
char display_text[32 + 2 + 1];

void setup() {
	pocuter = new Pocuter();
	pocuter->begin(PocuterDisplay::BUFFER_MODE_DOUBLE_BUFFER);
	pocuter->Display->continuousScreenUpdate(false);
	
	pocuterSettings.brightness = getSetting("GENERAL", "Brightness", 5);
	pocuter->Display->setBrightness(pocuterSettings.brightness);
	pocuterSettings.systemColor = getSetting("GENERAL", "SystemColor", C_BLUE);
	
	// enable or disable double click (disabling can achieve faster reaction to single clicks)
	disableDoubleClick(BUTTON_A);
	disableDoubleClick(BUTTON_B);
	disableDoubleClick(BUTTON_C);
	
	// setup your app here
	lastFrame = micros();
	
	// create a 'smart' keyboard for entering a six digit hex color code
	// disable autoupdate to allow for flicker free display post-processing
	// see the post-processing block for example of real-time keyboard input processing + formatting
	keyboard_color = new PocuterUtil::Keyboard( pocuter, (char*)"Enter color code", KEYSET_HEX, 6 );
	keyboard_color->autoupdate = false;
	
	// create a keyboard for entering full text with space char, limit length to 32 characters + set default text
	keyboard_text = new PocuterUtil::Keyboard( pocuter, (char*)"Enter text", KEYSET_FULL, 32 );
	keyboard_text->set( (char*)"Hello World!" );
	
	// create a keyboard for entering an ip address and bind it to a configuration setting
	keyboard_ip = new PocuterUtil::Keyboard( pocuter, (char*)"Enter ip address", KEYSET_IPADDR );
	keyboard_ip->bind( (char*)"keyboard", (char*)"ip-address", true );
	
	// initialize result display text
	strcpy( display_text, "> ");
	
	printf("Keyboard Demo Started...\n");
}

void loop() {

	/* *****************************************************************************
	 // 1) Loop initialization routines
	 // ***************************************************************************** */
	dt = (micros() - lastFrame) / 1000.0 / 1000.0;
	lastFrame = micros();
	updateInput();
	
	if( ACTION_BACK_TO_MENU ) {
		pocuter->OTA->setNextAppID(1);
		pocuter->OTA->restart();
	}

	// get display size
	uint16_t sizeX;
	uint16_t sizeY;
	pocuter->Display->getDisplaySize(sizeX, sizeY);
	
	// clear screen
	UGUI* gui = pocuter->ugui;	
	gui->UG_FillFrame(0, 0, sizeX, sizeY, C_BLACK);
	gui->UG_FontSelect(&FONT_POCUTER_5X7);
	gui->UG_SetForecolor( pocuterSettings.systemColor );
	
	
	/* *****************************************************************************
	// 2) User-Application background processing routines
	// ***************************************************************************** */
	// THIS SPACE LEFT INTENTIONALLY BLANK
	
	
	/* *****************************************************************************
	// 3) User-Application idle state code + input testing
	// ***************************************************************************** */
	// no active keyboards -- display usage text, last entry result, test button inputs
	if( !(keyboard_color->active || keyboard_text->active || keyboard_ip->active) ) {
		
		// display application label
		gui->UG_SetForecolor( pocuterSettings.systemColor | 0x00808080 );
		gui->UG_PutStringSingleLine(0, 0, "Keyboard Demo" );
		
		// display application usage text
		gui->UG_SetForecolor( pocuterSettings.systemColor );
		gui->UG_PutStringSingleLine(0, 16, "A: color keyboard" );
		gui->UG_PutStringSingleLine(0, 24, "B: text keyboard" );
		gui->UG_PutStringSingleLine(0, 32, "C: IP keyboard" );
		
		// display text of last keyboard entry
		gui->UG_PutStringSingleLine(0, 48, display_text );
		pocuter->Display->updateScreen();
		
		// select keyboard from button press
		if( ACTION_SINGLE_CLICK_A ) keyboard_color->active = true;
		if( ACTION_SINGLE_CLICK_B ) keyboard_text->active = true;
		if( ACTION_SINGLE_CLICK_C ) keyboard_ip->active = true;
		
		// force button state reset by restarting loop()
		return;
	}

	
	/* *****************************************************************************
	// 4) Active keyboard handling next because the keyboard is a modal interface
	// ***************************************************************************** */
	// text keyboard is active -- test for completed condition and copy text to buffer
	if( keyboard_text->active ) {
		if( keyboard_text->getchar() ) {
			if( !keyboard_text->active ) {
				strcpy( &display_text[2], keyboard_text->get() );
			}
		}
		return;
	}
	
	// ip address keyboard is active -- test for completed condition, copy text to buffer, and save data
	if( keyboard_ip->active ) {
		if( keyboard_ip->getchar() ) {
			if( !keyboard_ip->active ) {
				strcpy( &display_text[2], keyboard_text->get() );
				keyboard_ip->save();
			}
		}
		return;
	}
	
	// color keyboard is active -- perform real time 'smart' post-processing
	if( keyboard_color->active ) {
		
		// display keyboard + process input
		bool changed = keyboard_color->getchar();
		
		// display color swatch overlay + update screen
		gui->UG_FillFrame(sizeX - 16, (sizeY/2)-7, sizeX, (sizeY/2)+8, keyboard_color_swatch);
		pocuter->Display->updateScreen();
		
		if( changed ) {
			// input complete -- copy string buffers
			if( !keyboard_color->active ) {
				
				// write formated color string to keyboard buffer
				keyboard_color->set( keyboard_color_string );
				
				// copy keyboard text to display buffer
				strcpy( &display_text[2], keyboard_color->get() );
			}
			
			// input changed -- update color swatch variable
			else {
				// get current keyboard text
				char *text = keyboard_color->get();
				
				// format keyboard text to color string (zero-fill, left-justify)
				memset( keyboard_color_string, '0', 6);
				memcpy( keyboard_color_string, text, strlen(text) );
				keyboard_color_string[6] = '\0';
				
				// convert hex color string to numeric
				keyboard_color_swatch = (UG_COLOR)strtol( keyboard_color_string, 0, 16 );
			}
		}
		return;
	}
	
	
	/* *****************************************************************************
	// 5) User-Application modal display and interfaces 
	// ***************************************************************************** */
	// THIS SPACE LEFT INTENTIONALLY BLANK
}
