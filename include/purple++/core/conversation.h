//
// Created by int3 on 29.11.17.
//

#pragma once

#include <purple++/detail/util.h>

struct _PurpleConversation;

namespace purplepp {

class conversation {
	// non-owning
	_PurpleConversation* _impl;

public:
	conversation();
	virtual ~conversation() = default;

	// base API
	boost::string_view get_name() const;

	// internal
	static conversation* _get_wrapper(_PurpleConversation* impl);

	// ui_ops
	virtual void write_conv(boost::string_view who, boost::string_view alias, boost::string_view message, time_t mtime) = 0;
};

class simple_conversation : public conversation {
public:
	void write_conv(boost::string_view who, boost::string_view alias, boost::string_view message, time_t mtime) override;
};

}