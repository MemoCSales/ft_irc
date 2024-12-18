#ifndef LOCKGUARD_HPP
# define LOCKGUARD_HPP

# include <iostream>
# include <sstream>
# include <string>
# include <typeinfo>
# include <Utils.hpp>
# include "Mutex.hpp"

# ifndef DEBUG
#  define DEBUG 0
# endif

class LockGuard
{
	public:
		explicit LockGuard(Mutex & m);
		~LockGuard();

	private:
		Mutex const& mutex;
};

// std::ostream& operator << (std::ostream& os, LockGuard& rhs);

#endif // LOCKGUARD_HPP