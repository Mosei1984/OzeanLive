#include "start_menu.h"
#include "gfx.h"
#include "config.h"
#include "eeprom_store.h"
#include "buttons.h"

StartChoice runStartMenu(bool hasSaveAvailable) {
  uint8_t selected = 0;
  uint8_t lastDrawn = 255; // Force initial draw
  const uint8_t NUM_OPTIONS = 3;
  
  // Draw initial screen once
  tft.fillScreen(0x0010);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(3);
  tft.setCursor((TFT_WIDTH - 18 * 10) / 2, 20);
  tft.print("OzeanLive");
  
  tft.setTextSize(2);
  tft.setCursor((TFT_WIDTH - 11 * 12) / 2, 50);
  tft.print("Tamagotchi");
  
  while (true) {
    // Only redraw menu options if selection changed
    if (selected != lastDrawn) {
      const char* options[NUM_OPTIONS] = {"Start New", "Load", "Reset"};
      int16_t startY = 90;
      int16_t spacing = 25;
      
      // Clear menu area only
      tft.fillRect(70, startY - 5, TFT_WIDTH - 140, NUM_OPTIONS * spacing + 10, 0x0010);
      
      tft.setTextSize(2);
      for (uint8_t i = 0; i < NUM_OPTIONS; i++) {
        int16_t y = startY + i * spacing;
        
        if (i == 1 && !hasSaveAvailable) {
          tft.setTextColor(0x7BEF); // Grayed out
        } else if (i == selected) {
          tft.setTextColor(0xFFE0); // Highlighted
        } else {
          tft.setTextColor(0xFFFF); // Normal
        }
        
        tft.setCursor(80, y);
        if (i == selected) {
          tft.print("> ");
        } else {
          tft.print("  ");
        }
        tft.print(options[i]);
      }
      
      lastDrawn = selected;
    }
    
    delay(50); // Reduced from 100ms for better responsiveness
    
    Buttons.poll();
    bool btnLeftPressed = false;
    bool btnOkPressed = false;
    bool btnRightPressed = false;
    Buttons.getAndClearPressed(btnLeftPressed, btnOkPressed, btnRightPressed);
    
    if (btnLeftPressed) {
#ifdef DEBUG_BUTTONS
      Serial.println("[START] LEFT - Navigate menu");
#endif
      if (selected == 0) {
        selected = NUM_OPTIONS - 1;
      } else {
        selected--;
      }
    }
    
    if (btnRightPressed) {
#ifdef DEBUG_BUTTONS
      Serial.println("[START] RIGHT - Navigate menu");
#endif
      selected = (selected + 1) % NUM_OPTIONS;
    }
    
    if (btnOkPressed) {
      
      if (selected == 1 && !hasSaveAvailable) {
#ifdef DEBUG_BUTTONS
        Serial.println("[START] OK - Cannot load (no save available)");
#endif
        continue;
      }
      
      if (selected == 2) {
#ifdef DEBUG_BUTTONS
        Serial.println("[START] OK - Reset save");
#endif
        clearSave();
        
        tft.fillScreen(0x0000);
        tft.setTextColor(0xFFFF);
        tft.setTextSize(2);
        tft.setCursor((TFT_WIDTH - 13 * 12) / 2, TFT_HEIGHT / 2);
        tft.print("Save cleared!");
        delay(1000);
        
        // Redraw title screen after reset
        tft.fillScreen(0x0010);
        tft.setTextColor(0xFFFF);
        tft.setTextSize(3);
        tft.setCursor((TFT_WIDTH - 18 * 10) / 2, 20);
        tft.print("OzeanLive");
        tft.setTextSize(2);
        tft.setCursor((TFT_WIDTH - 11 * 12) / 2, 50);
        tft.print("Tamagotchi");
        
        hasSaveAvailable = false;
        selected = 0;
        lastDrawn = 255; // Force redraw
        continue;
      }
      
#ifdef DEBUG_BUTTONS
      Serial.print("[START] OK - Selected: ");
      Serial.println(selected == 0 ? "START NEW" : "LOAD");
#endif
      
      // Wait for button release before returning
      while (digitalRead(PIN_BTN_OK) == LOW)
        ;
      
      return (StartChoice)selected;
    }
  }
}
