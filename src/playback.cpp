#include "playback.h"

void PlaybackManager::AssignPlayback(const Note *note,
                                     ExpireCallback on_expire) {
  for (auto &playback : playbacks_) {
    if (!playback.is_allocated) {
      playback.note = note;
      playback.remaining = note->length;
      playback.is_allocated = true;
      playback.on_expire = std::move(on_expire);
      return;
    }
  }
}

void PlaybackManager::PollPlaybacks() {
  for (auto &playback : playbacks_) {
    if (!playback.is_allocated) {
      continue;
    }
    if (--playback.remaining <= 0) {
      if (playback.on_expire) {
        playback.on_expire(playback.note);
      }
      playback.is_allocated = false;
      playback.on_expire = nullptr;
    }
  }
}

void PlaybackManager::CutAll() {
  for (auto &playback : playbacks_) {
    if (playback.is_allocated) {
      if (playback.on_expire) {
        playback.on_expire(playback.note);
      }
      playback.is_allocated = false;
      playback.on_expire = nullptr;
    }
  }
}

void PlaybackManager::FindAndCut(const Note *note) {
  for (auto &playback : playbacks_) {
    if (!playback.is_allocated) {
      continue;
    }
    if (playback.note->pitch == note->pitch) {
      if (playback.on_expire) {
        playback.on_expire(playback.note);
      }
      playback.is_allocated = false;
      playback.on_expire = nullptr;
    }
  }
}
