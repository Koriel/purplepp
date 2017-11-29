//
// Created by int3 on 25.11.17.
//

#pragma once

#include <purple++/detail/util.h>

struct _PurpleStatus;
struct _PurpleStatusType;

namespace purplepp {

class status_primitive {
	uint8_t _value;
public:
	/* implicit */ status_primitive(uint8_t value) : _value(value) {}

	static status_primitive unset;
	static status_primitive offline;
	static status_primitive available;
	static status_primitive unavailable;
	static status_primitive invisible;
	static status_primitive away;
	static status_primitive extended_away;
	static status_primitive mobile;
	static status_primitive tune;
	static status_primitive mood;
	static status_primitive NUMBER;

	uint8_t _get_value() const {
		return _value;
	}
};

class status_type {
	// non-owning
	_PurpleStatusType* _impl;

public:
	/* implicit */ status_type(_PurpleStatusType* impl);
	status_primitive get_primitive() const;
};

class status {
	// non-owning
	_PurpleStatus* _impl;

public:
	/* implicit */ status(_PurpleStatus* impl);
	status_type get_type() const;
};

}