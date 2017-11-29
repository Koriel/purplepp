//
// Created by int3 on 25.11.17.
//

#include <purple++/core/connection.h>
#include <purple++/core/account.h>

#include <libpurple/connection.h>
#include <libpurple/account.h>

namespace purplepp {

connection::connection(account& acc) : _impl(nullptr), _acc(acc) {

}

account& connection::get_account() {
	return _acc;
}

void connection::on_signed_on(connection::signed_on_callback_t&& cb) {
	_signed_on_cb = std::move(cb);
}

connection& connection::_get_wrapper(_PurpleConnection* impl) {
	auto& conn = account::_get_wrapper(impl->account)->get_connection();
	if (conn._impl == nullptr) conn._impl = impl;
	return conn;
}

void connection::_trigger_signed_on() {
	if (_signed_on_cb) {
		_signed_on_cb(*this);
	}
}


void simple_connection::on_connect_progress(boost::string_view text, size_t step, size_t step_count) {
	printf("on_connect_progress: %s, %zu/%zu\n", text.to_string().c_str(), step, step_count);
}

void simple_connection::on_connected() {
	printf("on_connected\n");
}

void simple_connection::on_disconnected() {
	printf("on_disconnected\n");
}

void simple_connection::on_notice(boost::string_view text) {
	printf("on_notice [%s]\n", text.to_string().c_str());
}

void simple_connection::on_report_disconnect(boost::string_view text) {
	printf("on_report_disconnect [%s]\n", text.to_string().c_str());
}

void simple_connection::on_report_disconnect_reason(int reason, boost::string_view text) {
	printf("on_report_disconnect_reason: [%d, %s]\n", reason, text.to_string().c_str());
}

}