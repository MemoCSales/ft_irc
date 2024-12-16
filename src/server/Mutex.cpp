#include "Mutex.hpp"


Mutex::Mutex():_status(0)
{
	if (pthread_mutex_init(&_mutex, NULL) != 0)
	{
		throw std::runtime_error("Failed to initialize mutex");
	}
}

Mutex::~Mutex()
{
	pthread_mutex_destroy(&_mutex);
}

void Mutex::lock()
{
	pthread_mutex_lock(&_mutex);
	_status = 1;
}

void Mutex::unlock()
{
	{
		if (_status)
		{
			_status = 0;
			pthread_mutex_unlock(&_mutex);
		}
	}
}