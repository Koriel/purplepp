//
// Created by int3 on 25.11.17.
//

#pragma once

#include <memory>
#include <boost/utility/string_view.hpp>

namespace purplepp {

class account;

class library {
	class impl;
	std::unique_ptr<impl> _impl;

public:
	library();
	~library() noexcept;

	void init();
	void add_account(std::unique_ptr<account> account);
	void run_loop();
	static void add_task(std::function<void()> task);

	void set_debug(bool enabled);

	library(const library&) = delete;
	library& operator=(const library&) = delete;

	library(library&&) = default;
	library& operator=(library&&) = default;
};

}