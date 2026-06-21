#include "chain.h"

#include <algorithm>

Chain::Chain(EventBus *event_bus) : event_bus_(event_bus) {}

Chain::~Chain() {
  ClearAdvanceSubscription();
  for (auto &[seq, handles] : forward_handles_) {
    event_bus_->RemoveListener(seq, EventName::kNoteOn, handles.note_on);
    event_bus_->RemoveListener(seq, EventName::kNoteOff, handles.note_off);
  }
}

void Chain::AddSequence(Sequence *seq) {
  sequences_.push_back(seq);

  ForwardHandles handles;
  handles.note_on = seq->AddListener(
      EventName::kNoteOn, [this](unsigned int data) {
        event_bus_->DispatchEvent(this, Event{EventName::kNoteOn, data});
      });
  handles.note_off = seq->AddListener(
      EventName::kNoteOff, [this](unsigned int data) {
        event_bus_->DispatchEvent(this, Event{EventName::kNoteOff, data});
      });
  forward_handles_[seq] = handles;
}

void Chain::RemoveSequence(Sequence *seq) {
  auto it = forward_handles_.find(seq);
  if (it != forward_handles_.end()) {
    event_bus_->RemoveListener(seq, EventName::kNoteOn, it->second.note_on);
    event_bus_->RemoveListener(seq, EventName::kNoteOff, it->second.note_off);
    forward_handles_.erase(it);
  }

  if (active_sequence_ == seq) {
    ClearAdvanceSubscription();
    active_sequence_ = nullptr;
  }

  sequences_.erase(std::remove(sequences_.begin(), sequences_.end(), seq),
                   sequences_.end());
}

void Chain::ClearAdvanceSubscription() {
  if (active_sequence_ != nullptr && advance_handle_ != 0) {
    event_bus_->RemoveListener(active_sequence_, EventName::kLoopWrap,
                               advance_handle_);
  }
  advance_handle_ = 0;
}

void Chain::SetActiveSequence(int index) {
  if (index < 0 || static_cast<size_t>(index) >= sequences_.size()) {
    return;
  }
  auto *seq = sequences_[index];
  if (seq == nullptr) {
    return;
  }

  ClearAdvanceSubscription();

  active_sequence_index_ = static_cast<unsigned int>(index);
  active_sequence_ = seq;
  active_sequence_->Play();

  play_action_ = seq->play_action();
  next_action_ = seq->next_action();
  target_iterations_ = seq->next_timing();
  wrap_count_ = 0;

  if (play_action_ == Sequence::PlayAction::kLoopIteration &&
      target_iterations_ > 0) {
    advance_handle_ = event_bus_->AddListener(
        active_sequence_, EventName::kLoopWrap, [this](unsigned int) {
          if (++wrap_count_ >= target_iterations_) {
            SetNextSequence();
          }
        });
  }
}

void Chain::SetNextSequence() {
  if (active_sequence_ == nullptr) {
    return;
  }

  active_sequence_->Stop();

  int target = static_cast<int>(active_sequence_index_);
  switch (next_action_) {
  case Sequence::NextAction::kNext:
    target = static_cast<int>(active_sequence_index_) + 1;
    break;
  case Sequence::NextAction::kPrevious:
    target = static_cast<int>(active_sequence_index_) - 1;
    break;
  case Sequence::NextAction::kFirst:
    target = 0;
    break;
  case Sequence::NextAction::kLast:
    target = static_cast<int>(sequences_.size()) - 1;
    break;
  case Sequence::NextAction::kNone:
    target = static_cast<int>(active_sequence_index_) + 1;
    break;
  }

  if (target < 0) {
    target = static_cast<int>(sequences_.size()) - 1;
  }
  if (static_cast<size_t>(target) >= sequences_.size()) {
    target = 0;
  }

  SetActiveSequence(target);
}
