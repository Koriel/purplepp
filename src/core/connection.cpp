//
// Created by int3 on 25.11.17.
//

#include <purple++/core/connection.h>
#include <purple++/core/account.h>

#include <libpurple/connection.h>
#include <libpurple/account.h>

#include <fmt/printf.h>
#include <fmt/ostream.h>

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
	fmt::print("on_connect_progress: {}, {}/{}\n", text, step, step_count);
}

void simple_connection::on_connected() {
	fmt::print("on_connected\n");
}

void simple_connection::on_disconnected() {
	fmt::print("on_disconnected\n");
}

void simple_connection::on_notice(boost::string_view text) {
	fmt::print("on_notice [{}]\n", text);
}

void simple_connection::on_report_disconnect(boost::string_view text) {
	fmt::print("on_report_disconnect [{}]\n", text);
}

void simple_connection::on_report_disconnect_reason(int reason, boost::string_view text) {
	fmt::print("on_report_disconnect_reason: [{}, {}]\n", reason, text);
}

}