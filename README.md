# Pure Digit Random Sequencer

An Arduino sketch that runs on the GSM Pure Digit Eurorack module (https://gmsn.co.uk/product/pure-digit/), providing a shift-register based random sequencer with quantiser.

## Getting Started

These instructions will get you a copy of the project up and running on your module.

### Prerequisites

You'll need a Pure Digit module, the Ardunio IDE and a programmer like USBasp or the Pololu USB AVR programmer.

Launch the Arduino IDE, and open the RandomSeq sketch from this folder. You only need the standard Arduino libraries; there is a copy of the Pure Digit library included in this repo (as there are some customisations).

### Installing

Follow the instructions in this video in this thread to build and burn the code to the module: https://forum.gmsn.co.uk/t/burning-code-to-the-gmsn-pure-digital-modules/42

### Hardware Mod

You will need to make sure your CV input jacks have the switching pin connected to ground. This can be done by soldering a short length of wire between two of the pins - see https://forum.gmsn.co.uk/t/shift-register-random-sequencer/1065/42

## Using

This module configures the Pure Digit as a shift-register based random sequencer (similar to the Turing Machine) with a quantiser. See https://musicthing.co.uk/pages/turing.html for how the Turing Machine works.

The controls are as follows:

* Encoder: turn to change setting values, press to cycle through settings
* In 1:    clock input; at least peaking at +1V to synchronise.
* In 2:    threshold offset CV; -5V to +5V offsets threshold value between -half and +half its range
  * So if you set the threshold to mid position with the encoder, -5V on CV will offset it to its minimum and +5V on CV will offset it to its maximum.
* Out:     quantised 1V/Octave output CV

There are five settings to cycle through on the encoder:

* Threshold: dial threshold from minimum to maximum. 
  * The LED display shows the position of a 'virtual' threshold pot. At the two extremes, the position display shows the decimal point; this indicates that the sequencer will loop and not randomly change its output. At the left extreme, the loop is 16 steps; and the right extreme the loop is 32 steps.
  * When the position is at the top, there is a 50:50 chance that the register will change one bit when stepped.  
* Range: set the upper octave limit for the output.
  * Minimum CV out is always 0V.
  * This range setting is from 0 to 9.
  * On every clock step, the decimal point in this mode will flash.
* Clock divider: set how many clocks to count before stepping the sequencer
  * 0: divisor is 1
  * 1: divisor is 2
  * 2: divisor is 12
  * 3: divisor is 24
  * 4: divisor is 48
* Write mode:
  * Turn encoder left; each click triggers a 0 bit to be written into the lowest bit of the shift register
  * Turn encoder right; each click triggers a 1 bit to be written into the lowest bit of the shift register
* Quantiser scale: turn the encoder to cycle through the scales
  * C:  chromatic
  * M:  major
  * m:  minor
  * P:  pentatonic
  * b:  blues
  * m.: melodic minor
  * H:  harmonic minor
  
## Built With

* [PureDigit Library](https://github.com/robgmsn/PureDigit) - The Pure Digit library

## Authors

* **Nick Tuckett** - *All coding* - [DozenCrows](https://github.com/dozencrows)

## License

This project is licensed under the MIT License - see the [LICENSE.txt](LICENSE.txt) file for details

## Acknowledgments

* Rob Spencer of GMSN for the Pure Digit, its library, help and advice.
* Andy Cobley for help and advice.
* Ric Wadsworth for testing and feedback.
* Tom Whitwell of Music Thing Modular for opening my mind to shift register random sequencing.

