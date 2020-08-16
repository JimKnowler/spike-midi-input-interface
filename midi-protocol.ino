// Elegoo Arduino Nano
// https://www.amazon.co.uk/ELEGOO-Arduino-board-ATmega328P-compatible/dp/B072BMYZ18
// Requires Arduino configuration:
// - Nano
// - ATMega328P
// - /dev/cu.wchusbserial14420

// 4N28 optocoupler midi interface taken from
// https://www.instructables.com/id/Arduino-MIDI-in-shield/

// requires https://github.com/Locoduino/RingBuffer

// nice summary of midi protocol https://www.nyu.edu/classes/bello/FMT_files/9_MIDI_code.pdf

#include <RingBuf.h>

// The interrupt pin will be set high when data is available to read from 
//     the 8 bit register
#define PIN_INTERRUPT   2

// Data can be read in parallel from pins 0 to 7 of the 8 bit data register
#define PIN_DATA_0      10
#define PIN_DATA_7      3  

// middle c = MIDI C3
#define MIDDLE_C 60     // 0x3C

const byte MIDI_MESSAGE_TYPE_BIT = 1 << 7;

enum MidiMessageType {
  DATA = 0,
  STATUS = 1,
};

struct MidiMessageStatus {
  byte type : 4;
  byte channel: 4;
};

// active sense: http://midi.teragonaudio.com/tech/midispec/sense.htm
const byte MIDI_STATUS_MESSAGE_ACTIVE_SENSE = 0xfe;

// labels for each note
const char* NOTE_IN_OCTAVE_LABELS[] = {
  "C",
  "C#",
  "D",
  "D#",
  "E",
  "F",
  "F#",
  "G",
  "G#",
  "A",
  "A#",
  "B",
};

// ring buffer for midi message codes
// - written from interrupt
// - read from loop
RingBuf<byte, 128> ringBuffer;

// the last status byte received
// a status byte can be used by multiple messages => i.e. 'running status'
byte currentStatus;

// used to store 2 bytes of data
int messageDataIndex = 0;
byte messageData[2];

byte readData() {
  byte value = 0;
  
  for (int pinData=PIN_DATA_7; pinData <= PIN_DATA_0; pinData++) {
    value = (value << 1) | digitalRead(pinData);
  }

  return value;
}

void interruptHandler() {
  ringBuffer.push(readData());  
}

void setup() {
  Serial.begin(57600);
  Serial.println("");
  Serial.println("MIDI Receiver prototype setup");
  
  for (int pinData=PIN_DATA_0; pinData>=PIN_DATA_7; pinData--) {
    pinMode(pinData, INPUT);
  }

  pinMode(PIN_INTERRUPT, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT), interruptHandler, RISING);
}

MidiMessageType getMidiMessageType(byte message) {
  return (0 != (message & MIDI_MESSAGE_TYPE_BIT)) ? STATUS : DATA;
}

MidiMessageStatus parseMidiMessageStatus(byte statusByte) {
  MidiMessageStatus messageStatus;
  messageStatus.type = (statusByte >> 4) & 0x0f;
  messageStatus.channel = statusByte & 0x0f;

  return messageStatus;
}

void messageDataToNote(byte note, char* output) {
  // middle c = C3
  const int delta = int(note) - int(MIDDLE_C);
  
  int noteInOctave = delta % 12;
  int octave = 3 + int(delta/12);
  
  if (delta < 0) {
    noteInOctave = (noteInOctave + 12) % 12;
    if (noteInOctave > 0) {
      octave -= 1;
    }
  }

  sprintf(output, "%s%d", NOTE_IN_OCTAVE_LABELS[noteInOctave], octave);
}

void processMidiMessage() {
  MidiMessageStatus messageStatus = parseMidiMessageStatus(currentStatus);
  char buffer[128];
  
  switch (messageStatus.type) {
    case 0x8:
    {
      // note off
      byte channel = messageStatus.channel;
      char note[4];
      messageDataToNote(messageData[0], note);
      sprintf(buffer, "note off [%s] on channel [%d]", note, channel + 1);
      break;
    }
    case 0x9:
      // note on (or off, if velocity is 0)
      {
        byte channel = messageStatus.channel;
        const byte velocity = messageData[1];
        char note[4];
        messageDataToNote(messageData[0], note);
        if (velocity == 0) {
          sprintf(buffer, "note off [%s] on channel [%d]", note, channel + 1);
        } else {
          sprintf(buffer, "note on [%s] on channel [%d] with velocity [%d]", note, channel + 1, velocity);
        }
      }
      break;
    default:
      {
        sprintf(buffer, "received unknown message [%02x] [%02x] [%02x]", currentStatus, messageData[0], messageData[1]);
      }
      break;
  }

  Serial.println(buffer);
}

void processMidiByte(byte value) {
  if (value == MIDI_STATUS_MESSAGE_ACTIVE_SENSE) {
      // ignore keyboard sending this message to let us know that it is still connected
      return;
  }
  
  const MidiMessageType messageType = getMidiMessageType(value);

  switch (messageType) {
    case STATUS:
    {
      currentStatus = value;
      messageDataIndex = 0;
      break;
    }
    case DATA:
    {
      messageData[messageDataIndex] = value;
      messageDataIndex += 1;

      if (messageDataIndex == 2) {
        processMidiMessage();
        messageDataIndex = 0;
      }
      break;
    }
  }
}

void loop() {
  byte value;

  while (ringBuffer.lockedPop(value)) {
    processMidiByte(value);
  }
}
