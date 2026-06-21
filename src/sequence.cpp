#include "sequence.h"
#include "playback.h"

Sequence::Sequence(EventBus *event_bus, MIDIDelegate *delegate,
                   PlaybackManager *playback_manager, unsigned int id)
    : id_(id), delegate_(delegate), event_bus_(event_bus),
      playback_manager_(playback_manager) {}

Sequence::~Sequence() { ClearClockSource(); }

void Sequence::AddNote(unsigned int position, const Note &note) {
  if (notes_.find(position) != notes_.end()) {
    notes_[position].push_back(note);
    return;
  }
  notes_.insert({position, std::vector<Note>{note}});
}

void Sequence::SetClockSource(EventBusDelegate *emitter, EventName event) {
  ClearClockSource();
  clock_emitter_ = emitter;
  clock_event_ = event;
  clock_handle_ = event_bus_->AddListener(
      emitter, event, [this](unsigned int) { Trig(); });
}

void Sequence::ClearClockSource() {
  if (clock_emitter_ != nullptr && clock_handle_ != 0) {
    event_bus_->RemoveListener(clock_emitter_, clock_event_, clock_handle_);
  }
  clock_emitter_ = nullptr;
  clock_handle_ = 0;
}

void Sequence::Trig() {
  if (!is_active_) {
    return;
  }

  long long target = playhead_;
  switch (trig_mode_) {
  case TrigMode::kAdvanceByOne:
    target = playhead_ + 1;
    break;
  case TrigMode::kAdvanceByStep:
    target = playhead_ + static_cast<long long>(stepper_);
    break;
  case TrigMode::kAdvanceToNextNote: {
    auto it = notes_.upper_bound(static_cast<unsigned int>(playhead_));
    if (it == notes_.end()) {
      if (!loop_ || notes_.empty()) {
        return;
      }
      it = notes_.begin();
    }
    target = it->first;
    break;
  }
  case TrigMode::kRestart:
    target = loop_range_[0];
    break;
  case TrigMode::kFireCurrent:
    break;
  }

  AdvanceTo(target);
}

void Sequence::AdvanceTo(long long target) {
  bool wrapped = false;
  if (loop_ && target >= loop_range_[1]) {
    long long span = loop_range_[1] - loop_range_[0];
    if (span > 0) {
      target = loop_range_[0] + ((target - loop_range_[0]) % span);
    } else {
      target = loop_range_[0];
    }
    wrapped = true;
  }

  playhead_ = target;
  FireNotesAtPlayhead();

  if (wrapped) {
    event_bus_->DispatchEvent(this, Event{EventName::kLoopWrap, id_});
  }
}

void Sequence::FireNotesAtPlayhead() {
  auto it = notes_.find(static_cast<unsigned int>(playhead_));
  if (it == notes_.end()) {
    return;
  }

  for (auto &note : it->second) {
    playback_manager_->FindAndCut(&note);
    delegate_->SendNoteOn(&note);
    event_bus_->DispatchEvent(
        this, Event{EventName::kNoteOn, static_cast<unsigned int>(note.pitch)});
    playback_manager_->AssignPlayback(&note, [this](const Note *expired) {
      delegate_->SendNoteOff(expired);
      event_bus_->DispatchEvent(
          this, Event{EventName::kNoteOff,
                      static_cast<unsigned int>(expired->pitch)});
    });
  }
}
