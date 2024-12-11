#include "LockGuard.hpp"

LockGuard::LockGuard(Mutex& m) : mutex(m)
{
	mutex.lock();
}

LockGuard::~LockGuard()
{
	mutex.unlock();
}