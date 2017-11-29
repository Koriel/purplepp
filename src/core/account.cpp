//
// Created by int3 on 25.11.17.
//

#include <purple++/core/account.h>
#include <purple++/core/connection.h>
#include <purple++/core/status.h>
#include <libpurple/account.h>
#include <cassert>
#include <purple++/core/conversation.h>
#include <fmt/ostream.h>

namespace purplepp {

account::account(boost::string_view username, boost::string_view protocol_id) :
		_impl(purple_account_new(username.to_string().c_str(), protocol_id.to_string().c_str()), &purple_account_destroy)
{
	_impl->ui_data = this;
}

void account::set_connection(std::unique_ptr<connection> connection) {
	_connection = std::move(connection);
}

void account::set_factory(std::function<conversation*()> conv_factory) {
	_conv_factory = std::move(conv_factory);
}

void account::set_password(boost::string_view password) {
	purple_account_set_password(_impl.get(), password.to_string().c_str());
}

void account::set_remember_password(bool value) {
	purple_account_set_remember_password(_impl.get(), value ? 1 : 0);
}

void account::set_enabled(boost::string_view ui, bool value) {
	purple_account_set_enabled(_impl.get(), ui.to_string().c_str(), value ? 1 : 0);
}

void account::connect() {
	purple_account_connect(_impl.get());
}

connection& account::get_connection() {
	return *_connection;
}

boost::string_view account::get_username() {
	return _impl->username;
}

_PurpleAccount* account::_get_impl() const {
	return _impl.get();
}

account* account::_get_wrapper(_PurpleAccount* impl) {
	return (account*)impl->ui_data;
}

void account::_create_conversation(_PurpleConversation* impl) {
	detail::thread_local_cache<PurpleConversation*, conversation>::set(impl);

	// returned pointer is automatically saved into conv->ui_data, so no need to save it somewhere
	_conversations.insert(_conv_factory());
}

void account::_destroy_conversation(_PurpleConversation* impl) {
	auto conv = (conversation*)impl->ui_data;
	_conversations.erase(conv);
	delete conv;
	impl->ui_data = nullptr;
}

void simple_account::notify_added(const char* remote_user, const char* id, const char* alias, const char* message) {
	fmt::print("notify_added [{}, {}, {}, {}]\n", remote_user, id, alias, message);
}

void simple_account::status_changed(const status& status) {
	fmt::print("status_changed [{}]\n", status.get_type().get_primitive());
}

void simple_account::request_add(const char* remote_user, const char* id, const char* alias, const char* message) {
	fmt::print("request_add [{}, {}, {}, {}]\n", remote_user, id, alias, message);
}

void* simple_account::request_authorize(const char* remote_user, const char* id, const char* alias, const char* message, bool on_list, void* user_data) {
	fmt::print("request_authorize [{}, {}, {}, {}, {}]\n", remote_user, id, alias, message, on_list ? "true" : "false");
	return nullptr;
}

void simple_account::close_account_request(void* ui_handle) {
	fmt::print("close_account_request\n");
}

}