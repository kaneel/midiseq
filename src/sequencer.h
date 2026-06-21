#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "event_bus.h"
#include "midi.h"

class Sequencer : public EventBusDelegate {
public:
  Sequencer(EventBus *event_bus, MIDIDelegate *midi_delegate);
  ~Sequencer() = default;

  void SetBPM(unsigned int bpm);
  void SetLoop(long long start, long long end) {
    loop_range_[0] = start;
    loop_range_[1] = end;
  }

  void Play(long long now);
  void Pause();
  void Stop();
  void Loop(bool loop) { loop_ = loop; }
  void Run(long long now);

private:
  EventBus *event_bus_;
  MIDIDelegate *midi_delegate_;

  unsigned int bpm_{120};

  long long last_tick_{0};
  double micros_per_tick_{0.0};
  unsigned int position_tick_{0};

  bool is_playing_{false};
  long long loop_range_[2]{0, 0};
  bool loop_{false};
};

#endif
