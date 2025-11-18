#pragma once

enum StartChoice {
  START_NEW,
  START_LOAD,
  RESET_SAVE
};

// Returns user's choice. Blocks until selection made.
// hasSaveAvailable: if false, "Load" option is disabled/grayed out
StartChoice runStartMenu(bool hasSaveAvailable);
