#ifndef MUTEX_HPP
# define MUTEX_HPP

# include <iostream>
# include <sstream>
# include <string>
# include <typeinfo>
# include <Utils.hpp>
# include <pthread.h>
# include <stdexcept>

# ifndef DEBUG
#  define DEBUG 0
# endif


class Mutex
{
	public:
		Mutex();
		~Mutex();

		void lock();
		void unlock();

	private:
		pthread_mutex_t _mutex;
};

#endif // MUTEX_HPP