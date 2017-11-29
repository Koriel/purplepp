//
// Created by int3 on 29.11.17.
//

#include <purple++/core/conversation.h>
#include <purple++/detail/util.h>

#include <libpurple/conversation.h>

#include <cassert>

namespace purplepp {

conversation::conversation() {
	_impl = detail::thread_local_cache<PurpleConversation*, conversation>::get();
	_impl->ui_data = this;
}

boost::string_view conversation::get_name() const {
	return purple_conversation_get_name(_impl);
}

conversation* conversation::_get_wrapper(_PurpleConversation* impl) {
	assert(impl->ui_data);
	return (conversation*)impl->ui_data;
}

void simple_conversation::write_conv(boost::string_view who, boost::string_view alias, boost::string_view message, time_t mtime) {
	std::string name = alias.empty() ? who.empty() ? "<empty>" : who.to_string() : alias.to_string();

	printf("write_conv: (%s) %s %s: %s\n", get_name().to_string().c_str(),
		   purple_utf8_strftime("(%H:%M:%S)", localtime(&mtime)),
		   name.c_str(), message);
}

}