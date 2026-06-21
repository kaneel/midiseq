#include "event_bus.h"

#include <vector>

EventBus::EventBus() {}

void EventBus::Register(EventBusDelegate *delegate) {
  delegates_.try_emplace(delegate);
}

size_t EventBus::AddListener(EventBusDelegate *delegate, EventName event,
                             EventListener listener) {
  Listeners &listeners = delegates_[delegate];
  size_t id = next_id_++;
  listeners[event].emplace(id, std::move(listener));
  return id;
}

void EventBus::RemoveListener(EventBusDelegate *delegate, EventName event,
                              size_t id) {
  auto it = delegates_.find(delegate);
  if (it == delegates_.end()) {
    return;
  }
  auto slot_it = it->second.find(event);
  if (slot_it == it->second.end()) {
    return;
  }
  slot_it->second.erase(id);
}

void EventBus::DispatchEvent(EventBusDelegate *delegate, Event event) {
  auto it = delegates_.find(delegate);
  if (it == delegates_.end()) {
    return;
  }
  auto slot_it = it->second.find(event.name);
  if (slot_it == it->second.end()) {
    return;
  }

  std::vector<EventListener> snapshot;
  snapshot.reserve(slot_it->second.size());
  for (const auto &[id, listener] : slot_it->second) {
    snapshot.push_back(listener);
  }
  for (const auto &listener : snapshot) {
    listener(event.data);
  }
}
