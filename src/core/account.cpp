//
// Created by int3 on 25.11.17.
//

#include <purple++/core/account.h>
#include <purple++/core/connection.h>
#include <purple++/core/status.h>
#include <purple++/core/blist.h>
#include <purple++/core/saved_status.h>
#include <purple++/core/conversation.h>
#include <purple++/library.h>

#include <libpurple/account.h>
#include <libpurple/savedstatuses.h>
#include <fmt/ostream.h>
#include <cassert>

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

void account::set_status(status_primitive status) {
	assert(_impl.get());

	// TODO: fix it
	purple_savedstatus_activate_for_account(purple_savedstatus_new(NULL, static_cast<PurpleStatusPrimitive>(status)), _impl.get());

	/*
	auto c_status = static_cast<PurpleStatusPrimitive>(status);
	auto status_id = purple_primitive_get_id_from_type(c_status);
	purple_account_set_status(_impl.get(), status_id, TRUE, nullptr);
	*/
}

connection& account::get_connection() {
	return *_connection;
}

boost::string_view account::get_username() {
	return detail::to_view(_impl->username);
}

boost::string_view account::get_protocol() const {
	return detail::to_view(_impl->protocol_id);
}

bool account::is_connected() const {
	return purple_account_is_connected(_impl.get()) != 0;
}

void account::apply_to_conversation(boost::string_view name, callback_t<conversation_ptr> cb) const {
	library::add_task([=, name = name.to_string()]{
		PURPLEPP_ASSERT_IS_LOOP_THREAD();

		auto conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_ANY, name.c_str(), _impl.get());
		if (conv != nullptr) {
			cb(account::_create_conversation(conv));
		}
		else {
			fmt::print(stderr, "conversation {} not found..\n", name);
		}
	});
}

void account::apply_to_conversations(callback_t<std::vector<conversation_ptr>&> cb) const {
	library::add_task([=]{
		PURPLEPP_ASSERT_IS_LOOP_THREAD();

		std::vector<std::unique_ptr<conversation>> conversations;

		auto collector = [&](std::unique_ptr<conversation> conv) {
			conversations.push_back(std::move(conv));
		};

		{
			std::lock_guard<std::mutex> _(_any_data_mutex);
			_any_data = (void*)&collector;

			purple_conversation_foreach([](PurpleConversation* impl) {
				auto conv = account::_create_conversation(impl);
				assert(conv.get() != nullptr);

				auto acc = conv->get_account();
				assert(acc);

				auto& coll_ptr = *reinterpret_cast<decltype(collector)*>(acc->_any_data);
				coll_ptr(std::move(conv));
			});
		}

		cb(conversations);
	});
}

void account::apply_to_buddy(boost::string_view name, callback_t<buddy&> cb) {
	library::add_task([=, name = name.to_string()]{
		PURPLEPP_ASSERT_IS_LOOP_THREAD();

		if (name.empty()) return;

		auto buddyptr = purple_find_buddy(_impl.get(), name.c_str());
		if (!buddyptr) return;

		buddy wrapper{ buddyptr };
		cb(wrapper);
	});
}

_PurpleAccount* account::_get_impl() const {
	return _impl.get();
}

account* account::_get_wrapper(_PurpleAccount* impl) {
	assert(impl != nullptr);

	return (account*)impl->ui_data;
}

std::unique_ptr<conversation> account::_create_conversation(_PurpleConversation* impl) {
	detail::thread_local_cache<PurpleConversation*, conversation>::set(impl);
	return account::_get_wrapper(impl->account)->_conv_factory();
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