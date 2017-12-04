//
// Created by int3 on 25.11.17.
//

#include <purple++/detail/util.h>

#ifndef _NDEBUG
#include <purple++/library.h>
#endif

namespace purplepp {

namespace detail {

#ifndef _NDEBUG
std::thread::id get_loop_thread_id() {
	return library::get_loop_thread_id();
}
#endif

boost::string_view to_view(const char* ptr) {
	return ptr ? ptr : boost::string_view{};
}

}

}