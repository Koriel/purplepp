//
// Created by int3 on 02.12.17.
//

#pragma once

#include <purple++/core/status.h>
#include <purple++/detail/util.h>
#include <boost/variant.hpp>

struct _PurpleBlistNode;
struct _PurpleBuddy;
struct _PurpleGroup;
struct _PurpleContact;
struct _PurpleChat;

namespace purplepp {

class account;

/** PurpleBlistNodeType */
class blist_node_type {
	uint8_t _value;

public:
	/* implicit */ blist_node_type(uint8_t value);

	static const blist_node_type group;
	static const blist_node_type contact;
	static const blist_node_type buddy;
	static const blist_node_type chat;
	static const blist_node_type other;

	friend bool operator==(blist_node_type lhs, blist_node_type rhs);
	friend std::ostream& operator<< (std::ostream& os, blist_node_type type);
};

/** PurpleBuddy */
class buddy {
	PURPLEPP_NON_OWNING_WRAPPER(buddy, _PurpleBuddy);

	boost::string_view get_name() const;
	boost::string_view get_alias() const;
	presence get_presence() const;
	void test() const;

	account* get_account() const;

	friend std::ostream& operator<< (std::ostream& os, const buddy& buddy);
};

/** PurpleGroup */
class group {
	PURPLEPP_NON_OWNING_WRAPPER(group, _PurpleGroup);

	friend std::ostream& operator<< (std::ostream& os, const group& group);
};

/** PurpleContact */
class contact {
	PURPLEPP_NON_OWNING_WRAPPER(contact, _PurpleContact);

	boost::string_view get_alias() const;
	int get_total_size() const noexcept;
	int get_current_size() const noexcept;
	int get_online() const noexcept;

	friend std::ostream& operator<< (std::ostream& os, const contact& contact);
};

/** PurpleChat */
class chat {
	PURPLEPP_NON_OWNING_WRAPPER(chat, _PurpleChat);

	friend std::ostream& operator<< (std::ostream& os, const chat& chat);
};

/** PurpleBlistNode */
class blist_node {
	PURPLEPP_NON_OWNING_WRAPPER(blist_node, _PurpleBlistNode);

	blist_node_type get_type() const noexcept;

	buddy as_buddy() const noexcept;
	group as_group() const noexcept;
	contact as_contact() const noexcept;
	chat as_chat() const noexcept;

	boost::variant<std::nullptr_t, buddy, group, contact, chat> as_variant() const noexcept;

	friend std::ostream& operator<< (std::ostream& os, const blist_node& node);
};

}