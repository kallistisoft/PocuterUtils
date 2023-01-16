#include "settings.h"
#include "system.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>

#include "md5.h"
MD5 md5sum;

long lastFrame;

#define DEBUG_TEMPFILE_ONLY 1

#define CENTER_TEXT(y,text) sizeX/2 - gui->UG_StringWidth(text)/2, y, text
#define NEXTLINE(text) gui->UG_PutStringSingleLine(0, 18+text_y, text); text_y += 12;

#define WWW_ERRORMSG(...) \
	printf( "[%s] ", timestamp ); \
	sprintf( www_error_msg, __VA_ARGS__ ); \
	printf( "%s\n", www_error_msg ); \
	delay(10); \
	return;

#define LOGMSG( _format_, ... ) \
	printf("[%s] " _format_ "\n", timestamp, __VA_ARGS__ ); delay(10);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Import index.html file contents as string literal
const char index_html[] PROGMEM = 
#include "index_html.h" 
;

// char* GetCurrentTimeString() :: format current date time string
char time_str[256];
char* GetCurrentTimeString() {
	tm time;
	pocuter->PocTime->getLocalTime( &time );
    snprintf(
		time_str, 256, 
		"%04d-%02d-%02d %02d:%02d:%02d", 
		time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,	
		time.tm_hour, time.tm_min, time.tm_sec
	);
	return &time_str[0];
}

// void DEBUG_HTTP_REQUEST( *request ) :: print request debug info to serial
void DEBUG_HTTP_REQUEST( AsyncWebServerRequest *request ) {
	char *timestamp = GetCurrentTimeString();

	// debug: print request method and URL
	printf("\n");
	LOGMSG( "%s %s", request->method() == HTTP_GET ? "GET" : "POST", request->url().c_str() );

	// debug: print request parameter values
	int params = request->params();
	for(int i=0; i < params; i++){
		AsyncWebParameter* p = request->getParam(i);
		if(p->isFile()){ //p->isPost() is also true
			LOGMSG("FILE[ %s ]: '%s', %u bytes", p->name().c_str(), p->value().c_str(), p->size());
		} else if(p->isPost()){
			LOGMSG("POST[ %s ]: %s", p->name().c_str(), p->value().c_str());
		} else {
			LOGMSG("GET[ %s ]: %s", p->name().c_str(), p->value().c_str());
		}
	}

}

// global variables for response state tracking
char  www_error_msg   [256] = "";
char  www_path_image  [256] = "";
char  www_path_backup [256] = "";
char  www_path_temp   [256] = "";

char  www_image_hash  [33]  = "";
FILE* www_image_file = 0;
long  www_image_size = 0;


void setup() {
	pocuter = new Pocuter();
	pocuter->begin(PocuterDisplay::BUFFER_MODE_DOUBLE_BUFFER);
	pocuter->Display->continuousScreenUpdate(false);
	
	pocuterSettings.brightness = getSetting("GENERAL", "Brightness", 5);
	pocuter->Display->setBrightness(pocuterSettings.brightness);
	pocuterSettings.systemColor = getSetting("GENERAL", "SystemColor", C_LIME);
	
	// enable or disable double click (disabling can achieve faster reaction to single clicks)
	disableDoubleClick(BUTTON_A);
	disableDoubleClick(BUTTON_B);
	disableDoubleClick(BUTTON_C);
	
	// setup your app here
	lastFrame = micros();

	printf("\n\nStarting App Uploader...\n");

	// route: GET /
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	printf("* Creating route for GET /...\n");	
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
		DEBUG_HTTP_REQUEST( request );
		request->send_P(200, "text/html", index_html );
	});

	// route: POST /update [appID] [appImage]
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	printf("* Creating route for POST /update...\n");
	server.on("/update", HTTP_POST, 

	// POST: Verify uploaded file and launch application
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	[] (AsyncWebServerRequest *request) {
		DEBUG_HTTP_REQUEST( request );
		char *timestamp = GetCurrentTimeString();

		// error: upload handler generated an error message
		if( strlen(www_error_msg) ) {
			LOGMSG("%s", www_error_msg );
			request->send(200, "text/plain", www_error_msg );
			return;
		} 

		// verify: request has all parameters and proper types (string,file)
		if( !(
			request->hasParam("appID",true) && 
			request->hasParam("appMD5",true) && 
			request->hasParam("appSize",true) && 
			request->hasParam("appImage",true,true)) 
		) {
			WWW_ERRORMSG("Error: Missing or incorrect request parameters!");
			if( www_image_file ) remove( www_path_temp );
			request->send(200, "text/plain", www_error_msg );
			return;
		}

		// get request parameters
		AsyncWebParameter* paramID = request->getParam("appID",true);
		AsyncWebParameter* paramMD5 = request->getParam("appMD5",true);
		AsyncWebParameter* paramSize = request->getParam("appSize",true);
		AsyncWebParameter* paramImage = request->getParam("appImage",true,true);
		long appID = atol( paramID->value().c_str() );
		const char *appMD5 = paramMD5->value().c_str();
		long appSize = atol( paramSize->value().c_str() );
		long image_size = paramImage->size();

		// verify: uploaded file is same size as declared size
		if( www_image_size != appSize ) {
			WWW_ERRORMSG("Error: Uploaded file size doesn't match declared file size: %u -> %u", www_image_size, appSize );
			if( www_image_file ) remove( www_path_temp );
			request->send(200, "text/plain", www_error_msg );
			return;
		}

		// verify: appImage has a valid size
		if( www_image_size < 600*1024 ) {
			WWW_ERRORMSG("Error: Invalid size for upload file (%u) -- must be larger than 600KiB!", www_image_size );
			if( www_image_file ) remove( www_path_temp );
			request->send(200, "text/plain", www_error_msg );
			return;
		}

		// verify: MD5 hash of uploaded file matches declared MD5 hash
		if( strcmp( www_image_hash, appMD5 ) != 0 ) {
			WWW_ERRORMSG("Error: Uploaded MD5 hash doesn't equal declared file hash: %s -> %s", www_image_hash, appMD5 );
			if( www_image_file ) remove( www_path_temp );
			request->send(200, "text/plain", www_error_msg );
			return;			
		}

		// debug: upload debug mode -- skip writing file unless self-hoisting
		if( DEBUG_TEMPFILE_ONLY && appID != 8080 ){
			remove( www_path_image );			
			LOGMSG("DEBUG: skipping installation of uploaded image file...", 0 );
			request->send(200, "text/plain", "DEBUG: skipping installation of temporary image..." );
			return;
		}

		// remove: existing application backup file
		LOGMSG(" DEL: %s", www_path_backup );
		remove( www_path_backup );

		// rename: existing application file
		LOGMSG("MOVE: %s -> %s", www_path_image, www_path_backup );
		rename( www_path_image, www_path_backup );

		// rename: temporary file
		LOGMSG("MOVE: %s -> %s", www_path_temp, www_path_image );
		rename( www_path_temp, www_path_image );

		// test: are we self-hoisting the 'Code Uploader' application?
		if( appID == 8080 ) {
			printf(" ----\n");
			LOGMSG("HOIST: Changing target application to 'Self-Hoisting Boot Proxy' - #8081", 0 );
			appID = 8081;
		}

		// send: request response
		printf(" ----\n");
		LOGMSG(" RUN: %u\n", appID );
		request->send(200, "text/plain", "OK: Launching application...");
		delay(100);

		// launch: application
		pocuter->OTA->setNextAppID( appID );
		pocuter->OTA->restart();
	},

	// UPLOAD: File upload request handler -- save streamed file...
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t size, bool final) {
		const char *image_name = filename.c_str();
		char *timestamp = GetCurrentTimeString();
		char *numtest;

		// start: verify parameters, mkdir, fopen
		if( index == 0 ) {
			printf("\n\n");
			DEBUG_HTTP_REQUEST( request );

			// reset: error message + file pointer\			
			www_error_msg[0] =
			www_path_temp[0] =
			www_path_image[0] =
			www_path_backup[0] = '\0';
			www_image_hash[0] = '\0';
			www_image_file = NULL;			
			www_image_size = 0;
			md5sum.reset();

			// verify: request has valid appID parameter
			if( !(request->hasParam("appID",true) ) ) {
				WWW_ERRORMSG("Error: Missing appID parameter!");
			}

			// verify: appID is numeric and >= 2
			AsyncWebParameter* paramID = request->getParam("appID",true);
			long appID = strtol( paramID->value().c_str(), &numtest, 10);
			if( *numtest || appID < 2 ) {
				WWW_ERRORMSG("Error: appID isn't a number >= 2!");
			}

			// verify: appImage filename
			if( strcmp( image_name, "esp32c3.app" ) != 0 ) {
				WWW_ERRORMSG( "Error: Invalid name for upload image file!: '%s'", image_name );
			}

			// debug: begin writing file to sd card
			LOGMSG( "IMAGE: %S", image_name );

			// mkdir: app folder
			char dirpath[256];
			snprintf( dirpath, 255, "%s/apps/%u", pocuter->SDCard->getMountPoint(), appID );
			LOGMSG( " PATH: %s", dirpath );
			if( !access( dirpath, F_OK) == 0 ) {
				if( !mkdir( dirpath, S_IRWXU ) == 0 ) {
					WWW_ERRORMSG( "Error: creating application folder '%s': errno: %u", dirpath, errno );
				}
			}

			// calc: temporary, backup, and image file names
			snprintf( www_path_image,  255, "%s/esp32c3.app",        dirpath );			
			snprintf( www_path_backup, 255, "%s/esp32c3.app.backup", dirpath );
			snprintf( www_path_temp,   255, "%s/esp32c3.app.upload", dirpath );			

			// open: image file handle
			LOGMSG( "WRITE: %s", www_path_temp );
			www_image_file = fopen( www_path_temp, "w" );
			if( !www_image_file ) {
				WWW_ERRORMSG( "Error: opening image file for writting '%s': %u", www_path_temp, errno );
			}
		}

		// error: ignore data
		if( strlen(www_error_msg) ) return;

		// write: image data stream
		if( size != 0 && www_image_file ) {
			LOGMSG( " DATA: %u bytes", size );
			long bytes = fwrite( data, 1, size, www_image_file );
			if( bytes != size ) {
				WWW_ERRORMSG( "Error: writting file '%s' - block size mismatch: %u -> %u", www_path_temp, size, bytes );
				fclose( www_image_file );
			}
			www_image_size += size;
			md5sum.add( data, size );
		}

		// stop: close open file
		if( final && www_image_file ) {
			LOGMSG(" DONE: %u bytes", www_image_size );
			fclose( www_image_file ); 			

 			strncpy( www_image_hash, md5sum.getHash().c_str(), 32 );
 			LOGMSG(" HASH: %s", www_image_hash );
 
 			printf(" ----\n");
		}

	});

  	// Start server
	printf("* Starting Web Server...\n\n");
  	server.begin();
}


void loop() {
	uint text_y = 0;	
	pocuter->Sleep->disable();

	dt = (micros() - lastFrame) / 1000.0 / 1000.0;
	lastFrame = micros();
	updateInput();
	
	if (ACTION_BACK_TO_MENU) {
		pocuter->OTA->setNextAppID(1);
		pocuter->OTA->restart();
	}
	
	// update your app here
	// dt contains the amount of time that has passed since the last update, in seconds
	UGUI* gui = pocuter->ugui;
	uint16_t sizeX;
	uint16_t sizeY;
	pocuter->Display->getDisplaySize(sizeX, sizeY);
	
	gui->UG_FillFrame(0, 0, sizeX, sizeY, C_BLACK);
	gui->UG_FillFrame(0, 0, sizeX, 13, C_BLUE);
	gui->UG_FontSelect(&FONT_POCUTER_5X7);
	gui->UG_SetForecolor( C_YELLOW );
    gui->UG_PutStringSingleLine( CENTER_TEXT( 0, "Code Uploader" ) );
	gui->UG_SetForecolor( C_WHITE );

	if( !pocuter->SDCard->cardIsMounted() ) {
		gui->UG_SetForecolor( C_YELLOW );
		NEXTLINE( "SD card is not" );
		NEXTLINE( "mounted.");
		text_y += 2;
		NEXTLINE( "Please restart" );
		NEXTLINE( "the application!" );
		return;
	}

	PocuterWIFI::wifiCredentials cred;
	pocuter->WIFI->getCredentials( &cred );	

	if( pocuter->WIFI->getState() == PocuterWIFI::WIFI_STATE_CONNECTED ) {
		char addr[64];
		const PocuterWIFI::ipInfo* info = pocuter->WIFI->getIpInfo();
		if( info->ipV4 ) {
			snprintf(addr, 64, "%s", inet_ntoa(info->ipV4));
			NEXTLINE( "Server address:" );
			gui->UG_SetForecolor( C_PLUM );
			text_y += 2;
			NEXTLINE( addr);
		} else {
			NEXTLINE( "Obtaining DHCP" );
			NEXTLINE( "address from:" );
			gui->UG_SetForecolor( C_PLUM );			
			text_y += 2;
			NEXTLINE( (char*)cred.ssid );
		}
	}
	else {		
		if( cred.ssid ) {
			NEXTLINE( "Trying to connect" );
			NEXTLINE( "to WiFi network:" );
			gui->UG_SetForecolor( C_PLUM);
			text_y += 2;
			NEXTLINE( (char*)cred.ssid );
		}
		else {
			NEXTLINE( "Unable to connect" );
			NEXTLINE( "to WiFi network:" );
		}
		pocuter->Display->updateScreen();
		return;
	}
	pocuter->Display->updateScreen();
}