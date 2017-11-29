//
// Created by int3 on 25.11.17.
//

#pragma once

#include <purple++/detail/util.h>

struct _PurpleStatus;
struct _PurpleStatusType;

namespace purplepp {

// instead of enum
class status_primitive {
	uint8_t _value;
public:
	/* implicit */ status_primitive(uint8_t value) : _value(value) {}

	static const status_primitive unset;
	static const status_primitive offline;
	static const status_primitive available;
	static const status_primitive unavailable;
	static const status_primitive invisible;
	static const status_primitive away;
	static const status_primitive extended_away;
	static const status_primitive mobile;
	static const status_primitive tune;
	static const status_primitive mood;
	static const status_primitive NUMBER;

	uint8_t _get_value() const;

	friend std::ostream& operator<< (std::ostream& os, status_primitive prim);
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