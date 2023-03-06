#ifndef _EOKAS_SCANNER_H_
#define _EOKAS_SCANNER_H_

#include <eokas-base/main.h>

_BeginNamespace(eokas)

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif
	
	struct token_t
	{
		enum token_type
		{
			VAR, VAL, MAKE,
            MODULE, IMPORT, EXPORT,
            FUNC, PROC, STRUCT, ENUM,
			IF, ELSE, LOOP,  BREAK, CONTINUE, RETURN, TRUE, FALSE,
			COMMA, SEMICOLON, COLON, QUESTION, AT, POUND, DOLLAR,
			ADD, SUB, MUL, DIV, MOD, XOR, FLIP,
			LRB, RRB, LSB, RSB, LCB, RCB,
			AND, AND2, OR, OR2,
			ASSIGN, EQ, NOT, NE, GT, GE, LT, LE,
			SHIFT_L, SHIFT_R,
			DOT, DOT2, DOT3,
			INT_B, INT_X, INT_D, FLOAT, STRING, ID, EOS, COUNT, UNKNOWN
		};
		
		static const char* const names[COUNT];
		
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
