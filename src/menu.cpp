#include "menu.h"
#include "buttons.h"

// Interaktives Menü: 4 Einträge (Feed, Play, Rest, Clean)
enum MenuItem {
  MENU_FEED = 0,
  MENU_PLAY = 1,
  MENU_REST = 2,
  MENU_CLEAN = 3
};

static MenuItem currentItem = MENU_FEED;
static MenuItem lastDrawnItem = (MenuItem)255; // Sentinel to force initial draw

static bool btnLeftPressed  = false;
static bool btnOkPressed    = false;
static bool btnRightPressed = false;

static void readButtons() {
  Buttons.poll();
  bool l = false, o = false, r = false;
  Buttons.getAndClearPressed(l, o, r);
  btnLeftPressed = l;
  btnOkPressed = o;
  btnRightPressed = r;
}

// Menülogik: Auswahl verschieben & Aktionen auslösen
void updateMenuLogic() {
  readButtons();
  
  // Block input during animations (EATING, POOPING, PLAYING, SLEEPING, etc.)
  if (isActionInProgress()) {
#ifdef DEBUG_BUTTONS
    static bool blockedMessageShown = false;
    if (!blockedMessageShown && (btnLeftPressed || btnRightPressed || btnOkPressed)) {
      Serial.println("[MENU] Input blocked - Animation in progress");
      blockedMessageShown = true;
    }
    if (!btnLeftPressed && !btnRightPressed && !btnOkPressed) {
      blockedMessageShown = false;
    }
#endif
    btnLeftPressed = false;
    btnRightPressed = false;
    btnOkPressed = false;
    return;
  }
  
  // Capture and clear button flags for one-shot behavior
  bool left = btnLeftPressed;
  bool right = btnRightPressed;
  bool ok = btnOkPressed;
  btnLeftPressed = false;
  btnRightPressed = false;
  btnOkPressed = false;

  if (left) {
#ifdef DEBUG_BUTTONS
    Serial.println("[BTN] LEFT pressed - Navigate menu");
#endif
    // Auswahl nach links
    if (currentItem == MENU_FEED) {
      currentItem = MENU_CLEAN;
    } else {
      currentItem = static_cast<MenuItem>(static_cast<int>(currentItem) - 1);
    }
  } else if (right) {
#ifdef DEBUG_BUTTONS
    Serial.println("[BTN] RIGHT pressed - Navigate menu");
#endif
    // Auswahl nach rechts
    if (currentItem == MENU_CLEAN) {
      currentItem = MENU_FEED;
    } else {
      currentItem = static_cast<MenuItem>(static_cast<int>(currentItem) + 1);
    }
  } else if (ok) {
#ifdef DEBUG_BUTTONS
    Serial.print("[BTN] OK pressed - Action: ");
#endif
    // Aktion bestätigen
    switch (currentItem) {
      case MENU_FEED: 
#ifdef DEBUG_BUTTONS
        Serial.println("FEED");
#endif
        pendingAction = ACTION_FEED; 
        break;
      case MENU_PLAY: 
#ifdef DEBUG_BUTTONS
        Serial.println("PLAY");
#endif
        pendingAction = ACTION_PLAY; 
        break;
      case MENU_REST: 
#ifdef DEBUG_BUTTONS
        Serial.println("REST");
#endif
        pendingAction = ACTION_REST; 
        break;
      case MENU_CLEAN: 
#ifdef DEBUG_BUTTONS
        Serial.println("CLEAN");
#endif
        pendingAction = ACTION_CLEAN; 
        break;
    }
  }
}

// Zeichnet Menü mit Hervorhebung des aktuell gewählten Eintrags
void drawBottomMenu() {
  // Skip redraw if selection hasn't changed
  if (currentItem == lastDrawnItem) return;
  
  int16_t y = TFT_HEIGHT - BOTTOM_BAR_H;

  tft.fillRect(0, y, TFT_WIDTH, BOTTOM_BAR_H, COLOR_STATUS_BG);

  tft.setTextColor(0xFFFF);
  tft.setTextSize(1);

  const char *labelFeed = "Futter";
  const char *labelPlay = "Spielen";
  const char *labelRest = "Schlafen";
  const char *labelClean = "Putzen";

  // Wir teilen die Leiste in 4 Bereiche
  int16_t sectionW = TFT_WIDTH / 4;

  // Feed-Box
  int16_t feedX = 0;
  int16_t feedW = sectionW;
  // Play-Box
  int16_t playX = sectionW;
  int16_t playW = sectionW;
  // Rest-Box
  int16_t restX = sectionW * 2;
  int16_t restW = sectionW;
  // Clean-Box
  int16_t cleanX = sectionW * 3;
  int16_t cleanW = TFT_WIDTH - cleanX;

  // Hervorhebung zeichnen
  auto drawItem = [&](MenuItem item, int16_t x, int16_t w, const char *label) {
    bool selected = (item == currentItem);
    uint16_t boxColor = selected ? 0xFFFF : COLOR_STATUS_BG;   // weiß wenn selektiert
    uint16_t textColor = selected ? COLOR_STATUS_BG : 0xFFFF; // invertierte Schrift

    // abgerahmter Kasten
    tft.drawRect(x + 1, y + 1, w - 2, BOTTOM_BAR_H - 2, 0xFFFF);
    if (selected) {
      tft.fillRect(x + 2, y + 2, w - 4, BOTTOM_BAR_H - 4, boxColor);
    }

    tft.setTextColor(textColor);
    int16_t tx = x + (w / 2) - (strlen(label) * 6) / 2; // grobe Zentrierung
    int16_t ty = y + (BOTTOM_BAR_H / 2) - 4;
    tft.setCursor(tx, ty);
    tft.print(label);
  };

  drawItem(MENU_FEED, feedX, feedW, labelFeed);
  drawItem(MENU_PLAY, playX, playW, labelPlay);
  drawItem(MENU_REST, restX, restW, labelRest);
  drawItem(MENU_CLEAN, cleanX, cleanW, labelClean);
  
  // Update cache
  lastDrawnItem = currentItem;
}
