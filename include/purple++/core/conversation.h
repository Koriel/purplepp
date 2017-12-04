//
// Created by int3 on 29.11.17.
//

#pragma once

#include <purple++/detail/util.h>

struct _PurpleConversation;
struct _PurpleConvIm;
struct _PurpleConvChat;

namespace purplepp {

class account;

// instead of enum
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

// instead of enum
class conversation_type {
	uint8_t _value;
public:
	/* implicit */ conversation_type(uint8_t value) : _value(value) {}

	static const conversation_type unknown;
	static const conversation_type im;
	static const conversation_type chat;
	static const conversation_type misc;
	static const conversation_type any;

	uint8_t _get_value() const;

	friend std::ostream& operator<< (std::ostream& os, conversation_type type);
};

class conv_im {
	// non-owning
	_PurpleConvIm* _impl;

public:
	/* implicit */ conv_im(_PurpleConvIm* impl);
	~conv_im() noexcept = default;

	void send(boost::string_view message);
};

class conv_chat {
	// non-owning
	_PurpleConvChat* _impl;

public:
	/* implicit */ conv_chat(_PurpleConvChat* impl);

	void send(boost::string_view message);
};

class conversation {
	// non-owning
	_PurpleConversation* _impl;

protected:
	conversation();

public:
	virtual ~conversation() = default;

	// base API
	boost::string_view get_name() const;
	conversation_type get_type() const;
	account* get_account() const;
	void send(boost::string_view message);

	// internal
	static conversation* _get_wrapper(_PurpleConversation* impl);

	// ui_ops
	virtual void write_conv(boost::string_view who, boost::string_view alias, boost::string_view message, message_flags flags, time_t mtime) = 0;

	PURPLEPP_NON_COPYABLE_NON_MOVABLE(conversation);
};

class simple_conversation : public conversation {
public:
	void write_conv(boost::string_view who, boost::string_view alias, boost::string_view message, message_flags flags, time_t mtime) override;
};

}