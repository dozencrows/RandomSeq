#include "Quantiser.h"
#include <Arduino.h>
#include <avr/pgmspace.h>

static const int noteVoltsLookUp[] PROGMEM = {
  2047, 2030, 2013, 1996, 1979, 1962, 1945, 1928, 1911, 1893, 1876, 1859, 1842, 1825, 1808, 1791,
  1774, 1757, 1740, 1723, 1706, 1689, 1672, 1655, 1638, 1621, 1603, 1586, 1569, 1552, 1535, 1518,
  1501, 1484, 1467, 1450, 1433, 1416, 1399, 1382, 1365, 1348, 1331, 1313, 1296, 1279, 1262, 1245,
  1228, 1211, 1194, 1177, 1160, 1143, 1126, 1109, 1092, 1075, 1058, 1041, 1024, 1006, 989, 972,
  955, 938, 921, 904, 887, 870, 853, 836, 819, 802, 785, 768, 751, 734, 716, 699,
  682, 665, 648, 631, 614, 597, 580, 563, 546, 529, 512, 495, 478, 461, 444, 426,
  409, 392, 375, 358, 341, 324, 307, 290, 273, 256, 239, 222, 205, 188, 171, 154,
  136, 119, 102, 85, 68, 51, 34, 17, 0
}; 

static const char majorScale[] PROGMEM = {
  0, 2, 4, 5, 7, 9, 11, 12
};

static const char minorScale[] PROGMEM = {
  0, 2, 3, 5, 7, 8, 10, 12  
};

void Quantiser::setScale(Scale scale) {
  switch (scale) {
    case CHROMATIC:
      scaleSemitones_ = 0;
      break;

    case MAJOR:
      scaleSemitones_ = majorScale;
      break;

    case MINOR:
      scaleSemitones_ = minorScale;
      break;
  }
}

void Quantiser::setNote(int note) {
  note_ = note;
  octave_ = note / 12;
  semitone_ = note % 12;

  if (scaleSemitones_) {
    for (int i = 0; i < 8; i++) {
      int scaleSemitone = pgm_read_byte_near(scaleSemitones_ + i);
      if (semitone_ <= scaleSemitone) {
        semitone_ = scaleSemitone;
        break;
      }
    }
  
    if (semitone_ == 12) {
      octave_++;
      semitone_ = 0;
    }  
  }
}

int Quantiser::getCV() {
  int noteLookup = octave_ * 12 + semitone_;
  return pgm_read_word_near(noteVoltsLookUp + noteLookup);
}

int Quantiser::getSemitone() {
  return semitone_;
}
