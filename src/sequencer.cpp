#include "sequencer.h"

namespace {
constexpr double PPQ = 480.0;

double microsPerTick(unsigned int bpm) { return 60000000.0 / bpm / PPQ; }
} // namespace

Sequencer::Sequencer(EventBus *event_bus, MIDIDelegate *midi_delegate)
    : event_bus_(event_bus), midi_delegate_(midi_delegate) {
  SetBPM(bpm_);
}

void Sequencer::SetBPM(unsigned int bpm) {
  if (bpm == 0) {
    return;
  }
  bpm_ = bpm;
  micros_per_tick_ = microsPerTick(bpm);
}

void Sequencer::Play(long long now) {
  last_tick_ = now;
  is_playing_ = true;
  event_bus_->DispatchEvent(this, Event{EventName::kTransportPlay, 0});
}

void Sequencer::Pause() {
  is_playing_ = false;
  event_bus_->DispatchEvent(this, Event{EventName::kTransportPause, 0});
}

void Sequencer::Stop() {
  is_playing_ = false;
  position_tick_ = 0;
  event_bus_->DispatchEvent(this, Event{EventName::kTransportStop, 0});
}

void Sequencer::Run(long long now) {
  if (!is_playing_) {
    last_tick_ = now;
    return;
  }

  long long delta = now - last_tick_;
  if (delta < static_cast<long long>(micros_per_tick_)) {
    return;
  }

  while (delta >= static_cast<long long>(micros_per_tick_)) {
    delta -= static_cast<long long>(micros_per_tick_);

    event_bus_->DispatchEvent(this,
                              Event{EventName::kTick, position_tick_++});

    if (loop_ && position_tick_ >= loop_range_[1]) {
      position_tick_ = static_cast<unsigned int>(loop_range_[0]);
    }
  }

  last_tick_ = now - delta;
}
