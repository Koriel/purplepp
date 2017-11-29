//
// Created by int3 on 25.11.17.
//

#pragma once

#include <purple++/detail/util.h>
#include <unordered_set>

struct _PurpleAccount;
struct _PurpleConversation;

namespace purplepp {

class connection;
class status;
class conversation;

class account {
	// members
	impl_ptr<_PurpleAccount> _impl;
	std::unique_ptr<connection> _connection;
	std::function<conversation*()> _conv_factory;
	std::unordered_set<conversation*> _conversations;

	void set_connection(std::unique_ptr<connection> connection);
	void set_factory(std::function<conversation*()> conv_factory);
public:
	explicit account(boost::string_view username, boost::string_view protocol_id);
	virtual ~account() noexcept = default;

	// base C-API
	void set_password(boost::string_view password);
	void set_remember_password(bool value);
	void set_enabled(boost::string_view ui, bool value);
	void connect();

	// base methods
	connection& get_connection();
	boost::string_view get_username();

	// for C-wrapping
	_PurpleAccount* _get_impl() const;
	static account* _get_wrapper(_PurpleAccount* impl);
	void _create_conversation(_PurpleConversation* impl);
	void _destroy_conversation(_PurpleConversation* impl);

	// factory method
	template <class Account, class Connection, class Conversation, class... Args> friend std::unique_ptr<account> make_account(Args&& ...args);

	// ui_ops
	/** A buddy who is already on this account's buddy list added this account
	 *  to their buddy list.
	 */
	virtual void notify_added(const char *remote_user, const char *id, const char *alias, const char *message) = 0;

	/** This account's status changed. */
	virtual void status_changed(const status& status) = 0;

	/** Someone we don't have on our list added us; prompt to add them. */
	virtual void request_add(const char *remote_user, const char *id, const char *alias, const char *message) = 0;

	/** Prompt for authorization when someone adds this account to their buddy
	 * list.  To authorize them to see this account's presence, call \a
	 * authorize_cb (\a user_data); otherwise call \a deny_cb (\a user_data);
	 * @return a UI-specific handle, as passed to #close_account_request.
	 */
	virtual void* request_authorize(const char *remote_user, const char *id, const char *alias, const char *message, bool on_list,
							   //PurpleAccountRequestAuthorizationCb authorize_cb,
							   //PurpleAccountRequestAuthorizationCb deny_cb,
							   void *user_data
	) = 0;

	/** Close a pending request for authorization.  \a ui_handle is a handle
	 *  as returned by #request_authorize.
	 */
	virtual void close_account_request(void *ui_handle) = 0;

	PURPLEPP_NON_COPYABLE_NON_MOVABLE(account);
};

class simple_account : public account {
public:
	using account::account;

	void notify_added(const char* remote_user, const char* id, const char* alias, const char* message) override;
	void status_changed(const status& status) override;
	void request_add(const char* remote_user, const char* id, const char* alias, const char* message) override;
	void* request_authorize(const char* remote_user, const char* id, const char* alias, const char* message, bool on_list, void* user_data) override;
	void close_account_request(void* ui_handle) override;
};

template <class Account, class Connection, class Conversation, class... Args>
static std::unique_ptr<account> make_account(Args&& ...args) {
	// check invariants
	static_assert(std::is_base_of<account, Account>::value, "Account must be derived from purple::account");
	static_assert(std::is_base_of<connection, Connection>::value, "Connection must be derived from purple::connection");
	static_assert(std::is_base_of<conversation, Conversation>::value, "Conversation must be derived from purple::conversation");

	auto ret = std::unique_ptr<account>(new Account(std::forward<Args>(args)...));
	ret->set_connection(std::unique_ptr<Connection>(new Connection(*ret)));
	ret->set_factory([](){
		return new Conversation;
	});
	return ret;
}

}