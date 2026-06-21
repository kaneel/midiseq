#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <cstddef>
#include <functional>
#include <map>

enum class EventName {
  kNoteOn,
  kNoteOff,
  kMidiMessage,
  kTick,
  kLoopWrap,
  kTransportPlay,
  kTransportPause,
  kTransportStop,
};

typedef std::function<void(unsigned int)> EventListener;
typedef std::map<size_t, EventListener> ListenerMap;
typedef std::map<EventName, ListenerMap> Listeners;

struct Event {
  EventName name;
  unsigned int data;
};

class EventBusDelegate {
public:
  virtual ~EventBusDelegate() = default;
  friend class EventBus;
};

class EventBus : public EventBusDelegate {
public:
  friend class EventBusDelegate;
  EventBus();
  ~EventBus() override = default;

  void Register(EventBusDelegate *delegate);
  size_t AddListener(EventBusDelegate *, EventName event,
                     EventListener listener);
  void RemoveListener(EventBusDelegate *, EventName event, size_t id);
  void DispatchEvent(EventBusDelegate *, Event event);

private:
  std::map<EventBusDelegate *, Listeners> delegates_;
  size_t next_id_{1};
};

#endif
