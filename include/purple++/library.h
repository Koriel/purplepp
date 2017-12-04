//
// Created by int3 on 25.11.17.
//

#pragma once

#include <memory>
#include <boost/utility/string_view.hpp>
#include <purple++/detail/util.h>

namespace purplepp {

class account;

struct library {
public:
	static void init();
	static void add_account(std::unique_ptr<account> account);
	static void run_loop();
	static void stop_loop();
	static void wait_loop();
	static void add_task(std::function<void()> task);

	static void set_debug(bool enabled);

	static account* find_account(boost::string_view login);

	static std::thread::id get_loop_thread_id();

	PURPLEPP_STATIC_CLASS(library);
};

}