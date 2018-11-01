/*
 * Quantiser
 */
#if !defined(__QUANTISER_H__)
#define __QUANTISER_H__

class Quantiser {
  public:
    enum Scale {
      CHROMATIC = 0, MAJOR, MINOR, PENTATONIC, BLUES, MELODIC_MINOR, HARMONIC_MINOR, SCALE_COUNT
    };

    void setScale(Scale scale);
    void setNote(int note);
    int getCV();
    int getSemitone();

  private:
    int note_ = 0;
    int octave_ = 0;
    int semitone_ = 0;
    const char* scaleSemitones_ = 0;
};

#endif
