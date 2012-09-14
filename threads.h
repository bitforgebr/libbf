/*
 * threads.h
 *
 *  Created on: Sep 14, 2012
 *      Author: gianni
 *
 * BitForge http://www.bitforge.com.br
 * Copyright (c) 2012 All Right Reserved,
 */

#ifndef __INCLUDE_LIBBF_THREADS_H_
#define __INCLUDE_LIBBF_THREADS_H_

#include <mutex>

namespace bitforge
{

/**
 * Helper function to run a lamba protected by a mutex
 *
 * example:
 * 		std::mutex m;
 * 		lock( m, [&]{ ..do something..});
 *
 * @param mutex Mutex lock to be held during the call.
 * @param fn Lambda / functor to be called.
 */
template<typename L, typename F>
void lock(L& mutex, F fn)
{
	std::lock_guard<L> hold(mutex);
	fn();
}

} // namespace bitforge

#endif // __INCLUDE_LIBBF_THREADS_H_