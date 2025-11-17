#include "menu.h"

// Interaktives Menü: 4 Einträge (Feed, Play, Rest, Clean)
enum MenuItem {
  MENU_FEED = 0,
  MENU_PLAY = 1,
  MENU_REST = 2,
  MENU_CLEAN = 3
};

static MenuItem currentItem = MENU_FEED;
static MenuItem lastDrawnItem = MENU_FEED;

// Button-Zustände (Entprellung)
static int lastBtnLeftState  = HIGH;
static int lastBtnOkState    = HIGH;
static int lastBtnRightState = HIGH;

static bool btnLeftPressed  = false;
static bool btnOkPressed    = false;
static bool btnRightPressed = false;

static unsigned long lastBtnReadMs = 0;
constexpr unsigned long DEBOUNCE_MS = 40;

// interne Helfer: Buttons lesen
static void readButtons() {
  unsigned long now = millis();
  if (now - lastBtnReadMs < DEBOUNCE_MS) return;
  lastBtnReadMs = now;

  int sLeft  = digitalRead(PIN_BTN_LEFT);
  int sOk    = digitalRead(PIN_BTN_OK);
  int sRight = digitalRead(PIN_BTN_RIGHT);

  btnLeftPressed  = (lastBtnLeftState  == HIGH && sLeft  == LOW);
  btnOkPressed    = (lastBtnOkState    == HIGH && sOk    == LOW);
  btnRightPressed = (lastBtnRightState == HIGH && sRight == LOW);

  lastBtnLeftState  = sLeft;
  lastBtnOkState    = sOk;
  lastBtnRightState = sRight;
}

// Menülogik: Auswahl verschieben & Aktionen auslösen
void updateMenuLogic() {
  readButtons();
  
  // Capture and clear button flags for one-shot behavior
  bool left = btnLeftPressed;
  bool right = btnRightPressed;
  bool ok = btnOkPressed;
  btnLeftPressed = false;
  btnRightPressed = false;
  btnOkPressed = false;

  if (left) {
    // Auswahl nach links
    if (currentItem == MENU_FEED) {
      currentItem = MENU_CLEAN;
    } else {
      currentItem = static_cast<MenuItem>(static_cast<int>(currentItem) - 1);
    }
  } else if (right) {
    // Auswahl nach rechts
    if (currentItem == MENU_CLEAN) {
      currentItem = MENU_FEED;
    } else {
      currentItem = static_cast<MenuItem>(static_cast<int>(currentItem) + 1);
    }
  } else if (ok) {
    // Aktion bestätigen
    switch (currentItem) {
      case MENU_FEED: pendingAction = ACTION_FEED; break;
      case MENU_PLAY: pendingAction = ACTION_PLAY; break;
      case MENU_REST: pendingAction = ACTION_REST; break;
      case MENU_CLEAN: pendingAction = ACTION_CLEAN; break;
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
