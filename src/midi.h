#ifndef MIDI_H
#define MIDI_H

#include <memory>

#include "RtMidi.h"
#include "event_bus.h"
#include "note.h"

class MIDIDelegate {
public:
  virtual void SendNoteOn(const Note *note) = 0;
  virtual void SendNoteOff(const Note *note) = 0;
  virtual void SendMessage(unsigned int data1, unsigned int data2,
                           unsigned int data3) = 0;
};

class MIDIManager : public MIDIDelegate, public EventBusDelegate {
public:
  MIDIManager(EventBus *event_bus);
  ~MIDIManager() = default;

  // MIDIDelegate
  void SendNoteOn(const Note *note) override;
  void SendNoteOff(const Note *note) override;
  void SendMessage(unsigned int data1, unsigned int data2,
                   unsigned int data3) override;

private:
  std::unique_ptr<RtMidiOut> midiout_;
  std::unique_ptr<RtMidiIn> midiin_;
  EventBus *event_bus_;

  unsigned output_port_{0};
  unsigned input_port_{0};
};

#endif
