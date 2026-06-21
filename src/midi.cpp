#include "midi.h"

namespace {
constexpr unsigned char kStatusNoteOn = 0x90;
constexpr unsigned char kStatusNoteOff = 0x80;

void listOutputs(RtMidiOut *midiout) {
  unsigned int nPorts = midiout->getPortCount();
  if (nPorts == 0) {
    std::cout << "No output ports available!" << std::endl;
  }

  for (unsigned int i = 0; i < nPorts; i++) {
    try {
      std::string portName = midiout->getPortName(i);
      std::cout << "Output port #" << i << ": " << portName << std::endl;
    } catch (RtMidiError &error) {
      error.printMessage();
    }
  }
}
} // namespace

MIDIManager::MIDIManager(EventBus *event_bus) : event_bus_(event_bus) {
  midiout_ = std::make_unique<RtMidiOut>();
  midiin_ = std::make_unique<RtMidiIn>();

  listOutputs(midiout_.get());

  if (midiout_->getPortCount() > 0) {
    midiout_->openPort(0);
  }
}

void MIDIManager::SendMessage(unsigned int data1, unsigned int data2,
                              unsigned int data3) {
  std::vector<unsigned char> message{static_cast<unsigned char>(data1),
                                     static_cast<unsigned char>(data2),
                                     static_cast<unsigned char>(data3)};
  midiout_->sendMessage(&message);
}

void MIDIManager::SendNoteOn(const Note *note) {
  SendMessage(kStatusNoteOn, note->pitch, note->velocity);
  event_bus_->DispatchEvent(
      this, Event{EventName::kNoteOn, static_cast<unsigned int>(note->pitch)});
}

void MIDIManager::SendNoteOff(const Note *note) {
  SendMessage(kStatusNoteOff, note->pitch, note->velocity);
  event_bus_->DispatchEvent(
      this, Event{EventName::kNoteOff, static_cast<unsigned int>(note->pitch)});
}
