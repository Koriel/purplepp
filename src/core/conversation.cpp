//
// Created by int3 on 29.11.17.
//

#include <purple++/core/conversation.h>
#include <purple++/core/account.h>
#include <purple++/core/status.h>
#include <purple++/core/blist.h>
#include <purple++/detail/internal.h>
#include <purple++/detail/lambda_visitor.h>
#include <purple++/library.h>

#include <libpurple/conversation.h>

#include <cassert>
#include <fmt/ostream.h>
#include <fmt/printf.h>
#include <conversation.h>

namespace purplepp {

/** message_flags */

std::ostream& operator<<(std::ostream& os, message_flags flags) {
	fmt::MemoryWriter writer;

	if (flags.is_send()) writer << "send, ";
	if (flags.is_recv()) writer << "recv, ";
	if (flags.is_system()) writer << "system, ";
	if (flags.is_auto_resp()) writer << "auto_resp, ";
	if (flags.is_active_only()) writer << "active_only, ";
	if (flags.is_nick()) writer << "nick, ";
	if (flags.is_no_log()) writer << "no_log, ";
	if (flags.is_whisper()) writer << "whisper, ";
	if (flags.is_error()) writer << "error, ";
	if (flags.is_delayed()) writer << "delayed, ";
	if (flags.is_raw()) writer << "raw, ";
	if (flags.is_images()) writer << "images, ";
	if (flags.is_notify()) writer << "notify, ";
	if (flags.is_no_linkify()) writer << "no_linkify, ";
	if (flags.is_invisible()) writer << "invisible, ";
	if (flags.is_remote_send()) writer << "remote_send, ";

	auto str = writer.str();
	if (!str.empty()) {
		str.pop_back();
		str.pop_back();
	}
	return os << "[" << str << "]";
}

uint32_t message_flags::_get_value() const {
	return _value;
}

const message_flags message_flags::send(PURPLE_MESSAGE_SEND);
const message_flags message_flags::recv(PURPLE_MESSAGE_RECV);
const message_flags message_flags::system(PURPLE_MESSAGE_SYSTEM);
const message_flags message_flags::auto_resp(PURPLE_MESSAGE_AUTO_RESP);
const message_flags message_flags::active_only(PURPLE_MESSAGE_ACTIVE_ONLY);
const message_flags message_flags::nick(PURPLE_MESSAGE_NICK);
const message_flags message_flags::no_log(PURPLE_MESSAGE_NO_LOG);
const message_flags message_flags::whisper(PURPLE_MESSAGE_WHISPER);
const message_flags message_flags::error(PURPLE_MESSAGE_ERROR);
const message_flags message_flags::delayed(PURPLE_MESSAGE_DELAYED);
const message_flags message_flags::raw(PURPLE_MESSAGE_RAW);
const message_flags message_flags::images(PURPLE_MESSAGE_IMAGES);
const message_flags message_flags::notify(PURPLE_MESSAGE_NOTIFY);
const message_flags message_flags::no_linkify(PURPLE_MESSAGE_NO_LINKIFY);
const message_flags message_flags::invisible(PURPLE_MESSAGE_INVISIBLE);
const message_flags message_flags::remote_send(PURPLE_MESSAGE_REMOTE_SEND);

message_flags::message_flags(uint32_t value) : _value(value) {

}

bool message_flags::is_send() const 		{ return (_value & send._get_value()) != 0; }
bool message_flags::is_recv() const 		{ return (_value & recv._get_value()) != 0; }
bool message_flags::is_system() const 		{ return (_value & system._get_value()) != 0; }
bool message_flags::is_auto_resp() const 	{ return (_value & auto_resp._get_value()) != 0; }
bool message_flags::is_active_only() const 	{ return (_value & active_only._get_value()) != 0; }
bool message_flags::is_nick() const 		{ return (_value & nick._get_value()) != 0; }
bool message_flags::is_no_log() const 		{ return (_value & no_log._get_value()) != 0; }
bool message_flags::is_whisper() const 		{ return (_value & whisper._get_value()) != 0; }
bool message_flags::is_error() const 		{ return (_value & error._get_value()) != 0; }
bool message_flags::is_delayed() const 		{ return (_value & delayed._get_value()) != 0; }
bool message_flags::is_raw() const 			{ return (_value & raw._get_value()) != 0; }
bool message_flags::is_images() const 		{ return (_value & images._get_value()) != 0; }
bool message_flags::is_notify() const 		{ return (_value & notify._get_value()) != 0; }
bool message_flags::is_no_linkify() const 	{ return (_value & no_linkify._get_value()) != 0; }
bool message_flags::is_invisible() const 	{ return (_value & invisible._get_value()) != 0; }
bool message_flags::is_remote_send() const 	{ return (_value & remote_send._get_value()) != 0; }

/** conv_message */
time_t conv_message::get_timestamp() const {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	return _impl->when;
}

boost::string_view conv_message::get_text() const {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	return detail::to_view(_impl->what);
}

message_flags conv_message::get_flags() const {
	return _impl->flags;
}

boost::string_view conv_message::get_author() const {
	return detail::to_view(_impl->who);
}

/** conversation_type */

PURPLEPP_ENUM_ASSERT_CORRECT(conversation_type::unknown, PURPLE_CONV_TYPE_UNKNOWN);
PURPLEPP_ENUM_ASSERT_CORRECT(conversation_type::im, PURPLE_CONV_TYPE_IM);
PURPLEPP_ENUM_ASSERT_CORRECT(conversation_type::chat, PURPLE_CONV_TYPE_CHAT);
PURPLEPP_ENUM_ASSERT_CORRECT(conversation_type::misc, PURPLE_CONV_TYPE_MISC);
PURPLEPP_ENUM_ASSERT_CORRECT(conversation_type::any, PURPLE_CONV_TYPE_ANY);

std::ostream& operator<<(std::ostream& os, conversation_type type) {
	switch (static_cast<uint8_t>(type)) {
		case PURPLE_CONV_TYPE_UNKNOWN:	return os << "unknown";
		case PURPLE_CONV_TYPE_IM:		return os << "im";
		case PURPLE_CONV_TYPE_CHAT:		return os << "chat";
		case PURPLE_CONV_TYPE_MISC:		return os << "misc";
		case PURPLE_CONV_TYPE_ANY:		return os << "any";
		default:						return os << "<unknown(" << static_cast<uint8_t>(type) << ")>";
	}
}

/** conv_im */

void conv_im::send(boost::string_view message) {
	library::add_task([=] {
		PURPLEPP_ASSERT_IS_LOOP_THREAD();
		purple_conv_im_send(_impl, message.to_string().c_str());
	});
}

double conv_im::get_online_percent() const {
	bool is_online = false;

	if (_impl->icon == nullptr) return 0.1;

	auto name = purple_buddy_icon_get_username(_impl->icon);
	account::_get_wrapper(_impl->conv->account)->apply_to_buddy(detail::to_view(name), [&](buddy& b){
		is_online = b.get_presence().get_active_status().get_primitive() != status_primitive::offline;
	});
	return is_online ? 1.0 : 0.5;
}

void conv_im::apply_to_buddy(callback_t<purplepp::buddy&> cb) const {
	if (_impl->icon == nullptr) return;
	auto name = purple_buddy_icon_get_username(_impl->icon);
	account::_get_wrapper(_impl->conv->account)->apply_to_buddy(detail::to_view(name), cb);
}

/** conv_chat */

void conv_chat::send(boost::string_view message) {
	library::add_task([=] {
		PURPLEPP_ASSERT_IS_LOOP_THREAD();
		purple_conv_chat_send(_impl, message.to_string().c_str());
	});
}

double conv_chat::get_online_percent() const {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	size_t total = 0;
	size_t online = 0;
	for_each_buddy([&](buddy& buddy) {
		auto status = buddy.get_presence().get_active_status().get_primitive();
		total++;
		if (status != status_primitive::offline) online++;
	});

	return static_cast<double>(online) / total;
}

void conv_chat::for_each_chat_buddy(callback_t<conv_chat_buddy&> cb) const {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	for (auto&& p : detail::ghashtable<char*, PurpleConvChatBuddy*>(_impl->users)) {
		conv_chat_buddy buddy{ p.second };
		cb(buddy);
	}
}

void conv_chat::for_each_buddy(callback_t<purplepp::buddy&> cb) const {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	for_each_chat_buddy([&](conv_chat_buddy& buddy){
		account::_get_wrapper(_impl->conv->account)->apply_to_buddy(buddy.get_name(), cb);
	});
}

/** conv_chat_buddy */
conv_chat_buddy::flags conv_chat_buddy::get_flags() const {
	return static_cast<flags>(_impl->flags);
}

boost::string_view conv_chat_buddy::get_name() const {
	return detail::to_view(_impl->name);
}

/** conversation */

conversation::conversation() {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	_impl = detail::thread_local_cache<PurpleConversation*, conversation>::get();
	_impl->ui_data = this;

	fmt::print("_impl: {}, _ui_data: {}\n", (void*)_impl, (void*)_impl->ui_data);
}

conversation::~conversation() noexcept {
	_impl->ui_data = nullptr;
}

boost::string_view conversation::get_name() const {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	return detail::to_view(_impl->name);
}

conversation_type conversation::get_type() const {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	return static_cast<conversation_type>(_impl->type);
}

account* conversation::get_account() const {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	return account::_get_wrapper(_impl->account);
}

conv_chat conversation::as_chat() const noexcept {
	return conv_chat{ _impl->u.chat };
}

conv_im conversation::as_im() const noexcept {
	return conv_im{ _impl->u.im };
}

boost::variant<boost::blank, conv_chat, conv_im> conversation::as_variant() const noexcept {
	switch (_impl->type) {
		case PURPLE_CONV_TYPE_CHAT:	return as_chat();
		case PURPLE_CONV_TYPE_IM:	return as_im();
		default:					return boost::blank{};
	}
}

void conversation::send(boost::string_view message) {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	detail::apply_lambda_visitor(as_variant(), [=](auto&& c) { c.send(message); });
}

void conversation::for_each_message(callback_t<conv_message&> cb) {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	for (auto ptr : detail::glist<PurpleConvMessage*>(purple_conversation_get_message_history(_impl))) {
		conv_message msg{ ptr };
		cb(msg);
	}
}

double conversation::get_online_percent() const {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	return detail::apply_lambda_visitor<double>(as_variant(), [=](auto&& c) { return c.get_online_percent(); }, [](boost::blank) { return 0.0; });
}

/*conversation* conversation::_get_wrapper(_PurpleConversation* impl) {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	assert(impl->ui_data);
	return (conversation*)impl->ui_data;
}*/

void simple_conversation::write_conv(boost::string_view who, boost::string_view alias, boost::string_view message, message_flags flags, time_t mtime) {
	PURPLEPP_ASSERT_IS_LOOP_THREAD();

	std::string name = alias.empty() ? who.empty() ? "<empty>" : who.to_string() : alias.to_string();

	fmt::print("write_conv {}: ({}) {} {}: {}\n", flags, get_name(), purple_utf8_strftime("(%H:%M:%S)", localtime(&mtime)), name, message);

	if (flags.is_recv()) {
		send(fmt::format(u8"Эхо: {}", message));
	}
}

}