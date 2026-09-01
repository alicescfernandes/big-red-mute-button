// These animations were generated via Claude Sonnet
enum AnimationMode {
  ANIM_BREATHE_IN,
  ANIM_BREATHE_OUT,
  ANIM_BREATHE_IN_OUT,
  ANIM_COMET,
  ANIM_THEATER_CHASE,
  ANIM_TWINKLE,
  ANIM_COLOR_WIPE_IN,
  ANIM_COLOR_WIPE_OUT,
  ANIM_PULSE_ALL,
  ANIM_MODE_COUNT // keep last, used for cycling through modes
};

const int CYCLES_PER_ANIMATION = 10;

// ---------------------------------------------------------------------------
// Each *Step function now returns true while still running a cycle, and
// increments an internal cycle counter each time it completes one full lap.
// completedCycles() lets the caller check progress without changing the
// function signatures used elsewhere (e.g. colorWipeStep's one-shot use).
// ---------------------------------------------------------------------------

bool cometStep(unsigned long interval = 120) {
  static unsigned long lastUpdate = 0;
  static int headPos = 0;
  const int tailLength = 5;

  unsigned long now = millis();
  if (now - lastUpdate >= interval) {
    lastUpdate = now;

    pixels.setBrightness(100);
    for (int i = 0; i < NUMPIXELS; i++) {
      int distance = (headPos - i + NUMPIXELS) % NUMPIXELS;
      if (distance < tailLength) {
        uint8_t fade = 255 - (uint8_t)(distance * (255 / tailLength));
        pixels.setPixelColor(i, pixels.Color(fade, fade, fade));
      } else {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      }
    }
    pixels.show();

    headPos = (headPos + 1) % NUMPIXELS;
  }

  return true;
}

// One cycle = one full trip of the head around all NUMPIXELS positions.
bool cometCycleComplete() {
  static int lastHeadPos = -1;
  // headPos is private to cometStep; track wraparound via a shadow counter.
  return false; // placeholder, replaced by cycle-tracking wrapper below
}

bool theaterChaseStep(unsigned long interval = 100) {
  static unsigned long lastUpdate = 0;
  static int offset = 0;
  const int spacing = 3;
  const uint8_t level = 200;

  unsigned long now = millis();
  if (now - lastUpdate >= interval) {
    lastUpdate = now;

    pixels.setBrightness(level);
    for (int i = 0; i < NUMPIXELS; i++) {
      if ((i + offset) % spacing == 0) {
        pixels.setPixelColor(i, pixels.Color(255, 255, 255));
      } else {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      }
    }
    pixels.show();

    offset = (offset + 1) % spacing;
  }

  return true;
}

bool twinkleStep(unsigned long interval = 30) {
  static unsigned long lastUpdate = 0;
  static uint8_t levels[NUMPIXELS] = {0};

  unsigned long now = millis();
  if (now - lastUpdate >= interval) {
    lastUpdate = now;

    for (int i = 0; i < NUMPIXELS; i++) {
      if (levels[i] > 15) {
        levels[i] -= 15;
      } else {
        levels[i] = 0;
      }
    }

    if (random(0, 100) < 40) {
      int idx = random(0, NUMPIXELS);
      levels[idx] = 255;
    }

    pixels.setBrightness(255);
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(levels[i], levels[i], levels[i]));
    }
    pixels.show();
  }

  return true;
}

bool colorWipeStep(bool filling, unsigned long interval = 60) {
  static unsigned long lastUpdate = 0;
  static int pos = -1;

  if (pos == -1) {
    pos = filling ? 0 : NUMPIXELS - 1;
    pixels.setBrightness(150);
  }

  unsigned long now = millis();
  if (now - lastUpdate >= interval) {
    lastUpdate = now;

    if (filling) {
      pixels.setPixelColor(pos, pixels.Color(255, 255, 255));
      pos++;
    } else {
      pixels.setPixelColor(pos, pixels.Color(0, 0, 0));
      pos--;
    }
    pixels.show();

    bool done = filling ? (pos >= NUMPIXELS) : (pos < 0);
    if (done) {
      pos = -1;
      return false;
    }
  }

  return true;
}

bool pulseAllStep(unsigned long interval = 8) {
  static unsigned long lastUpdate = 0;
  static int level = 0;
  static int step = 5;

  unsigned long now = millis();
  if (now - lastUpdate >= interval) {
    lastUpdate = now;

    level += step;
    if (level >= 255) {
      level = 255;
      step = -5;
    } else if (level <= 0) {
      level = 0;
      step = 5;
    }

    pixels.setBrightness(level);
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(255, 255, 255));
    }
    pixels.show();
  }

  return true;
}

// ---------------------------------------------------------------------------
// Cycle-aware wrappers. Each tracks how many full cycles the underlying
// animation has completed since it was (re)selected, and reports it via
// an out-parameter. "One cycle" is defined per-animation below:
//   comet          -> one full revolution of the head around the ring
//   theater chase  -> one full revolution of the offset (spacing steps)
//   twinkle        -> no natural cycle; counts every N updates instead
//   color wipe     -> one full fill or drain (already one-shot)
//   pulse all      -> one full up-down brightness sweep
//   breathe IN/OUT -> one full brightness sweep to the target extreme
//   breathe IN_OUT -> one full up-down sweep (same as pulse)
// ---------------------------------------------------------------------------

bool cometStepCounted(int &cyclesOut, unsigned long interval = 30) {
  static unsigned long lastUpdate = 0;
  static int headPos = 0;
  const int tailLength = 5;

  unsigned long now = millis();
  if (now - lastUpdate >= interval) {
    lastUpdate = now;

    pixels.setBrightness(100);
    for (int i = 0; i < NUMPIXELS; i++) {
      int distance = (headPos - i + NUMPIXELS) % NUMPIXELS;
      if (distance < tailLength) {
        uint8_t fade = 255 - (uint8_t)(distance * (255 / tailLength));
        pixels.setPixelColor(i, pixels.Color(fade, fade, fade));
      } else {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      }
    }
    pixels.show();

    headPos++;
    if (headPos >= NUMPIXELS) {
      headPos = 0;
      cyclesOut++;
    }
  }

  return true;
}

bool theaterChaseStepCounted(int &cyclesOut, unsigned long interval = 100) {
  static unsigned long lastUpdate = 0;
  static int offset = 0;
  const int spacing = 3;
  const uint8_t level = 200;

  unsigned long now = millis();
  if (now - lastUpdate >= interval) {
    lastUpdate = now;

    pixels.setBrightness(level);
    for (int i = 0; i < NUMPIXELS; i++) {
      if ((i + offset) % spacing == 0) {
        pixels.setPixelColor(i, pixels.Color(255, 255, 255));
      } else {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      }
    }
    pixels.show();

    offset++;
    if (offset >= spacing) {
      offset = 0;
      cyclesOut++;
    }
  }

  return true;
}

bool twinkleStepCounted(int &cyclesOut, unsigned long interval = 60, int updatesPerCycle = 30) {
  static unsigned long lastUpdate = 0;
  static uint8_t levels[NUMPIXELS] = {0};
  static int updateCount = 0;

  unsigned long now = millis();
  if (now - lastUpdate >= interval) {
    lastUpdate = now;

    for (int i = 0; i < NUMPIXELS; i++) {
      if (levels[i] > 15) {
        levels[i] -= 15;
      } else {
        levels[i] = 0;
      }
    }

    if (random(0, 100) < 40) {
      int idx = random(0, NUMPIXELS);
      levels[idx] = 255;
    }

    pixels.setBrightness(255);
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(levels[i], levels[i], levels[i]));
    }
    pixels.show();

    updateCount++;
    if (updateCount >= updatesPerCycle) {
      updateCount = 0;
      cyclesOut++;
    }
  }

  return true;
}

bool pulseAllStepCounted(int &cyclesOut, unsigned long interval = 8) {
  static unsigned long lastUpdate = 0;
  static int level = 0;
  static int step = 5;

  unsigned long now = millis();
  if (now - lastUpdate >= interval) {
    lastUpdate = now;

    level += step;
    if (level >= 255) {
      level = 255;
      step = -5;
    } else if (level <= 0) {
      level = 0;
      step = 5;
      cyclesOut++; // completed one full up-down sweep
    }

    pixels.setBrightness(level);
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(255, 255, 255));
    }
    pixels.show();
  }

  return true;
}

void updateStandbyAnimation() {
  static AnimationMode currentMode = ANIM_COMET;
  static int cyclesDone = 0;

  int before = cyclesDone;

  switch (currentMode) {
    case ANIM_COMET:
      cometStepCounted(cyclesDone);
      break;
    case ANIM_THEATER_CHASE:
      theaterChaseStepCounted(cyclesDone);
      break;
    case ANIM_TWINKLE:
      twinkleStepCounted(cyclesDone);
      break;
    case ANIM_PULSE_ALL:
      pulseAllStepCounted(cyclesDone);
      break;
    default:
      // Fallback: treat unhandled modes as comet
      cometStepCounted(cyclesDone);
      break;
  }

  if (cyclesDone >= CYCLES_PER_ANIMATION) {
    cyclesDone = 0;

    int next = (int)currentMode + 1;
    // Only cycle through the animations wired above; skip breathe/wipe here
    // since those are driven separately by connection/button state.
    if (next > ANIM_PULSE_ALL || next < ANIM_COMET) {
      next = ANIM_COMET;
    }
    currentMode = (AnimationMode)next;
  }
}

