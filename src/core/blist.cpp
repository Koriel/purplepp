//
// Created by int3 on 02.12.17.
//

#include <purple++/core/blist.h>
#include <libpurple/blist.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <purple++/core/account.h>

namespace purplepp {

/** blsit_node_type */
const blist_node_type blist_node_type::group { PURPLE_BLIST_GROUP_NODE };
const blist_node_type blist_node_type::contact { PURPLE_BLIST_CONTACT_NODE };
const blist_node_type blist_node_type::buddy { PURPLE_BLIST_BUDDY_NODE };
const blist_node_type blist_node_type::chat { PURPLE_BLIST_CHAT_NODE };
const blist_node_type blist_node_type::other { PURPLE_BLIST_OTHER_NODE };

blist_node_type::blist_node_type(uint8_t value) : _value(value) {
	//PURPLEPP_ASSERT_IS_LOOP_THREAD();
}

bool operator==(blist_node_type lhs, blist_node_type rhs) {
	return lhs._value == rhs._value;
}

std::ostream& operator<<(std::ostream& os, blist_node_type type) {
	switch (type._value) {
		case PURPLE_BLIST_GROUP_NODE: return os << "group";
		case PURPLE_BLIST_CONTACT_NODE: return os << "contact";
		case PURPLE_BLIST_BUDDY_NODE: return os << "buddy";
		case PURPLE_BLIST_CHAT_NODE: return os << "chat";
		case PURPLE_BLIST_OTHER_NODE: return os << "other";
		default: return os << "<unknown(" << type._value << ")>";
	}
}

/** buddy */

boost::string_view buddy::get_name() const {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	return detail::to_view(_impl->name);
}

boost::string_view buddy::get_alias() const {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	return detail::to_view(_impl->alias);
}

account* buddy::get_account() const {
	return account::_get_wrapper(_impl->account);
}

presence buddy::get_presence() const {
	return { _impl->presence };
}

std::ostream& operator<<(std::ostream& os, const buddy& buddy) {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	return os << fmt::format("[buddy: name = {}, alias = {}]", buddy.get_name(), buddy.get_alias());
}

/** group */

std::ostream& operator<<(std::ostream& os, const group& group) {
	return os << "[group: ...]";
}

/** contact */
boost::string_view contact::get_alias() const {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	return detail::to_view(_impl->alias);
}

int contact::get_total_size() const noexcept {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	return _impl->totalsize;
}

int contact::get_current_size() const noexcept {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	return _impl->currentsize;
}

int contact::get_online() const noexcept {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	return _impl->online;
}

std::ostream& operator<<(std::ostream& os, const contact& contact) {
	return os << fmt::format("[contact: alias = {}, size = {}/{}, online = {}]",
							 contact.get_alias(), contact.get_current_size(), contact.get_total_size(), contact.get_online());
}

/** chat */

std::ostream& operator<<(std::ostream& os, const chat& chat) {
	return os << fmt::format("[chat: ...]");
}

/** blist_node */

blist_node_type blist_node::get_type() const noexcept {
	return { static_cast<uint8_t>(_impl->type) };
}

buddy blist_node::as_buddy() const noexcept {
	assert(get_type() == blist_node_type::buddy);

	return buddy{ reinterpret_cast<_PurpleBuddy*>(_impl) };
}

group blist_node::as_group() const noexcept {
	assert(get_type() == blist_node_type::group);

	return group { reinterpret_cast<_PurpleGroup*>(_impl) };
}

contact blist_node::as_contact() const noexcept {
	assert(get_type() == blist_node_type::contact);

	return contact { reinterpret_cast<_PurpleContact*>(_impl) };
}

chat blist_node::as_chat() const noexcept {
	assert(get_type() == blist_node_type::chat);

	return chat { reinterpret_cast<_PurpleChat*>(_impl) };
}

boost::variant<std::nullptr_t, buddy, group, contact, chat> blist_node::as_variant() const noexcept {
	if (get_type() == blist_node_type::buddy) {
		return as_buddy();
	}
	else if (get_type() == blist_node_type::group) {
		return as_group();
	}
	else if (get_type() == blist_node_type::contact) {
		return as_contact();
	}
	else if (get_type() == blist_node_type::chat) {
		return as_chat();
	}
	else {
		return nullptr;
	}
}

std::ostream& operator<<(std::ostream& os, const blist_node& node) {
	if (node.get_type() == blist_node_type::buddy) {
		return os << node.as_buddy();
	}
	else if (node.get_type() == blist_node_type::group) {
		return os << node.as_group();
	}
	else if (node.get_type() == blist_node_type::contact) {
		return os << node.as_contact();
	}
	else if (node.get_type() == blist_node_type::chat) {
		return os << node.as_chat();
	}
	else {
		return os << "<unknown(" << node._impl->type << ")>";
	}
}

}