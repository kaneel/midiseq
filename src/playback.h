#ifndef PLAYBACK_MANAGER_H
#define PLAYBACK_MANAGER_H

#include "note.h"
#include <functional>

class PlaybackManager {
  static constexpr unsigned int MAX_PLAYBACKS = 64;

public:
  using ExpireCallback = std::function<void(const Note *)>;

  struct Playback {
    const Note *note{nullptr};
    long long remaining{0};
    bool is_allocated{false};
    ExpireCallback on_expire;
  };

  PlaybackManager() = default;
  ~PlaybackManager() = default;

  void AssignPlayback(const Note *note, ExpireCallback on_expire);
  void PollPlaybacks();
  void CutAll();
  void FindAndCut(const Note *note);

private:
  Playback playbacks_[MAX_PLAYBACKS]{};
};

#endif
