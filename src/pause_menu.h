#pragma once

enum PauseChoice {
  PAUSE_RESUME,
  PAUSE_SAVE_RESUME,
  PAUSE_SAVE_EXIT
};

PauseChoice runPauseMenu();
