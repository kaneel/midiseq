#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <map>

#include "event_bus.h"
#include "midi.h"
#include "note.h"

struct Note;
class PlaybackManager;

class Sequence : public EventBusDelegate {
public:
  enum class PlayAction { kNone, kLoopIteration, kBar };
  enum class NextAction { kNone, kNext, kPrevious, kFirst, kLast };
  enum class TrigMode {
    kAdvanceByOne,
    kAdvanceByStep,
    kAdvanceToNextNote,
    kRestart,
    kFireCurrent,
  };

  Sequence(EventBus *event_bus, MIDIDelegate *midi_delegate,
           PlaybackManager *playback_manager, unsigned int id);
  ~Sequence() override;

  unsigned int length() const { return length_; }
  PlayAction play_action() const { return play_action_; }
  NextAction next_action() const { return next_action_; }
  unsigned int next_timing() const { return next_timing_; }
  unsigned int id() const { return id_; }
  unsigned int stepper() const { return stepper_; }

  void SetLength(long long length) { length_ = length; }
  void SetStart(unsigned int start_tick) { start_tick_ = start_tick; }
  void SetLoop(long long start, long long end) {
    loop_range_[0] = start;
    loop_range_[1] = end;
  }
  void SetStepper(unsigned int ticks) { stepper_ = ticks; }

  void SetNextAction(Sequence *seq, PlayAction p_action, NextAction action,
                     unsigned int timing) {
    next_sequence_ = seq;
    next_action_ = action;
    play_action_ = p_action;
    next_timing_ = timing;
  }
  void Play() { is_active_ = true; }
  void Stop() { is_active_ = false; }
  void Pause() { is_active_ = false; }
  void Loop(bool loop) { loop_ = loop; }

  void AddNote(unsigned int position, const Note &note);

  // Bind this sequence's clock to (emitter, event). Replaces any prior binding.
  void SetClockSource(EventBusDelegate *emitter, EventName event);
  void ClearClockSource();
  void SetTrigMode(TrigMode mode) { trig_mode_ = mode; }
  TrigMode trig_mode() const { return trig_mode_; }

  // One pulse from the bound clock source.
  void Trig();

  // Subscriber helpers.
  size_t AddListener(EventName event, EventListener listener) {
    return event_bus_->AddListener(this, event, listener);
  }
  void RemoveListener(EventName event, size_t id) {
    event_bus_->RemoveListener(this, event, id);
  }

  void DispatchEvent(Event event) { event_bus_->DispatchEvent(this, event); }

private:
  void AdvanceTo(long long target);
  void FireNotesAtPlayhead();

  unsigned int id_{0};
  unsigned int start_tick_{0};
  unsigned int length_{0};
  MIDIDelegate *delegate_;
  EventBus *event_bus_;
  PlaybackManager *playback_manager_;

  long long playhead_{0};

  std::map<unsigned int, std::vector<Note>> notes_;

  bool is_active_{false};
  bool loop_{false};
  long long loop_range_[2]{0, 0};
  unsigned int stepper_{480};

  Sequence *next_sequence_{nullptr};
  PlayAction play_action_{PlayAction::kNone};
  NextAction next_action_{NextAction::kNone};
  unsigned int next_timing_{0};

  EventBusDelegate *clock_emitter_{nullptr};
  EventName clock_event_{EventName::kTick};
  size_t clock_handle_{0};
  TrigMode trig_mode_{TrigMode::kAdvanceByOne};
};

#endif
