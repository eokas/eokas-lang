
#ifndef  _EOKAS_BASE_ASCIL_H_
#define  _EOKAS_BASE_ASCIL_H_

#include "header.h"

_BeginNamespace(eokas)

#define _ascil_in_range(c, a, b)		((c) >= (a) && (c) <= (b))
#define _ascil_is_ascil(c)				_ascil_in_range(c, 0x00, 0x7F)
#define _ascil_is_control(c)			(_ascil_in_range(c, 0x01, 0x1F) || (c) == 0x7F)
#define _ascil_is_space(c)				((c) == 0x20)
#define _ascil_is_number(c)				_ascil_in_range(c, 0x30, 0x39)
#define _ascil_is_upper(c)				_ascil_in_range(c, 0x41, 0x5A)
#define _ascil_is_lower(c)				_ascil_in_range(c, 0x61, 0x7A)
#define _ascil_is_punct(c)				(\
										_ascil_in_range(c, 0x21, 0x2F) || \
										_ascil_in_range(c, 0x3A, 0x40) || \
										_ascil_in_range(c, 0x5B, 0x60) || \
										_ascil_in_range(c, 0x7B, 0x7E) \
										)
#define _ascil_is_alpha(c)				(_ascil_is_upper(c) || _ascil_is_lower(c))
#define _ascil_is_alpha_number(c)		(_ascil_is_alpha(c) || _ascil_is_number(c))
#define _ascil_is_hex(c)				(_ascil_is_number(c) || _ascil_in_range(c, 'A', 'F') || _ascil_in_range(c, 'a', 'f'))
#define _ascil_is_alpha_(c)				(_ascil_is_alpha(c) || (c) == '_')
#define _ascil_is_alpha_number_(c)		(_ascil_is_alpha_number(c) || (c) == '_')

struct Ascil
{
	char value;

	Ascil(char c):value(c){}
	Ascil& operator=(char c) { this->value = c; return *this; }
	operator char() { return this->value; }

	inline bool inRange(char a, char b) { return _ascil_in_range(this->value, a, b); }
	inline bool isAscil() { return _ascil_is_ascil(this->value); }
	inline bool isControl() { return _ascil_is_control(this->value); }
	inline bool isSpace() { return _ascil_is_space(this->value); }
	inline bool isNumber() { return _ascil_is_number(this->value); }
	inline bool isUpper() { return _ascil_is_upper(this->value); }
	inline bool isLower() { return _ascil_is_lower(this->value); }
	inline bool isPunct() { return _ascil_is_punct(this->value); }	
	inline bool isAlpha() { return _ascil_is_alpha(this->value); }
	inline bool isHex() { return _ascil_is_hex(this->value); }
	inline bool isAlphaNumber() { return _ascil_is_alpha_number(this->value); }
	inline bool isAlpha_() { return _ascil_is_alpha_(this->value); }
	inline bool isAlphaNumber_() { return _ascil_is_alpha_number_(this->value); }
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_ASCIL_H_
