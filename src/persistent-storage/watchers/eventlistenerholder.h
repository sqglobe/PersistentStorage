#ifndef EVENTLISTENERHOLDER_H
#define EVENTLISTENERHOLDER_H

#include <algorithm>
#include <list>
#include <memory>

namespace prstorage {

template <typename EventQueueType>
class EventListenerHolder {
 public:
  using EventsInfo = std::pair<typename EventQueueType::Event,
                               typename EventQueueType::Handle>;

 public:
  EventListenerHolder(std::shared_ptr<EventQueueType> eventQueue,
                      std::initializer_list<EventsInfo> handles);
  EventListenerHolder(std::shared_ptr<EventQueueType> eventQueue,
                      const std::list<EventsInfo>& handles);
  EventListenerHolder(std::shared_ptr<EventQueueType> eventQueue,
                      std::list<EventsInfo>&& handles);
  EventListenerHolder(EventListenerHolder&& holder);
  ~EventListenerHolder();

 private:
  EventListenerHolder(const EventListenerHolder&) = delete;
  EventListenerHolder& operator=(const EventListenerHolder&) = delete;

 private:
  std::weak_ptr<EventQueueType> mEventQueue;
  std::list<std::pair<typename EventQueueType::Event,
                      typename EventQueueType::Handle>>
      mHandles;
};

}  // namespace prstorage
template <typename EventQueueType>
prstorage::EventListenerHolder<EventQueueType>::EventListenerHolder(
    std::shared_ptr<EventQueueType> eventQueue,
    std::initializer_list<EventsInfo> handles) :
    mEventQueue(eventQueue),
    mHandles(handles)
{
}

template <typename EventQueueType>
prstorage::EventListenerHolder<EventQueueType>::EventListenerHolder(
    std::shared_ptr<EventQueueType> eventQueue,
    const std::list<EventListenerHolder::EventsInfo>& handles) :
    mEventQueue(eventQueue),
    mHandles(handles)
{
}

template <typename EventQueueType>
prstorage::EventListenerHolder<EventQueueType>::EventListenerHolder(
    std::shared_ptr<EventQueueType> eventQueue,
    std::list<EventListenerHolder::EventsInfo>&& handles) :
    mEventQueue(eventQueue),
    mHandles(std::move(handles))
{
}

template <typename EventQueueType>
prstorage::EventListenerHolder<EventQueueType>::EventListenerHolder(
    EventListenerHolder&& holder) :
    mEventQueue(std::move(holder.mEventQueue)),
    mHandles(std::move(holder.mHandles))
{
}

template <typename EventQueueType>
prstorage::EventListenerHolder<EventQueueType>::~EventListenerHolder()
{
  if (auto ptr = mEventQueue.lock()) {
    std::for_each(std::cbegin(mHandles), std::cend(mHandles),
                  [&ptr](const auto& handle) mutable {
                    ptr->removeListener(handle.first, handle.second);
                  });
  }
}
#endif  // EVENTLISTENERHOLDER_H
