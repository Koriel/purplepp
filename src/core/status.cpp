//
// Created by int3 on 25.11.17.
//

#include <purple++/core/status.h>
#include <glib.h> // for gpointer in status.h:93:2
#include <libpurple/status.h>

namespace purplepp {

PURPLEPP_ENUM_ASSERT_CORRECT(status_primitive::unset, PURPLE_STATUS_UNSET);
PURPLEPP_ENUM_ASSERT_CORRECT(status_primitive::offline, PURPLE_STATUS_OFFLINE);
PURPLEPP_ENUM_ASSERT_CORRECT(status_primitive::available, PURPLE_STATUS_AVAILABLE);
PURPLEPP_ENUM_ASSERT_CORRECT(status_primitive::unavailable, PURPLE_STATUS_UNAVAILABLE);
PURPLEPP_ENUM_ASSERT_CORRECT(status_primitive::invisible, PURPLE_STATUS_INVISIBLE);
PURPLEPP_ENUM_ASSERT_CORRECT(status_primitive::away, PURPLE_STATUS_AWAY);
PURPLEPP_ENUM_ASSERT_CORRECT(status_primitive::extended_away, PURPLE_STATUS_EXTENDED_AWAY);
PURPLEPP_ENUM_ASSERT_CORRECT(status_primitive::mobile, PURPLE_STATUS_MOBILE);
PURPLEPP_ENUM_ASSERT_CORRECT(status_primitive::tune, PURPLE_STATUS_TUNE);
PURPLEPP_ENUM_ASSERT_CORRECT(status_primitive::mood, PURPLE_STATUS_MOOD);
PURPLEPP_ENUM_ASSERT_CORRECT(status_primitive::NUMBER, PURPLE_STATUS_NUM_PRIMITIVES);

std::ostream& operator<<(std::ostream& os, status_primitive prim) {
	switch (detail::to_underlying(prim)) {
		case PURPLE_STATUS_UNSET: return os << "unset";
		case PURPLE_STATUS_OFFLINE: return os << "offline";
		case PURPLE_STATUS_AVAILABLE: return os << "available";
		case PURPLE_STATUS_UNAVAILABLE: return os << "unavailable";
		case PURPLE_STATUS_INVISIBLE: return os << "invisible";
		case PURPLE_STATUS_AWAY: return os << "away";
		case PURPLE_STATUS_EXTENDED_AWAY: return os << "extended_away";
		case PURPLE_STATUS_MOBILE: return os << "mobile";
		case PURPLE_STATUS_TUNE: return os << "tune";
		case PURPLE_STATUS_MOOD: return os << "mood";
		default: return os << "<unknown(" << detail::to_underlying(prim) << ")>";
	}
}

/** status_type */

status_primitive status_type::get_primitive() const {
	return static_cast<status_primitive>(purple_status_type_get_primitive(_impl));
}

/** status */

status_type status::get_type() const {
	return purple_status_get_type(_impl);
}

status_primitive status::get_primitive() const {
	return get_type().get_primitive();
}

/** presence */
status presence::get_active_status() const {
	return { purple_presence_get_active_status(_impl) };
}

}