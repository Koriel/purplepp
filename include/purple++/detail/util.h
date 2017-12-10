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

template <class E>
auto to_underlying(E e) -> typename std::underlying_type<E>::type {
	return static_cast<typename std::underlying_type<E>::type>(e);
}

/*template <class Obj, class F, class Tuple, std::size_t... I>
constexpr decltype(auto) apply_method_impl(Obj&& o, F&& f, Tuple&& t, std::index_sequence<I...> )
{
	return (std::forward<Obj>(o).*std::forward<F>(f))(std::get<I>(std::forward<Tuple>(t))...);
}*/

}

/*template <class Obj, class F, class Tuple>
constexpr decltype(auto) apply_method(Obj&& o, F&& f, Tuple&& t)
{
	return detail::apply_method_impl(std::forward<Obj>(o), std::forward<F>(f), std::forward<Tuple>(t),
							 std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{});
}*/

// owning implementation
template <class Impl>
using impl_ptr = std::unique_ptr<Impl, void(*)(typename std::add_pointer<Impl>::type)>;

template <class Impl>
impl_ptr<Impl> make_impl_ptr(Impl* impl, void(*deleter)(Impl*)) {
	return impl_ptr<Impl>(impl, deleter);
}

template <class... Args>
using callback_t = std::function<void(Args...)>;

#define PURPLEPP_NON_OWNING_WRAPPER(Wrapper, Impl) \
private: \
	Impl* _impl; \
public: \
	Wrapper(Impl* impl) : _impl(impl) {} \
	Impl* _get_impl() const noexcept { \
		PURPLEPP_ASSERT_IS_LOOP_THREAD();\
		return _impl;\
	}


#define PURPLEPP_ENUM_ASSERT_CORRECT(enumclass, cenum) static_assert(static_cast<std::underlying_type<decltype(enumclass)>::type>(enumclass) == cenum, "Check it!")

/*
#define PURPLEPP_DETAIL_FWD(x) std::forward<decltype(x)>(x)
#define PURPLEPP_DETAIL_GENERATE_LAMBDA_BODY(...) noexcept(noexcept(__VA_ARGS__)) -> decltype(__VA_ARGS__) { return __VA_ARGS__; }


	noexcept(noexcept(apply_method(PURPLEPP_DETAIL_FWD(obj), &decltype(obj)::method, PURPLEPP_DETAIL_FWD(args_tuple)))) \
            -> decltype(apply_method(PURPLEPP_DETAIL_FWD(obj), &decltype(obj)::method, PURPLEPP_DETAIL_FWD(args_tuple))) { \
                return apply_method(PURPLEPP_DETAIL_FWD(obj), &decltype(obj)::method, PURPLEPP_DETAIL_FWD(args_tuple)); \
            }; \
    }*/


}