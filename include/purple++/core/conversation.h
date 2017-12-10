//
// Created by int3 on 29.11.17.
//

#pragma once

#include <purple++/detail/util.h>
#include <boost/variant.hpp>
#include <boost/blank.hpp>

struct _PurpleConversation;
struct _PurpleConvIm;
struct _PurpleConvChat;
struct _PurpleConvMessage;
struct _PurpleConvChatBuddy;

namespace purplepp {

class account;
class buddy;
class conv_chat_buddy;

// instead of enum

// TODO: move to conv_message
class message_flags {
	uint32_t _value;
public:
	/* implicit */ message_flags(uint32_t value);

	static const message_flags send;
	static const message_flags recv;
	static const message_flags system;
	static const message_flags auto_resp;
	static const message_flags active_only;
	static const message_flags nick;
	static const message_flags no_log;
	static const message_flags whisper;
	static const message_flags error;
	static const message_flags delayed;
	static const message_flags raw;
	static const message_flags images;
	static const message_flags notify;
	static const message_flags no_linkify;
	static const message_flags invisible;
	static const message_flags remote_send;

	bool is_send() const;
	bool is_recv() const;
	bool is_system() const;
	bool is_auto_resp() const;
	bool is_active_only() const;
	bool is_nick() const;
	bool is_no_log() const;
	bool is_whisper() const;
	bool is_error() const;
	bool is_delayed() const;
	bool is_raw() const;
	bool is_images() const;
	bool is_notify() const;
	bool is_no_linkify() const;
	bool is_invisible() const;
	bool is_remote_send() const;

	friend std::ostream& operator<< (std::ostream& os, message_flags flags);

	uint32_t _get_value() const;
};

class conv_message {
	PURPLEPP_NON_OWNING_WRAPPER(conv_message, _PurpleConvMessage);

	time_t get_timestamp() const;
	boost::string_view get_text() const;
	message_flags get_flags() const;
	boost::string_view get_author() const;
};

// instead of enum
enum class conversation_type : uint8_t {
	unknown, im, chat, misc, any
};
std::ostream& operator<< (std::ostream& os, conversation_type);

class conv_im {
	PURPLEPP_NON_OWNING_WRAPPER(conv_im, _PurpleConvIm);

	~conv_im() noexcept = default;

	void send(boost::string_view message);
	double get_online_percent() const;

	void apply_to_buddy(callback_t<buddy&> cb) const;
};

class conv_chat {
	PURPLEPP_NON_OWNING_WRAPPER(conv_chat, _PurpleConvChat);

	void send(boost::string_view message);
	double get_online_percent() const;

	void for_each_chat_buddy(callback_t<conv_chat_buddy&> cb) const;
	void for_each_buddy(callback_t<buddy&> cb) const;
};

/** PurpleConvChatBuddy*/

class conv_chat_buddy {
	PURPLEPP_NON_OWNING_WRAPPER(conv_chat_buddy, _PurpleConvChatBuddy);

	enum class flags : uint8_t {
		none          = 0x0000, /**< No flags                     */
		voice         = 0x0001, /**< Voiced user or "Participant" */
		halfop        = 0x0002, /**< Half-op                      */
		op            = 0x0004, /**< Channel Op or Moderator      */
		founder       = 0x0008, /**< Channel Founder              */
		typing        = 0x0010, /**< Currently typing             */
		away          = 0x0020  /**< Currently away. @since 2.8.0 */
	};

	flags get_flags() const;

	bool is_flag_set(flags flag) const {
		return (static_cast<uint8_t>(get_flags()) & static_cast<uint8_t>(flag)) != 0;
	}
	bool is_voice() 	const	{ return is_flag_set(flags::voice); }
	bool is_halfop() 	const	{ return is_flag_set(flags::halfop); }
	bool is_op() 		const	{ return is_flag_set(flags::op); }
	bool is_founder() 	const	{ return is_flag_set(flags::founder); }
	bool is_typing() 	const	{ return is_flag_set(flags::typing); }
	bool is_away() 		const	{ return is_flag_set(flags::away); }

	boost::string_view get_name() const;
};

/** _PurpleConversation */
class conversation {
	// non-owning
	_PurpleConversation* _impl;

protected:
	conversation();

public:
	virtual ~conversation() noexcept;

	// base API
	boost::string_view get_name() const;
	conversation_type get_type() const;
	account* get_account() const;

	conv_chat as_chat() const noexcept;
	conv_im as_im() const noexcept;
	boost::variant<boost::blank, conv_chat, conv_im> as_variant() const noexcept;

	void send(boost::string_view message);

	void for_each_message(callback_t<conv_message&> cb);

	double get_online_percent() const;

	// internal
	//static conversation* _get_wrapper(_PurpleConversation* impl); // use account::_create_conversation instead!

	// ui_ops
	virtual void write_conv(boost::string_view who, boost::string_view alias, boost::string_view message, message_flags flags, time_t mtime) {};

	PURPLEPP_NON_COPYABLE_NON_MOVABLE(conversation);
};

class simple_conversation : public conversation {
public:
	void write_conv(boost::string_view who, boost::string_view alias, boost::string_view message, message_flags flags, time_t mtime) override;
};

}