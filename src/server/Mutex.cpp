#include "Mutex.hpp"

Mutex::Mutex()
{
	if (pthread_mutex_init(&mutex, NULL) != 0)
	{
		throw std::runtime_error("Failed to initialize mutex");
	}
}

Mutex::~Mutex()
{
	pthread_mutex_destroy(&mutex);
}

void Mutex::lock()
{
	pthread_mutex_lock(&mutex);
}

void Mutex::unlock()
{
	pthread_mutex_unlock(&mutex);
}