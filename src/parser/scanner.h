#ifndef _EOKAS_SCANNER_H_
#define _EOKAS_SCANNER_H_

#include <libarchaism/archaism.h>

_BeginNamespace(eokas)
	
	struct token_t
	{
		enum token_type
		{
			Var, Val, Make, Struct, Enum, Proc, Func, Array,
			If, Else, Loop, Continue, Break, Return,
			Is, As, True, False,
			Comma, Semicolon, Colon, Question, At, Pound, Dollar,
			Add, Sub, Mul, Div, Mod, Xor, Flip,
			LRB, RRB, LSB, RSB, LCB, RCB,
			And, And2, Or, Or2,
			Assign, Equal, Not, NEqual, Greater, GEqual, Less, LEqual,
			ShiftL, ShiftR,
			Dot, Dot2, Dot3,
			BInt, XInt, DInt, Float, Str, ID, Eos,
			Count, Unknown
		};
		static const char* const names[Count];
		
		token_type type;
		String value;
		
		token_t();
		
		const char* const name() const;
		
		bool infer(token_type default_type);
		
		void clear();
	};
	
	class scanner_t
	{
	public:
		scanner_t();
		
		virtual ~scanner_t();
	
	public:
		void ready(const char* source);
		
		void clear();
		
		const char* source();
		
		void next_token();
		
		token_t& token();
		
		token_t& look_ahead_token();
		
		int line();
		
		int column();
	
	private:
		void scan();
		
		void scan_number();
		
		void scan_string(char delimiter);
		
		void scan_identifier();
		
		void scan_line_comment();
		
		void scan_section_comment();
		
		void new_line();
		
		void read_char();
		
		void save_char(char c);
		
		void save_and_read_char();
		
		bool check_char(const char* charset);
	
	private:
		const char* m_source;
		const char* m_position;
		char m_current;
		token_t m_token;
		token_t m_look_ahead_token;
		int m_line;
		int m_column;
	};
_EndNamespace(eokas)

#endif//_EOKAS_SCANNER_H_
