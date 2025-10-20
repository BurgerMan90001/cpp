
/* compile commands
linux: cc -std=c99 -Wall parsing.c mpc.c -ledit -lm -o parsing
windows: cc -std=c99 -Wall parsing.c mpc.c -o parsing
*/

#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"

// If compiling on windows
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

// Fake readline function
char* readline(char* prompt) {
	fputs(prompt, stdout);
	fgets(buffer, 2048, stdin);
	char* cpy = malloc(strlen(buffer)+1);
	strcpy(cpy, buffer);
	cpy[strlen(cpy)-1] = '\0';
	return cpy;
}
// Fake add_history function
void add_history(char* unused) {}

// Else, include editline headers
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif




int main(int argc, char** argv) {
	// Parsers
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	// Define the language
	mpca_lang(MPCA_LANG_DEFAULT,
		"                                                   \
		number   : /-?[0-9]+/;                             \
		operator : '+' | '-' | '*' | '/' | '%'              \
		| \"add\" | \"sub\" | \"mul\" | \"div\";            \
		expr     : <number> | '(' <operator> <expr>+ ')' ;  \
		lispy    : /^/ <operator> <expr>+ /$/ ;             \
	  ",
	  Number, Operator, Expr, Lispy);
	
	puts("Lispy Version 0.0.1");
	puts("Press Ctrl+c to Exit\n");
	
	// Infinite loop until Ctrl+c
	while (1) {
		// Get user input
		char* input = readline("lispee> ");
		// Add command history
		add_history(input);
		
		// Try to parse user input
		mpc_result_t r;
		// If parsing sucessful
		if (mpc_parse("<stdin>", input, Lispy, &r)) {
			// Print the AST
			mpc_ast_print(r.output);
			mpc_ast_delete(r.output);
		} else {
			// Else, print error
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
	}
	
	/* Undefine and Delete our Parsers */
	mpc_cleanup(4, Number, Operator, Expr, Lispy);

	return 0;
}

