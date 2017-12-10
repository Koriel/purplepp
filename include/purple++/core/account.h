//
// Created by int3 on 25.11.17.
//

#pragma once

#include <purple++/detail/util.h>
#include <purple++/core/status.h>

#include <unordered_set>
#include <vector>
#include <mutex>

struct _PurpleAccount;
struct _PurpleConversation;

namespace purplepp {

class connection;
class status;
class conversation;
class buddy;

using conversation_ptr = std::unique_ptr<conversation>;

class account {
	// members
	impl_ptr<_PurpleAccount> _impl;
	std::unique_ptr<connection> _connection;
	std::function<std::unique_ptr<conversation>()> _conv_factory;

	// TODO: make thread-safe (weak_ptr)
	// TODO: make map: name -> conversation
	//std::unordered_set<conversation*> _conversations;

	mutable std::mutex _any_data_mutex;
	mutable void* _any_data;

	void set_connection(std::unique_ptr<connection> connection);
	void set_factory(std::function<std::unique_ptr<conversation>()> conv_factory);
public:
	explicit account(boost::string_view username, boost::string_view protocol_id);
	virtual ~account() noexcept = default;

	// base C-API
	void set_password(boost::string_view password);
	void set_remember_password(bool value);
	void set_enabled(boost::string_view ui, bool value);
	void connect();
	void set_status(status_primitive status);

	// base methods
	connection& get_connection();
	boost::string_view get_username();
	boost::string_view get_protocol() const;

	bool is_connected() const;

	void apply_to_conversation(boost::string_view name, callback_t<conversation_ptr> cb) const;
	void apply_to_conversations(callback_t<std::vector<conversation_ptr>&> cb) const;
	void apply_to_buddy(boost::string_view name, callback_t<buddy&> cb);

	// for C-wrapping
	_PurpleAccount* _get_impl() const;
	static account* _get_wrapper(_PurpleAccount* impl);
	static std::unique_ptr<conversation> _create_conversation(_PurpleConversation* impl);
	//void _destroy_conversation(_PurpleConversation* impl);

	// factory method
	template <class Account, class Connection, class Conversation, class... Args> friend std::unique_ptr<account> make_account(Args&& ...args);

	// ui_ops
	/** A buddy who is already on this account's buddy list added this account
	 *  to their buddy list.
	 */
	virtual void notify_added(const char *remote_user, const char *id, const char *alias, const char *message) {};

	/** This account's status changed. */
	virtual void status_changed(const status& status) {};

	/** Someone we don't have on our list added us; prompt to add them. */
	virtual void request_add(const char *remote_user, const char *id, const char *alias, const char *message) {};

	/** Prompt for authorization when someone adds this account to their buddy
	 * list.  To authorize them to see this account's presence, call \a
	 * authorize_cb (\a user_data); otherwise call \a deny_cb (\a user_data);
	 * @return a UI-specific handle, as passed to #close_account_request.
	 */
	virtual void* request_authorize(const char *remote_user, const char *id, const char *alias, const char *message, bool on_list,
							   //PurpleAccountRequestAuthorizationCb authorize_cb,
							   //PurpleAccountRequestAuthorizationCb deny_cb,
							   void *user_data
	) { return nullptr; };

	/** Close a pending request for authorization.  \a ui_handle is a handle
	 *  as returned by #request_authorize.
	 */
	virtual void close_account_request(void *ui_handle) {};

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
std::unique_ptr<account> make_account(Args&& ...args) {
	// check invariants
	static_assert(std::is_base_of<account, Account>::value, "Account must be derived from purple::account");
	static_assert(std::is_base_of<connection, Connection>::value, "Connection must be derived from purple::connection");
	static_assert(std::is_base_of<conversation, Conversation>::value, "Conversation must be derived from purple::conversation");

	auto ret = std::unique_ptr<account>(new Account(std::forward<Args>(args)...));
	ret->set_connection(std::unique_ptr<Connection>(new Connection(*ret)));
	ret->set_factory([](){
		return std::unique_ptr<Conversation>(new Conversation);
	});
	return ret;
}

}