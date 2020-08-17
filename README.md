# Spike-Midi-Input-Interface

An experiment to build a circuit to handle midi data from a standard midi keyboard controller.

[![Overhead of Circuit](https://github.com/JimKnowler/spike-midi-input-interface/raw/master/docs/circuit%20-%20overhead%20-%20small.jpg)](https://github.com/JimKnowler/spike-midi-input-interface/raw/master/docs/circuit%20-%20overhead.jpg)

[![Circuit + Oscilloscope](https://github.com/JimKnowler/spike-midi-input-interface/raw/master/docs/Work%20in%20progres%20-%20circuit%20and%20scope%20-%20small.jpg)](https://raw.githubusercontent.com/JimKnowler/spike-midi-input-interface/master/docs/Work%20in%20progres%20-%20circuit%20and%20scope.jpg)

![Serial Monitor output](https://github.com/JimKnowler/spike-midi-input-interface/raw/master/docs/screenshot%20-%20note%20off%20and%20on%20parsing.png)

*Note: this kind of interface can be implemented simply by purchasing an [Arduino MIDI shield](https://learn.sparkfun.com/tutorials/midi-shield-hookup-guide/all) that uses your Arduino's Rx serial comms*

*This is a project to learn how to implement serial communication for MIDI in circuitry*

## Prior Art

- [Receive - UART from scratch - James Sharman](https://www.youtube.com/watch?v=Bqc7YsC1f1Q&t=662s)

- [Sparkfun Midi Tutorial](https://learn.sparkfun.com/tutorials/midi-tutorial/hardware--electronic-implementation)

- [Arduino MIDI-in shield](https://www.instructables.com/id/Arduino-MIDI-in-shield/)

## Video Capture

- [Video of circuit reading midi messages](https://github.com/JimKnowler/spike-midi-input-interface/raw/master/docs/circuit%20-%20read%20from%20midi.MOV)

- [Video of Keyboard connected via Circuit to Arduino](
https://github.com/JimKnowler/spike-midi-input-interface/raw/master/docs/keyboard%20-%20circuit%20-%20read%20raw%20values%20from%20midi.MOV)

## Components

- 4N28 OptoCoupler/PhotoTransistor
    - Used to isolate MIDI instrument from circuit + prevent creating a ground loop
- 1Mhz oscillator - clock
- SN74LS163 - up/down counter
    - divide 1Mhz clock to 32.150kHz clock for MIDI
    - count reception of 9 bits (1 x start bit + 8 x data bits)
- SN74LS00 - NAND gate
    - used to create SR Latch
    - SET by start bit in midi message
    - RESET when 9 bits have been counted
    - used to synchronise count down from 9 while receiving data
    - used to allow clock to pulse while receiving data
    - used to signal 8 bit register (& arduino) that data is available 
- SN74164 - SHIFT in PARALLEL out
    - SHIFT in data bits from midi signal
    - PARALLEL out to 8 bit register
- SN74LS273 - 8 bit Register
    - Cache the last received 8 bit MIDI message 
- Arduino Nano - Microprocessor
    - Interrupt - triggered when 8 bit register is filled with MIDI message
    - Read MIDI message from 8 bit Register
    - Basic MIDI code parsing to detect NOTE ON/OFF messages and determine Velocity, Note + Octave

