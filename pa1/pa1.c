/**********************************************************************
 * Copyright (c) 2021
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>

#include <string.h>

#include "types.h"
#include "list_head.h"
#include "parser.h"

#include <sys/types.h>
#include <sys/wait.h>

static int __process_command(char * command);
// static void append_history(char * const command);

struct entry {
	struct list_head list;
	char *string;
	int index;
};

LIST_HEAD(history);
int index_init = 0;

/***********************************************************************
 * run_command()
 *
 * DESCRIPTION
 *   Implement the specified shell features here using the parsed
 *   command tokens.
 *
 * RETURN VALUE
 *   Return 1 on successful command execution
 *   Return 0 when user inputs "exit"
 *   Return <0 on error
 */
static int run_command(int nr_tokens, char *tokens[])
{
	pid_t pid;
	
	// printf("%s %s %s", tokens[0], tokens[1], tokens[2]);


	if (strcmp(tokens[0], "exit") == 0) return 0;

	else if (strcmp(tokens[0], "cd") == 0) {
		if (nr_tokens == 1) {
			chdir(getenv("HOME"));
			return 1;
		}
		
		if (strcmp(tokens[1], "~") == 0) {
			chdir(getenv("HOME"));
			return 1;
		}
		else {
			chdir(tokens[1]);
			return 1;
		}
	}

	else if (strcmp(tokens[0], "history") == 0) {
		struct entry * x = malloc(sizeof(struct entry));
		struct list_head *cursor;

		list_for_each(cursor, &history){
		// x->string = malloc(sizeof(char *)+ 1);

		// x->string = malloc(100);
			x = list_entry(cursor ,struct entry, list);
			// printf("%s", x->string);

		// x = strdup(h)
		// if ((str = strdup(x->string)) != NULL){
			fprintf(stderr, "%2d: %s", x->index, x->string);
		// }
		}
		return 1;
	}

	else if (strcmp(tokens[0], "!") == 0) {
		struct entry * x;
		// x->string = malloc(sizeof())
		struct list_head *cursor;
		int history_num = atoi(tokens[1]);
		// int ret = 0;

		list_for_each(cursor, &history){
		x = list_entry(cursor ,struct entry, list);
		// fprintf(stderr,"%s %d", x->string, x->index);

		if (x->index == history_num) {
			// append_history(x->string);
			__process_command(x->string);
			// fprintf(stderr,"%s", x->string);
			
			return 1;
		}

		}
	}

	else {
		
		pid = fork();

		if (pid == 0) {
			if (execvp(tokens[0], tokens) == -1) {
				fprintf(stderr, "Unable to execute %s\n", tokens[0]);
				exit(0);
			}
			else {
				// fprintf(stderr, "%s\n", tokens[0]);
				execvp(tokens[0], tokens);
				exit(0);
			}
		}
		else {
			wait(NULL);
		}
	}
	// fprintf(stderr, "Unable to execute %s\n", tokens[0]);
	// fprintf(stderr, "Unable to execute %s\n", tokens[1]);
}


/***********************************************************************
 * struct list_head history
 *
 * DESCRIPTION
 *   Use this list_head to store unlimited command history.
 */


/***********************************************************************
 * append_history()
 *
 * DESCRIPTION
 *   Append @command into the history. The appended command can be later
 *   recalled with "!" built-in command
 */
static void append_history(char * const command)
{
	struct entry* his = malloc(sizeof(struct entry));
	his->string = strdup(command);
	his->index = index_init++;
	list_add_tail(&his->list, &history);
	// printf("%s\n", his->string);

}


/***********************************************************************
 * initialize()
 *
 * DESCRIPTION
 *   Call-back function for your own initialization code. It is OK to
 *   leave blank if you don't need any initialization.
 *
 * RETURN VALUE
 *   Return 0 on successful initialization.
 *   Return other value on error, which leads the program to exit.
 */
static int initialize(int argc, char * const argv[])
{
	return 0;
}


/***********************************************************************
 * finalize()
 *
 * DESCRIPTION
 *   Callback function for finalizing your code. Like @initialize(),
 *   you may leave this function blank.
 */
static void finalize(int argc, char * const argv[])
{

}


/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING BELOW THIS LINE ******      */
/*          ****** BUT YOU MAY CALL SOME IF YOU WANT TO.. ******      */
static int __process_command(char * command)
{
	char *tokens[MAX_NR_TOKENS] = { NULL };
	int nr_tokens = 0;

	if (parse_command(command, &nr_tokens, tokens) == 0)
		return 1;

	return run_command(nr_tokens, tokens);
}

static bool __verbose = true;
static const char *__color_start = "[0;31;40m";
static const char *__color_end = "[0m";

static void __print_prompt(void)
{
	char *prompt = "$";
	if (!__verbose) return;

	fprintf(stderr, "%s%s%s ", __color_start, prompt, __color_end);
}

/***********************************************************************
 * main() of this program.
 */
int main(int argc, char * const argv[])
{
	char command[MAX_COMMAND_LEN] = { '\0' };
	int ret = 0;
	int opt;

	while ((opt = getopt(argc, argv, "qm")) != -1) {
		switch (opt) {
		case 'q':
			__verbose = false;
			break;
		case 'm':
			__color_start = __color_end = "\0";
			break;
		}
	}

	if ((ret = initialize(argc, argv))) return EXIT_FAILURE;

	/**
	 * Make stdin unbuffered to prevent ghost (buffered) inputs during
	 * abnormal exit after fork()
	 */
	setvbuf(stdin, NULL, _IONBF, 0);

	while (true) {
		__print_prompt();

		if (!fgets(command, sizeof(command), stdin)) break;
		// printf("%s\n",command);

		append_history(command);
		ret = __process_command(command);
		// printf("%s\n",command);

		if (!ret) break;
	}

	finalize(argc, argv);

	return EXIT_SUCCESS;
}
