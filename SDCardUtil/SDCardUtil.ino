#include <Pocuter.h>
#include "settings.h"
#include "system.h"

long lastFrame;

void setup() {
    pocuter = new Pocuter();
    pocuter->begin(PocuterDisplay::BUFFER_MODE_DOUBLE_BUFFER);
    pocuter->Display->continuousScreenUpdate(false);
    
    pocuterSettings.brightness = getSetting("GENERAL", "Brightness", 5);
    pocuter->Display->setBrightness(pocuterSettings.brightness);
    pocuterSettings.systemColor = getSetting("GENERAL", "SystemColor", C_LIME);

    // enable or disable double click (disabling can achieve faster reaction to single clicks)
    enableDoubleClick(BUTTON_A);
    enableDoubleClick(BUTTON_B);
    enableDoubleClick(BUTTON_C);

    // setup your app here
    lastFrame = micros();
}

void loop() {
    dt = (micros() - lastFrame) / 1000.0 / 1000.0;
    lastFrame = micros();
    updateInput();

    if (ACTION_BACK_TO_MENU) {
        pocuter->OTA->setNextAppID(1);
        pocuter->OTA->restart();
    }

    // initialize drawing area
    UGUI* gui = pocuter->ugui;
    uint16_t sizeX;
    uint16_t sizeY;
    pocuter->Display->getDisplaySize(sizeX, sizeY);

    gui->UG_FillFrame(0, 0, sizeX, sizeY, C_BLACK);
    gui->UG_FillFrame(0, 0, sizeX, 11, C_RED);
    gui->UG_FontSelect(&FONT_POCUTER_5X7);

    // get sdcard mount state
    bool is_mounted = pocuter->SDCard->cardIsMounted();
    bool is_card = pocuter->SDCard->cardInSlot();

    // display sd card mount state
    if( is_mounted ) {
        gui->UG_SetForecolor( C_YELLOW );
        gui->UG_PutStringSingleLine(0, 8*0, "CARD IS MOUNTED");

        gui->UG_SetForecolor( C_PLUM);
        gui->UG_PutStringSingleLine(0, 8*2, "Unmount card by");
        gui->UG_PutStringSingleLine(0, 8*3, "double clicking");
        gui->UG_PutStringSingleLine(0, 8*4, "any button");
    } else {
        if( is_card ) {
            // umounted card available for mounting
            gui->UG_SetForecolor( C_YELLOW );
            gui->UG_PutStringSingleLine(0, 8*0, "Unmounted Card");

            gui->UG_SetForecolor( C_PLUM);
            gui->UG_PutStringSingleLine(0, 8*2, "To MOUNT card");
            gui->UG_PutStringSingleLine(0, 8*3, "double click any");
            gui->UG_PutStringSingleLine(0, 8*4, "button");
        } else {
            // no card in slot
            gui->UG_SetForecolor( C_YELLOW );
            gui->UG_PutStringSingleLine(0, 8*0, "Please insert card");
        }
    }

    // mount or unmount card as a response to button double click
    if( ACTION_DOUBLE_CLICK_A || ACTION_DOUBLE_CLICK_B || ACTION_DOUBLE_CLICK_C ) {        
        const char* mount_point = pocuter->SDCard->getMountPoint();
        if( is_mounted ) {
            printf("--> unmount card!\n");
            pocuter->SDCard->unmount();
        } else {
            printf("<-- mount card!\n");
            pocuter->SDCard->mount();
        }
    }

    // update display
    pocuter->Display->updateScreen();
}
