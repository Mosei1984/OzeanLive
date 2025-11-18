#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

class ButtonManager
{
private:
    int lastBtnLeftState = HIGH;
    int lastBtnOkState = HIGH;
    int lastBtnRightState = HIGH;

    bool btnLeftPressed = false;
    bool btnOkPressed = false;
    bool btnRightPressed = false;

    unsigned long lastPollMs = 0;
    static constexpr unsigned long DEBOUNCE_MS = 40;

    uint8_t bleMask = 0;

public:
    void begin();
    void poll();
    void getAndClearPressed(bool &left, bool &ok, bool &right);
    void setBleMask(uint8_t mask);
};

extern ButtonManager Buttons;

#endif
