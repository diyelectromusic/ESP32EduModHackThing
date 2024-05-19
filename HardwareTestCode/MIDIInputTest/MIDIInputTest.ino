#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

#define MIDI_LED 2  // On board LED for the DevKit

void setup() {
  MIDI.begin(MIDI_CHANNEL_OMNI);
  pinMode (MIDI_LED, OUTPUT);
}

void loop() {
  if (MIDI.read()) {
    switch(MIDI.getType()) {
      case midi::NoteOn:
        digitalWrite (MIDI_LED, HIGH);
        break;
      case midi::NoteOff:
        digitalWrite (MIDI_LED, LOW);
        break;
    }
  }
}
