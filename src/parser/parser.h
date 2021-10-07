#ifndef _EOKAS_PARSER_H_
#define _EOKAS_PARSER_H_

#include <libarchaism/archaism.h>

_BeginNamespace(eokas)
	
	class parser_t
	{
	public:
		parser_t();
		
		~parser_t();
	
	public:
		struct ast_module_t* parse(const char* source);
		
		const String& error() const;
	
	private:
		class parser_impl_t* impl;
	};
_EndNamespace(eokas)

#endif//_EOKAS_PARSER_H_
