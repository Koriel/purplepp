//
// Created by int3 on 25.11.17.
//

#pragma once

#include <boost/utility/string_view.hpp>
#include <memory>
#include <thread>
#include <cassert>

#define PURPLEPP_NON_COPYABLE(cls) \
	cls(const cls&) = delete; \
	cls& operator=(const cls&) = delete

#define PURPLEPP_NON_MOVABLE(cls) \
	cls(cls&&) = delete; \
	cls& operator=(cls&&) = delete

#define PURPLEPP_NON_COPYABLE_NON_MOVABLE(cls) \
	PURPLEPP_NON_COPYABLE(cls); \
	PURPLEPP_NON_MOVABLE(cls);

#define PURPLEPP_STATIC_CLASS(cls) \
	cls() = delete; \
	~cls() = delete; \
	PURPLEPP_NON_COPYABLE_NON_MOVABLE(cls)


/**
 * Any access to purple structures without ownership should be performed only from libpurple main thread.
 * If you use or save pointer to purple object in another thread, you get UB because libpurple might delete this object without notice.
 *
 * This macro provides checkout of such a condition and should be used only internally.
 * */
#ifdef _NDEBUG
#define PURPLEPP_ASSERT_IS_LOOP_THREAD() (void)0
#else
#define PURPLEPP_ASSERT_IS_LOOP_THREAD() assert(purplepp::detail::get_loop_thread_id() == std::thread::id{} || purplepp::detail::get_loop_thread_id() == std::this_thread::get_id())
#endif

namespace purplepp {

namespace detail {

template <class T, class Tag>
class thread_local_cache {
	static T& _get_ref() {
		thread_local T cache;
		return cache;
	}
public:
	static void set(T value) {
		_get_ref() = value;
	}

	static T get() {
		return _get_ref();
	}
};

#ifndef _NDEBUG
std::thread::id get_loop_thread_id();
#endif

boost::string_view to_view(const char* ptr);

}

// owning implementation
template <class Impl>
using impl_ptr = std::unique_ptr<Impl, void(*)(typename std::add_pointer<Impl>::type)>;

template <class Impl>
impl_ptr<Impl> make_impl_ptr(Impl* impl, void(*deleter)(Impl*)) {
	return impl_ptr<Impl>(impl, deleter);
}

#define PURPLEPP_NON_OWNING_WRAPPER(Wrapper, Impl) \
private: \
	Impl* _impl; \
public: \
	Wrapper(Impl* impl) : _impl(impl) {} \
	Impl* _get_impl() const noexcept { \
		PURPLEPP_ASSERT_IS_LOOP_THREAD();\
		return _impl;\
	}


}