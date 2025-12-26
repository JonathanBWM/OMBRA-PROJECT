#include "event.h"

#include <intrin.h>

bool OmbraEvent::Triggered()
{
	return bPreNotified;
}

void OmbraEvent::Await()
{
	if (bPreNotified)
		return;

	_lock.Lock();
	_lock.Lock();
	_lock.Unlock();
}

void OmbraEvent::Trigger()
{
	bPreNotified = true;
	_lock.Unlock();
}
