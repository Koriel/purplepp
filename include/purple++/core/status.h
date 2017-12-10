//
// Created by int3 on 25.11.17.
//

#pragma once

#include <purple++/detail/util.h>

struct _PurpleStatus;
struct _PurpleStatusType;
struct _PurplePresence;

namespace purplepp {

// instead of enum
enum class status_primitive : uint8_t {
	unset = 0,
	offline,
	available,
	unavailable,
	invisible,
	away,
	extended_away,
	mobile,
	tune,
	mood,
	NUMBER
};

std::ostream& operator<< (std::ostream& os, status_primitive prim);

class status_type {
	PURPLEPP_NON_OWNING_WRAPPER(status_type, _PurpleStatusType);

	status_primitive get_primitive() const;
};

class status {
	PURPLEPP_NON_OWNING_WRAPPER(status, _PurpleStatus);

	status_type get_type() const;
	status_primitive get_primitive() const;
};

class presence {
	PURPLEPP_NON_OWNING_WRAPPER(presence, _PurplePresence);

	status get_active_status() const;
};

}