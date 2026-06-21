#ifndef CHAIN_H
#define CHAIN_H

#include <cstddef>
#include <map>
#include <vector>

#include "event_bus.h"
#include "sequence.h"

class Chain : public EventBusDelegate {
public:
  explicit Chain(EventBus *event_bus);
  ~Chain() override;

  void AddSequence(Sequence *seq);
  void RemoveSequence(Sequence *seq);
  void SetActiveSequence(int index);
  void SetNextSequence();

private:
  void ClearAdvanceSubscription();

  EventBus *event_bus_;
  std::vector<Sequence *> sequences_;

  // Forwarding subscriptions: per-sequence handles for kNoteOn/kNoteOff
  // re-dispatch under Chain's identity.
  struct ForwardHandles {
    size_t note_on{0};
    size_t note_off{0};
  };
  std::map<Sequence *, ForwardHandles> forward_handles_;

  Sequence *active_sequence_{nullptr};
  unsigned int active_sequence_index_{0};
  Sequence::PlayAction play_action_{Sequence::PlayAction::kNone};
  Sequence::NextAction next_action_{Sequence::NextAction::kNone};
  unsigned int target_iterations_{0};
  unsigned int wrap_count_{0};
  size_t advance_handle_{0};
};

#endif
