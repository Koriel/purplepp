//
// Created by int3 on 25.11.17.
//

#include <purple++/core/saved_status.h>
#include <glib.h>
#include <libpurple/savedstatuses.h>

namespace purplepp {

saved_status::saved_status(boost::string_view title, status_primitive type)
		: _impl(purple_savedstatus_new(title.to_string().c_str(), static_cast<PurpleStatusPrimitive>(type)), purple_savedstatus_delete_by_status)
{

}

void saved_status::set_message(boost::string_view message) {
	purple_savedstatus_set_message(_impl.get(), message.to_string().c_str());
}

void saved_status::set_type(status_primitive type) {
	purple_savedstatus_set_type(_impl.get(), static_cast<PurpleStatusPrimitive>(type));
}

void saved_status::activate() {
	purple_savedstatus_activate(_impl.get());
}

}