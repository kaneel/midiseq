#include <atomic>
#include <chrono>
#include <csignal>
#include <thread>

#include "chain.h"
#include "event_bus.h"
#include "midi.h"
#include "playback.h"
#include "sequence.h"
#include "sequencer.h"

namespace {
std::atomic<bool> g_running{true};

void HandleSignal(int) { g_running.store(false); }

template <typename T> T Now() {
  return std::chrono::duration_cast<T>(
      std::chrono::system_clock::now().time_since_epoch());
}

constexpr int QUARTER = 480;
constexpr int EIGHTH = QUARTER / 2;
} // namespace

int main() {
  std::signal(SIGINT, HandleSignal);
  std::signal(SIGTERM, HandleSignal);

  EventBus event_bus;
  MIDIManager midi_manager(&event_bus);
  PlaybackManager playback_manager;

  Sequencer sequencer(&event_bus, &midi_manager);
  Chain chain(&event_bus);

  Sequence seq(&event_bus, &midi_manager, &playback_manager, 1);
  Sequence seq2(&event_bus, &midi_manager, &playback_manager, 2);
  Sequence seq3(&event_bus, &midi_manager, &playback_manager, 3);

  event_bus.Register(&midi_manager);
  event_bus.Register(&sequencer);
  event_bus.Register(&chain);
  event_bus.Register(&seq);
  event_bus.Register(&seq2);
  event_bus.Register(&seq3);

  event_bus.AddListener(&sequencer, EventName::kTransportPause,
                        [&](unsigned int) { playback_manager.CutAll(); });
  event_bus.AddListener(&sequencer, EventName::kTransportStop,
                        [&](unsigned int) { playback_manager.CutAll(); });

  event_bus.AddListener(&sequencer, EventName::kTick, [&](unsigned int) {
    playback_manager.PollPlaybacks();
  });

  seq.SetClockSource(&sequencer, EventName::kTick);
  seq.SetTrigMode(Sequence::TrigMode::kAdvanceByOne);
  seq2.SetClockSource(&sequencer, EventName::kTick);
  seq2.SetTrigMode(Sequence::TrigMode::kAdvanceByOne);
  seq3.SetClockSource(&sequencer, EventName::kNoteOff);
  seq3.SetTrigMode(Sequence::TrigMode::kAdvanceToNextNote);

  seq.SetLength(4 * QUARTER);
  seq.SetLoop(0, 4 * QUARTER);
  seq.Loop(true);
  seq.AddNote(0, Note{63, 50, 500});

  seq2.SetLength(4 * QUARTER);
  seq2.SetLoop(0, 4 * QUARTER);
  seq2.Loop(true);
  seq2.AddNote(0, Note{60, 60, 500});

  seq3.SetLength(QUARTER);
  seq3.SetLoop(0, 4 * QUARTER);
  seq3.Loop(true);
  seq3.AddNote(0, Note{100, 63, 200});
  seq3.AddNote(QUARTER, Note{120, 63, 200});
  seq3.AddNote(QUARTER * 2, Note{110, 63, 200});

  seq.SetNextAction(&seq2, Sequence::PlayAction::kLoopIteration,
                    Sequence::NextAction::kNext, 3);
  seq2.SetNextAction(&seq, Sequence::PlayAction::kLoopIteration,
                     Sequence::NextAction::kPrevious, 1);

  chain.AddSequence(&seq);
  chain.AddSequence(&seq2);

  seq3.Play();
  event_bus.AddListener(&chain, EventName::kNoteOff, [&](unsigned int data) {
    printf("[chain] note off %u\n", data);
    seq3.Trig();
  });

  chain.SetActiveSequence(0);

  sequencer.SetBPM(120);
  sequencer.Play(Now<std::chrono::microseconds>().count());

  while (g_running.load()) {
    sequencer.Run(Now<std::chrono::microseconds>().count());
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }

  sequencer.Stop();
  playback_manager.CutAll();
  return 0;
}
