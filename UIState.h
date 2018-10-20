/*
 * Abstract base for UI state
 */
#if !defined(__UISTATE_H__)
#define __UISTATE_H__

class PureDigit;
class ShiftRegister;
class Quantiser;

class UIState {
  public:
    UIState(PureDigit& digit, ShiftRegister& shiftRegister, Quantiser& quantiser) :
      digit_(digit), shiftRegister_(shiftRegister), quantiser_(quantiser) {}
    virtual void init() = 0;
    virtual void select() = 0;
    virtual void update() = 0;

  protected:
    PureDigit& digit_;
    ShiftRegister& shiftRegister_;
    Quantiser& quantiser_;
};


#endif
