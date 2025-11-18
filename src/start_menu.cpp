#include "start_menu.h"
#include "gfx.h"
#include "config.h"
#include "eeprom_store.h"

StartChoice runStartMenu(bool hasSaveAvailable) {
  uint8_t selected = 0;
  const uint8_t NUM_OPTIONS = 3;
  
  bool btnLeftPressed = false;
  bool btnRightPressed = false;
  bool btnOkPressed = false;
  
  while (true) {
    tft.fillScreen(0x0010);
    
    tft.setTextColor(0xFFFF);
    tft.setTextSize(3);
    tft.setCursor((TFT_WIDTH - 18 * 10) / 2, 20);
    tft.print("OzeanLive");
    
    tft.setTextSize(2);
    tft.setCursor((TFT_WIDTH - 11 * 12) / 2, 50);
    tft.print("Tamagotchi");
    
    const char* options[NUM_OPTIONS] = {"Start New", "Load", "Reset"};
    int16_t startY = 90;
    int16_t spacing = 25;
    
    tft.setTextSize(2);
    for (uint8_t i = 0; i < NUM_OPTIONS; i++) {
      int16_t y = startY + i * spacing;
      
      if (i == 1 && !hasSaveAvailable) {
        tft.setTextColor(0x7BEF);
      } else if (i == selected) {
        tft.setTextColor(0xFFE0);
      } else {
        tft.setTextColor(0xFFFF);
      }
      
      tft.setCursor(80, y);
      if (i == selected) {
        tft.print("> ");
      } else {
        tft.print("  ");
      }
      tft.print(options[i]);
    }
    
    delay(100);
    
    int sLeft = digitalRead(PIN_BTN_LEFT);
    int sRight = digitalRead(PIN_BTN_RIGHT);
    int sOk = digitalRead(PIN_BTN_OK);
    
    if (sLeft == LOW && !btnLeftPressed) {
      btnLeftPressed = true;
      if (selected == 0) {
        selected = NUM_OPTIONS - 1;
      } else {
        selected--;
      }
    } else if (sLeft == HIGH) {
      btnLeftPressed = false;
    }
    
    if (sRight == LOW && !btnRightPressed) {
      btnRightPressed = true;
      selected = (selected + 1) % NUM_OPTIONS;
    } else if (sRight == HIGH) {
      btnRightPressed = false;
    }
    
    if (sOk == LOW && !btnOkPressed) {
      btnOkPressed = true;
      
      if (selected == 1 && !hasSaveAvailable) {
        continue;
      }
      
      if (selected == 2) {
        clearSave();
        
        tft.fillScreen(0x0000);
        tft.setTextColor(0xFFFF);
        tft.setTextSize(2);
        tft.setCursor((TFT_WIDTH - 13 * 12) / 2, TFT_HEIGHT / 2);
        tft.print("Save cleared!");
        delay(1000);
        
        hasSaveAvailable = false;
        selected = 0;
        btnOkPressed = false;
        continue;
      }
      
      return (StartChoice)selected;
    } else if (sOk == HIGH) {
      btnOkPressed = false;
    }
  }
}
