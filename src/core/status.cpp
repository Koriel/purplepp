//
// Created by int3 on 25.11.17.
//

#include <purple++/core/status.h>
#include <glib.h> // for gpointer in status.h:93:2
#include <libpurple/status.h>

namespace purplepp {

status_primitive status_primitive::unset(PURPLE_STATUS_UNSET);
status_primitive status_primitive::offline(PURPLE_STATUS_OFFLINE);
status_primitive status_primitive::available(PURPLE_STATUS_AVAILABLE);
status_primitive status_primitive::unavailable(PURPLE_STATUS_UNAVAILABLE);
status_primitive status_primitive::invisible(PURPLE_STATUS_INVISIBLE);
status_primitive status_primitive::away(PURPLE_STATUS_AWAY);
status_primitive status_primitive::extended_away(PURPLE_STATUS_EXTENDED_AWAY);
status_primitive status_primitive::mobile(PURPLE_STATUS_MOBILE);
status_primitive status_primitive::tune(PURPLE_STATUS_TUNE);
status_primitive status_primitive::mood(PURPLE_STATUS_MOOD);
status_primitive status_primitive::NUMBER(PURPLE_STATUS_NUM_PRIMITIVES);

uint8_t status_primitive::_get_value() const {
	return _value;
}

std::ostream& operator<<(std::ostream& os, status_primitive prim) {
	switch (prim._get_value()) {
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
		default: return os << "<unknown(" << prim._get_value() << ")>";
	}
}

/* status_type */
status_type::status_type(_PurpleStatusType* impl) : _impl(impl) {

}

status_primitive status_type::get_primitive() const {
	return static_cast<uint8_t>(purple_status_type_get_primitive(_impl));
}

/* status */

status::status(_PurpleStatus* impl) : _impl(impl) {

}

status_type status::get_type() const {
	return purple_status_get_type(_impl);
}


}