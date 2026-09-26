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
  ANIM_MODE_COUNT
};

const int CYCLES_PER_ANIMATION = 15;


int getUserBrightness() {
  int value = preferences.getInt(
    PREFERENCE_KEY,
    PREFERENCE_DEFAULT_VALUE
  );

  return constrain(value, 0, 255);
}

int applyBrightness(int level) {
  return level * getUserBrightness() / 255;
}

bool cometStep(unsigned long interval = 120) {
  static unsigned long lastUpdate = 0;
  static int headPos = 0;
  const int tailLength = 5;

  unsigned long now = millis();

  if (now - lastUpdate >= interval) {
    lastUpdate = now;

    for (int i = 0; i < NUMPIXELS; i++) {
      int distance = (headPos - i + NUMPIXELS) % NUMPIXELS;

      if (distance < tailLength) {
        int fade = 255 - distance * (255 / tailLength);
        fade = applyBrightness(fade);
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


bool cometCycleComplete() {
  return false;
}


bool theaterChaseStep(unsigned long interval = 100) {
  static unsigned long lastUpdate = 0;
  static int offset = 0;
  const int spacing = 3;

  unsigned long now = millis();

  if (now - lastUpdate >= interval) {
    lastUpdate = now;

    int brightness = applyBrightness(200);

    for (int i = 0; i < NUMPIXELS; i++) {
      if ((i + offset) % spacing == 0) {
        pixels.setPixelColor(
          i,
          pixels.Color(brightness, brightness, brightness)
        );
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

    for (int i = 0; i < NUMPIXELS; i++) {
      int brightness = applyBrightness(levels[i]);
      pixels.setPixelColor(
        i,
        pixels.Color(brightness, brightness, brightness)
      );
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
  }

  unsigned long now = millis();

  if (now - lastUpdate >= interval) {
    lastUpdate = now;

    if (filling) {
      int brightness = applyBrightness(255);
      pixels.setPixelColor(
        pos,
        pixels.Color(brightness, brightness, brightness)
      );
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

    int brightness = applyBrightness(level);

    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(
        i,
        pixels.Color(brightness, brightness, brightness)
      );
    }

    pixels.show();
  }

  return true;
}


bool cometStepCounted(int& cyclesOut, unsigned long interval = 30) {
  static unsigned long lastUpdate = 0;
  static int headPos = 0;
  const int tailLength = 5;

  unsigned long now = millis();

  if (now - lastUpdate >= interval) {
    lastUpdate = now;

    for (int i = 0; i < NUMPIXELS; i++) {
      int distance = (headPos - i + NUMPIXELS) % NUMPIXELS;

      if (distance < tailLength) {
        int fade = 255 - distance * (255 / tailLength);
        fade = applyBrightness(fade);
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


bool theaterChaseStepCounted(int& cyclesOut, unsigned long interval = 100) {
  static unsigned long lastUpdate = 0;
  static int offset = 0;
  const int spacing = 3;

  unsigned long now = millis();

  if (now - lastUpdate >= interval) {
    lastUpdate = now;

    int brightness = applyBrightness(200);

    for (int i = 0; i < NUMPIXELS; i++) {
      if ((i + offset) % spacing == 0) {
        pixels.setPixelColor(
          i,
          pixels.Color(brightness, brightness, brightness)
        );
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


bool twinkleStepCounted(
  int& cyclesOut,
  unsigned long interval = 60,
  int updatesPerCycle = 30
) {
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

    for (int i = 0; i < NUMPIXELS; i++) {
      int brightness = applyBrightness(levels[i]);
      pixels.setPixelColor(
        i,
        pixels.Color(brightness, brightness, brightness)
      );
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


bool pulseAllStepCounted(int& cyclesOut, unsigned long interval = 8) {
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
      cyclesOut++;
    }

    int brightness = applyBrightness(level);

    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(
        i,
        pixels.Color(brightness, brightness, brightness)
      );
    }

    pixels.show();
  }

  return true;
}


void updateStandbyAnimation() {
  static AnimationMode currentMode = ANIM_COMET;
  static int cyclesDone = 0;

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
      cometStepCounted(cyclesDone);
      break;
  }

  if (cyclesDone >= CYCLES_PER_ANIMATION) {
    cyclesDone = 0;

    int next = static_cast<int>(currentMode) + 1;

    if (next > ANIM_PULSE_ALL || next < ANIM_COMET) {
      next = ANIM_COMET;
    }

    currentMode = static_cast<AnimationMode>(next);
  }
}


void blinkMcuPixel(bool isConnected) {
  static unsigned long previousMillis = 0;
  static bool isOn = false;

  const unsigned long interval = 1000;
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    isOn = !isOn;

    if (isOn) {
      if (isConnected) {
        // on GRB
        mcuPixel.setPixelColor(0, mcuPixel.Color(255, 0, 0));
      } else {
        mcuPixel.setPixelColor(0, mcuPixel.Color(255, 255, 255));
      }
    } else {
      mcuPixel.clear();
    }

    mcuPixel.show();
  }
}
