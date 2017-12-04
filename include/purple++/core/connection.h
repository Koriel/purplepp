//
// Created by int3 on 25.11.17.
//

#pragma once

#include <purple++/detail/util.h>

struct _PurpleConnection;

namespace purplepp {

class account;

class connection {
protected:
	using signed_on_callback_t = std::function<void(connection&)>;

	_PurpleConnection* _impl = nullptr;

	// ordinary members
	account& _acc;

	signed_on_callback_t _signed_on_cb;
public:
	explicit connection(account& acc);
	virtual ~connection() noexcept = default;

	account& get_account();

	// callbacks setters
	void on_signed_on(signed_on_callback_t&& cb);

	// internal
	static connection& _get_wrapper(_PurpleConnection* impl);
	_PurpleConnection* _get_impl() const;

	// signals
	void _trigger_signed_on();

	// ui_ops
	virtual void on_connect_progress(boost::string_view text, size_t step, size_t step_count) = 0;
	virtual void on_connected() = 0;
	virtual void on_disconnected() = 0;
	virtual void on_notice(boost::string_view text) = 0;
	virtual void on_report_disconnect(boost::string_view text) = 0;
	//virtual void on_network_connected() = 0;
	//virtual void on_network_disconnected() = 0;
	virtual void on_report_disconnect_reason(int reason, boost::string_view text) = 0;

	PURPLEPP_NON_COPYABLE_NON_MOVABLE(connection);
};

class simple_connection : public connection {
	using connection::connection;

public:
	void on_connect_progress(boost::string_view text, size_t step, size_t step_count) override;
	void on_connected() override;
	void on_disconnected() override;
	void on_notice(boost::string_view text) override;
	void on_report_disconnect(boost::string_view text) override;
	void on_report_disconnect_reason(int reason, boost::string_view text) override;
};

}
