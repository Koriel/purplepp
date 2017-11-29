//
// Created by int3 on 25.11.17.
//

#pragma once

#include <purple++/core/status.h>

struct _PurpleSavedStatus;

namespace purplepp {

class saved_status {
	impl_ptr<_PurpleSavedStatus> _impl;

public:
	saved_status(boost::string_view title, status_primitive type);
	void set_message(boost::string_view message);
	void set_type(status_primitive type);
	void activate();
};

}