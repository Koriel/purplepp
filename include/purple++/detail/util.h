//
// Created by int3 on 25.11.17.
//

#pragma once

#include <boost/utility/string_view.hpp>
#include <memory>

#define PURPLEPP_NON_COPYABLE_NON_MOVABLE(cls) \
	cls(const cls&) = delete; \
	cls& operator=(const cls&) = delete; \
	cls(cls&&) = delete; \
	cls& operator=(cls&&) = delete;

#define PURPLEPP_NON_COPYABLE(cls) \
	cls(const cls&) = delete; \
	cls& operator=(const cls&) = delete;

#define PURPLEPP_STATIC_CLASS(cls) \
	cls() = delete; \
	~cls() = delete; \
	PURPLEPP_NON_COPYABLE_NON_MOVABLE(cls)

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

}

template <class Impl>
using impl_ptr = std::unique_ptr<Impl, void(*)(typename std::add_pointer<Impl>::type)>;

template <class Impl>
impl_ptr<Impl> make_impl_ptr(Impl* impl, void(*deleter)(Impl*)) {
	return impl_ptr<Impl>(impl, deleter);
}

}