#include "buttons.h"
#include "config.h"

ButtonManager Buttons;

void ButtonManager::begin()
{
    pinMode(PIN_BTN_LEFT, INPUT_PULLUP);
    pinMode(PIN_BTN_OK, INPUT_PULLUP);
    pinMode(PIN_BTN_RIGHT, INPUT_PULLUP);
}

void ButtonManager::poll()
{
    unsigned long now = millis();
    if (now - lastPollMs < DEBOUNCE_MS)
        return;
    lastPollMs = now;

    int sLeft = digitalRead(PIN_BTN_LEFT);
    int sOk = digitalRead(PIN_BTN_OK);
    int sRight = digitalRead(PIN_BTN_RIGHT);

    if (lastBtnLeftState == HIGH && sLeft == LOW)
        btnLeftPressed = true;
    if (lastBtnOkState == HIGH && sOk == LOW)
        btnOkPressed = true;
    if (lastBtnRightState == HIGH && sRight == LOW)
        btnRightPressed = true;

    lastBtnLeftState = sLeft;
    lastBtnOkState = sOk;
    lastBtnRightState = sRight;

    if (bleMask & 0x01)
        btnLeftPressed = true;
    if (bleMask & 0x02)
        btnOkPressed = true;
    if (bleMask & 0x04)
        btnRightPressed = true;
}

void ButtonManager::getAndClearPressed(bool &left, bool &ok, bool &right)
{
    left = btnLeftPressed;
    ok = btnOkPressed;
    right = btnRightPressed;

    btnLeftPressed = false;
    btnOkPressed = false;
    btnRightPressed = false;
}

void ButtonManager::setBleMask(uint8_t mask)
{
    bleMask = mask;
#ifdef DEBUG_BUTTONS
    if (mask != 0)
    {
        Serial.print("[BLE] Button mask updated: 0x");
        Serial.println(mask, HEX);
    }
#endif
}
