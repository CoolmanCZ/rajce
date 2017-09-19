/*
 * Copyright (C) 2016, 2017 Radek Malcic
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _threadsafe_queue_h_
#define _threadsafe_queue_h_

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#ifdef __MINGW32__
#include "mingw.mutex.h"
#include "mingw.condition_variable.h"
#endif

template < typename T > class ThreadSafeQueue {
 private:
	ThreadSafeQueue(const ThreadSafeQueue &) = delete;	//!< forbidden copy constructor
	ThreadSafeQueue & operator=(const ThreadSafeQueue &) = delete;	//!< forbidden assignment

	std::queue < T > m_queue;
	std::mutex m_mutex;
	std::condition_variable m_cond;

 public:
	ThreadSafeQueue() = default;
	~ThreadSafeQueue() = default;

	void Push(const T & item) {
		std::unique_lock < std::mutex > q_lock(m_mutex);
		m_queue.push(item);
		q_lock.unlock();	// minimize mutex contention
		m_cond.notify_one();	// notify one waiting thread
	}

	T Pop() {
		std::unique_lock < std::mutex > q_lock(m_mutex);
		while (m_queue.empty()) {
			m_cond.wait(q_lock);
		}
		auto item = m_queue.front();
		m_queue.pop();
		return item;
	}

	bool IsEmpty() {
		std::unique_lock < std::mutex > q_lock(m_mutex);
		bool is_empty = m_queue.empty();
		q_lock.unlock();	// minimize mutex contention
		return (is_empty);
	}

	int GetCount() {
		std::unique_lock < std::mutex > q_lock(m_mutex);
		int count = m_queue.size();
		q_lock.unlock();	// minimize mutex contention
		return (count);
	}
};

#endif

// vim: ts=4
