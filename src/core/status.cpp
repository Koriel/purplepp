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