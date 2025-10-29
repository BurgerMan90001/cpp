
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

// counts the number of nodes in a tree
int number_of_nodes(mpc_ast_t* tree) {
	// base case no children
	if (tree->children_num == 0) {
		return 1;
	}
	if (tree->children_num >= 1) {
		int total = 1;
		for (int i = 0; i < tree->children_num; i++) {
			total = total + number_of_nodes(tree->children[i]);
		}
		return total;
	}
	return 0;
}
long eval_op(long x, char* operator, long y) {
	// strcmp checks for string equality
	if (strcmp(operator, "+") == 0) { return x + y; }
	if (strcmp(operator, "-") == 0) { return x - y; }
	if (strcmp(operator, "*") == 0) { return x * y; }
	if (strcmp(operator, "/") == 0) { return x / y; }
	return 0;
}
long eval(mpc_ast_t* tree) {
	// If tagged as a number, return it
	if (strstr(tree->tag, "number")) {
		// atoi is char to int
		return atoi(tree->contents);
	}
	
	// Second child is the operator
	char* operator = tree->children[1]->contents;
	
	// Third child is x
	long x = eval(tree->children[2]);
	// Rest of the children are iterated
	int i = 3;
	while(strstr(tree->children[i]->tag, "expr")) {
		x = eval_op(x, operator, eval(tree->children[i]));
		i++;
	}
	return x;
}

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
		operator : '+' | '-' | '*' | '/' ;                 \
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
			
			long result = eval(r.output);
			
			printf("%li\n", result);
			
			// Print the AST
			//mpc_ast_print(r.output);
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

