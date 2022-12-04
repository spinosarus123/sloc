/* sloc, a simple source lines of code counter.
 * Copyright (C) 2022 Isak Ellmer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>

struct counter {
	unsigned long physical;
	unsigned long logical;
	unsigned long comment;
};

struct flag {
	int pre;
	int mlc;
	int slc;
	int str;
	int chr;
	
	int physical;
	int logical;
	int comment;
	
	int paren;
	int curly;
	int colon;

	int escapechar;

	char lastchar;
};

char parsechar(const char c, struct counter *counter, struct flag *flag) {
	if (flag->escapechar) {
		flag->escapechar = 0;
	}
	else if (c == '\\') {
		flag->escapechar = 1;
	}
	else if (flag->mlc) {
		if (c == '\n')
			counter->comment++;
		if (flag->lastchar == '*' && c == '/') {
			flag->mlc = 0;
			counter->comment++;
		}
	}
	else if (flag->slc) {
		if (c == '\n') {
			counter->comment++;
			flag->slc = 0;
		}
	}
	else if (flag->str) {
		if (c == '"')
			flag->str = 0;
	}
	else if (flag->chr) {
		if (c == '\'')
			flag->chr = 0;
	}
	else switch (c) {
	case '\n':
		if (flag->physical)
			counter->physical += 1;

		if (flag->pre)
			counter->logical = 1;
		else if (flag->colon)
			counter->logical += 1;

		counter->logical += flag->logical;

		flag->pre = 0;
		flag->curly = 0;
		flag->colon = 0;

		flag->physical = 0;
		flag->logical = 0;
		break;
	case '#':
		flag->pre = 1;
		break;
	case '*':
		if (flag->lastchar == '/')
			flag->mlc = 1;
		break;
	case '/':
		if (flag->lastchar == '/')
			flag->slc = 1;
		break;
	case '\"':
		flag->str = 1;
		flag->physical = 1;
		break;
	case '\'':
		flag->chr = 1;
		flag->physical = 1;
		break;
	case ' ':
	case '\t':
		break;
	case ';':
		flag->physical = 1;
		if (!flag->curly && !flag->paren)
			flag->logical++;
		flag->curly = 0;
		flag->colon = 0;
		break;
	case ':':
		flag->physical = 1;
		flag->colon = 1;
		break;
	case '(':
		flag->physical = 1;
		flag->paren++;
		break;
	case ')':
		flag->physical = 1;
		flag->paren--;
		break;
	case '{':
		flag->logical++;
		flag->physical = 1;
		break;
	case '}':
		flag->curly = 1;
		flag->physical = 1;
		break;
	default:
		flag->physical = 1;
		break;
	}
	flag->lastchar = c;
	return c;
}

int sloc(FILE *file, struct counter *counter) {
	if (!file)
		exit(2);
	struct flag flag = { 0 };
	counter->physical = 0;
	counter->logical = 0;
	counter->comment = 0;
	while (parsechar(fgetc(file), counter, &flag) != EOF);
	return 0;
}

int main(int argc, char **argv) {
	int ret;
	if (argc < 2)
		exit(1);
	FILE *file = fopen(argv[1], "r");
	struct counter counter;
	ret = sloc(file, &counter);
	if (!ret) {
		printf("physical: %lu\n", counter.physical);
		printf("logical: %lu\n", counter.logical);
		printf("comment: %lu\n", counter.comment);
	}
	fclose(file);
	return ret;
}
