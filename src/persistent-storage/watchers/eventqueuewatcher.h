#ifndef EVENTQUEUEWATCHER_H
#define EVENTQUEUEWATCHER_H

#include <eventpp/eventqueue.h>
#include <functional>
#include <memory>
#include <thread>

#include <iostream>

#include "enqueuedevents.h"
#include "eventlistenerholder.h"

namespace prstorage {

template <typename Element>
class EventQueueWatcher {
 public:
  using CallbackType = std::function<void(EnqueuedEvents, const Element&)>;

 private:
  using EventQueueType =
      eventpp::EventQueue<EnqueuedEvents, void(EnqueuedEvents, const Element&)>;

 public:
  EventQueueWatcher();
  ~EventQueueWatcher();

 private:
  EventQueueWatcher& operator=(const EventQueueWatcher&) = delete;
  EventQueueWatcher(const EventQueueWatcher&) = delete;

 public:
  EventListenerHolder<EventQueueType> appendListener(
      EnqueuedEvents event,
      const CallbackType& callback);
  EventListenerHolder<EventQueueType> appendListener(
      unsigned char eventMask,
      const CallbackType& callback);

  void appendPermanentListener(EnqueuedEvents event,
                               const CallbackType& callback);
  void appendPermanentListener(unsigned char eventMask,
                               const CallbackType& callback);

 protected:
  void elementAdded(const Element& element);
  void elementRemoved(const Element& element);
  void elementUpdated(const Element& element);

 private:
  auto addEventHandler(
      EnqueuedEvents event,
      const EventQueueWatcher<Element>::CallbackType& callback);

 private:
  std::shared_ptr<EventQueueType> mEventQueue;
  bool finished = false;
  std::thread mThread;
};

}  // namespace prstorage
template <typename Element>
prstorage::EventQueueWatcher<Element>::EventQueueWatcher() :
    mEventQueue(std::make_shared<EventQueueType>())
{
  auto thread_func = [& is_finished = this->finished](
                         const std::shared_ptr<EventQueueType>& sh_ptr) {
    auto queue_ptr = std::weak_ptr<EventQueueType>(sh_ptr);
    while (!is_finished) {
      if (auto ptr = queue_ptr.lock(); !ptr) {
        break;
      } else {
        if (auto waitRes = ptr->waitFor(std::chrono::milliseconds(10))) {
          try {
            ptr->process();
          } catch (const std::exception& ex) {
            std::cerr << "Get exception when try to process events : "
                      << ex.what() << std::endl;
          }
        }
      }
    }
  };

  mThread = std::thread(thread_func, mEventQueue);
}

template <typename Element>
prstorage::EventQueueWatcher<Element>::~EventQueueWatcher()
{
  finished = true;
  if (mThread.joinable()) {
    mThread.join();
  }
}

template <typename Element>
prstorage::EventListenerHolder<
    typename prstorage::EventQueueWatcher<Element>::EventQueueType>
prstorage::EventQueueWatcher<Element>::appendListener(
    EnqueuedEvents event,
    const EventQueueWatcher<Element>::CallbackType& callback)
{
  if (event == EnqueuedEvents::ALL_EVENTS) {
    return EventListenerHolder(
        mEventQueue, {
                         addEventHandler(EnqueuedEvents::ADDED, callback),
                         addEventHandler(EnqueuedEvents::UPDATED, callback),
                         addEventHandler(EnqueuedEvents::DELETED, callback),
                     });
  } else if (event == EnqueuedEvents::ADDED) {
    return EventListenerHolder(
        mEventQueue, {addEventHandler(EnqueuedEvents::ADDED, callback)});
  } else if (event == EnqueuedEvents::UPDATED) {
    return EventListenerHolder(
        mEventQueue, {addEventHandler(EnqueuedEvents::UPDATED, callback)});
  } else if (event == EnqueuedEvents::DELETED) {
    return EventListenerHolder(
        mEventQueue, {addEventHandler(EnqueuedEvents::DELETED, callback)});
  }
  assert(false);
}

template <typename Element>
prstorage::EventListenerHolder<
    typename prstorage::EventQueueWatcher<Element>::EventQueueType>
prstorage::EventQueueWatcher<Element>::appendListener(
    unsigned char eventMask,
    const EventQueueWatcher::CallbackType& callback)
{
  std::list<typename EventListenerHolder<
      typename EventQueueWatcher<Element>::EventQueueType>::EventsInfo>
      handlers;

  std::initializer_list<EnqueuedEvents> events{
      EnqueuedEvents::ADDED, EnqueuedEvents::UPDATED, EnqueuedEvents::DELETED};
  for (auto event : events) {
    if (eventMask & event) {
      handlers.push_back(addEventHandler(event, callback));
    }
  }

  return EventListenerHolder(mEventQueue, handlers);
}

template <typename Element>
void prstorage::EventQueueWatcher<Element>::appendPermanentListener(
    EnqueuedEvents event,
    const EventQueueWatcher::CallbackType& callback)
{
  if (event == EnqueuedEvents::ALL_EVENTS) {
    mEventQueue->appendListener(EnqueuedEvents::ADDED, callback);
    mEventQueue->appendListener(EnqueuedEvents::UPDATED, callback);
    mEventQueue->appendListener(EnqueuedEvents::DELETED, callback);
  } else if (event == EnqueuedEvents::ADDED) {
    mEventQueue->appendListener(EnqueuedEvents::ADDED, callback);
  } else if (event == EnqueuedEvents::UPDATED) {
    mEventQueue->appendListener(EnqueuedEvents::UPDATED, callback);
  } else if (event == EnqueuedEvents::DELETED) {
    mEventQueue->appendListener(EnqueuedEvents::DELETED, callback);
  }
}

template <typename Element>
void prstorage::EventQueueWatcher<Element>::appendPermanentListener(
    unsigned char eventMask,
    const EventQueueWatcher::CallbackType& callback)
{
  std::initializer_list<EnqueuedEvents> events{
      EnqueuedEvents::ADDED, EnqueuedEvents::UPDATED, EnqueuedEvents::DELETED};
  for (auto event : events) {
    if (eventMask & event) {
      mEventQueue->appendListener(event, callback);
    }
  }
}

template <typename Element>
void prstorage::EventQueueWatcher<Element>::elementAdded(const Element& element)
{
  mEventQueue->enqueue(EnqueuedEvents::ADDED, element);
}

template <typename Element>
void prstorage::EventQueueWatcher<Element>::elementRemoved(
    const Element& element)
{
  mEventQueue->enqueue(EnqueuedEvents::DELETED, element);
}

template <typename Element>
void prstorage::EventQueueWatcher<Element>::elementUpdated(
    const Element& element)
{
  mEventQueue->enqueue(EnqueuedEvents::UPDATED, element);
}

template <typename Element>
auto prstorage::EventQueueWatcher<Element>::addEventHandler(
    EnqueuedEvents event,
    const EventQueueWatcher::CallbackType& callback)
{
  return std::make_pair(event, mEventQueue->appendListener(event, callback));
}

#endif  // EVENTQUEUEWATCHER_H
