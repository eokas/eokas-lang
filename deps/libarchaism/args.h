
#ifndef  _EOKAS_ARCHAISM_ARGS_H_
#define  _EOKAS_ARCHAISM_ARGS_H_

#include "header.h"
#include "string.h"

_BeginNamespace(eokas)

class Args
{
public:
	Args(int argc, char** argv);

public:
	size_t count() const;
	bool has(const String& arg) const;
	const StringValue& get(size_t pos, const StringValue& def = StringValue::empty) const;
	const StringValue& get(const String& arg, const StringValue& def = StringValue::empty) const;

private:
	std::vector<StringValue> mArgs;
};

_EndNamespace(eokas)

#endif//_EOKAS_ARCHAISM_ARGS_H_
