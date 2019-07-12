#ifndef ENQUEUEDEVENTS_H
#define ENQUEUEDEVENTS_H

namespace prstorage {
enum class EnqueuedEvents {
  ADDED = 0x01,
  UPDATED = 0x02,
  DELETED = 0x04,
  ALL_EVENTS = ADDED | UPDATED | DELETED
};

inline unsigned char operator&(EnqueuedEvents event, unsigned char el)
{
  return static_cast<unsigned char>(event) & el;
}

inline unsigned char operator&(unsigned char el, EnqueuedEvents event)
{
  return event & el;
}

inline unsigned char operator&(EnqueuedEvents event1, EnqueuedEvents event2)
{
  return event1 & static_cast<unsigned char>(event2);
}

inline unsigned char operator|(EnqueuedEvents event, unsigned char el)
{
  return static_cast<unsigned char>(event) | el;
}

inline unsigned char operator|(unsigned char el, EnqueuedEvents event)
{
  return event | el;
}

inline unsigned char operator|(EnqueuedEvents event1, EnqueuedEvents event2)
{
  return event1 | static_cast<unsigned char>(event2);
}
}  // namespace prstorage

#endif  // ENQUEUEDEVENTS_H
