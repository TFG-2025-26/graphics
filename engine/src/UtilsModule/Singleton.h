// This file is part of the course TPV2@UCM - Samir Genaim

#pragma once

#ifndef SINGLETON_H_
#define SINGLETON_H_

#include <memory>
#include <cassert>

#include "defs.h"

/*
 * This is an attempt to have a single Singleton class that can be used
 * via inheritance to make some other class Singleton. It is just to avoid
 * copying the Singleton declarations each time.
 *
 * >>> Requirements:
 *
 * The class that inherits this class must implement a constructor
 * with no arguments, this is because of the call 'init()' in method
 * 'instance()'. In the case that defining such a constructor is against
 * your design, just define one that throws and exception.
 *
 * >>> Usage:
 *
 * class A : public Singleton<A> {
 *
 *    friend Singleton<A>; // this way Singleton can call the constructor of A
 *
 * private: // constructors are private
 *    A() {
 *      //....
 *    }
 *
 * public: // the rest of the functionality
 *    virtual ~A() {
 *    }
 * }
 *
 */


namespace flux_utils {
	template<typename T>
	class Singleton {
	protected:
		FLUX_API Singleton() {
		}

	public:

		// cannot copy objects of this type
		FLUX_API Singleton<T>& operator=(const Singleton<T>& o) = delete;
		FLUX_API Singleton(const Singleton<T>& o) = delete;

		FLUX_API virtual ~Singleton() {
		}

		// some singletons need to be initialised with some parameters, we
		// can call this method at the beginning of the program.
		template<typename ...Targs>
		FLUX_API inline static T* init(Targs &&...args) {
			assert(instance_.get() == nullptr);
			instance_.reset(new T(std::forward<Targs>(args)...));
			return instance_.get();
		}

		// in some cases, when singletons depend on each other, you have
		// to close them in a specific order, This is why we have this close
		// method
		FLUX_API inline static void close() {
			instance_.reset();
		}

		// get the singleton instance as a pointer
		//
		FLUX_API inline static T* instance() {
			// you can replace the "if" by assert(instance_.get() != nullptr)
			// to force initialisation at the beginning
			//
			if (instance_.get() == nullptr) {
				init();
			}
			return instance_.get();
		}

	private:
		static std::unique_ptr<T> instance_;
	};

	template<typename T>
	std::unique_ptr<T> Singleton<T>::instance_;
}

#endif // SINGLETON_H_