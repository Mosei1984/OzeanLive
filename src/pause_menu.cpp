#include "pause_menu.h"
#include "gfx.h"
#include "config.h"
#include "pet.h"
#include "eeprom_store.h"
#include "buttons.h"

PauseChoice runPauseMenu()
{
    uint8_t selected = 0;
    const uint8_t NUM_OPTIONS = 3;
    int16_t maxHP = getMaxHP();

    while (true)
    {
        tft.fillRect(30, 40, TFT_WIDTH - 60, TFT_HEIGHT - 80, 0x18C3);
        tft.drawRect(30, 40, TFT_WIDTH - 60, TFT_HEIGHT - 80, 0xFFFF);

        tft.setTextColor(0xFFE0);
        tft.setTextSize(2);
        tft.setCursor(80, 50);
        tft.print("PAUSED");

        tft.setTextColor(0xFFFF);
        tft.setTextSize(1);
        int16_t statsY = 72;
        tft.setCursor(40, statsY);
        tft.print("Hunger: ");
        tft.print(pet.hunger);

        tft.setCursor(40, statsY + 12);
        tft.print("Fun: ");
        tft.print(pet.fun);

        tft.setCursor(40, statsY + 24);
        tft.print("Energy: ");
        tft.print(pet.energy);

        tft.setCursor(40, statsY + 36);
        tft.print("HP: ");
        tft.print(pet.hp);
        tft.print("/");
        tft.print(maxHP);

        tft.setCursor(40, statsY + 48);
        tft.print("Age: ");
        uint32_t hours = pet.ageSec / 3600;
        uint32_t mins = (pet.ageSec % 3600) / 60;
        if (hours > 0) {
            tft.print(hours);
            tft.print("h ");
        }
        tft.print(mins);
        tft.print("m");

        const char *options[NUM_OPTIONS] = {"Resume", "Save & Resume", "Save & Exit"};
        int16_t startY = 140;
        int16_t spacing = 16;

        tft.setTextSize(1);
        for (uint8_t i = 0; i < NUM_OPTIONS; i++)
        {
            int16_t y = startY + i * spacing;

            if (i == selected)
            {
                tft.setTextColor(0xFFE0);
            }
            else
            {
                tft.setTextColor(0xFFFF);
            }

            tft.setCursor(40, y);
            if (i == selected)
            {
                tft.print("> ");
            }
            else
            {
                tft.print("  ");
            }
            tft.print(options[i]);
        }

        delay(100);

        Buttons.poll();
        bool btnLeftPressed = false;
        bool btnOkPressed = false;
        bool btnRightPressed = false;
        Buttons.getAndClearPressed(btnLeftPressed, btnOkPressed, btnRightPressed);

        if (btnLeftPressed)
        {
#ifdef DEBUG_BUTTONS
            Serial.print("[PAUSE] Navigate UP - Selected: ");
            Serial.println(selected);
#endif
            if (selected == 0)
            {
                selected = NUM_OPTIONS - 1;
            }
            else
            {
                selected--;
            }
        }

        if (btnRightPressed)
        {
#ifdef DEBUG_BUTTONS
            Serial.print("[PAUSE] Navigate DOWN - Selected: ");
            Serial.println(selected);
#endif
            selected = (selected + 1) % NUM_OPTIONS;
        }

        if (btnOkPressed)
        {
#ifdef DEBUG_BUTTONS
            Serial.print("[PAUSE] OK pressed on option: ");
            Serial.println(selected);
#endif

            if (selected == PAUSE_SAVE_RESUME)
            {
#ifdef DEBUG_BUTTONS
                Serial.println("[PAUSE] Action: SAVE & RESUME");
#endif
                saveFullIfDue(pet.hunger, pet.fun, pet.energy, pet.hp, pet.ageSec, pet.dead, true);

                tft.fillRect(40, TFT_HEIGHT - 40, TFT_WIDTH - 80, 20, 0x0000);
                tft.setTextColor(0x07E0);
                tft.setTextSize(1);
                tft.setCursor(80, TFT_HEIGHT - 35);
                tft.print("Saved!");
                delay(800);

                while (digitalRead(PIN_BTN_OK) == LOW)
                    ;

                return PAUSE_RESUME;
            }
            else if (selected == PAUSE_SAVE_EXIT)
            {
#ifdef DEBUG_BUTTONS
                Serial.println("[PAUSE] Action: SAVE & EXIT");
#endif
                saveFullIfDue(pet.hunger, pet.fun, pet.energy, pet.hp, pet.ageSec, pet.dead, true);

                while (digitalRead(PIN_BTN_OK) == LOW)
                    ;

                return PAUSE_SAVE_EXIT;
            }
            else
            {
#ifdef DEBUG_BUTTONS
                Serial.println("[PAUSE] Action: RESUME");
#endif
                while (digitalRead(PIN_BTN_OK) == LOW)
                    ;

                return PAUSE_RESUME;
            }
        }
    }
}
