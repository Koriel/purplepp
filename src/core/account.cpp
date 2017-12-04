//
// Created by int3 on 25.11.17.
//

#include <purple++/core/account.h>
#include <purple++/core/connection.h>
#include <purple++/core/status.h>
#include <purple++/library.h>
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

void account::set_factory(std::function<std::unique_ptr<conversation>()> conv_factory) {
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
	assert(_impl.get());

	purple_account_connect(_impl.get());
}

connection& account::get_connection() {
	return *_connection;
}

boost::string_view account::get_username() {
	return _impl->username;
}

bool account::is_connected() const {
	return purple_account_is_connected(_impl.get()) != 0;
}

void account::apply_to_conversation(boost::string_view name, std::function<void(conversation&)> cb) const {
	//purple_prpl_send_attention(_connection->_get_impl(), name.to_string().c_str(), 0);

	library::add_task([=](){
		PURPLEPP_ASSERT_IS_LOOP_THREAD();

		fmt::print(stderr, "Executing task apply_to_conversation...\n");

		//cb(*account::_create_conversation(purple_conversation_new(PURPLE_CONV_TYPE_IM, _impl.get(), name.to_string().c_str())));

		auto conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_ANY, name.to_string().c_str(), _impl.get());
		if (conv != nullptr) {
			cb(*account::_create_conversation(conv));
		}
	});
}

_PurpleAccount* account::_get_impl() const {
	return _impl.get();
}

account* account::_get_wrapper(_PurpleAccount* impl) {
	return (account*)impl->ui_data;
}

std::unique_ptr<conversation> account::_create_conversation(_PurpleConversation* impl) {
	detail::thread_local_cache<PurpleConversation*, conversation>::set(impl);
	auto acc = account::_get_wrapper(impl->account);
	return acc->_conv_factory();
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