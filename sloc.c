/* sloc, a simple source line counter.
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
#include <string.h>

enum {
	physical, logical, comment, end
};

struct flag {
	int pre;
	int mlc;
	int slc;
	int str;
	int chr;
	
	int physical;
	int logical;
	
	int parentheses;

	int escapechar;
};

int lookahead(const char *str, const char c) {
	for (int i = 0; str[i]; i++) {
		if (str[i] == c)
			return 1;
		if (str[i] != ' ' && str[i] != '\t')
			return 0;
	}
	return 0;
}

void loc(const char *str, unsigned long count[end], struct flag *flag) {
	//printf("%i, %i, %i, %i - %s", flag->mlc, flag->slc, flag->str, flag->chr, str);
	//printf("%s", str);
	for (char *ptr = (char *)str; *ptr; ptr++) {
		if (!flag->mlc && !flag->slc && !flag->str && !flag->chr &&
				*ptr == '#') {
			flag->pre = 1;
		}
		else if (!flag->mlc && !flag->slc && !flag->str && !flag->chr &&
				strncmp(ptr, "/*", 2) == 0) {
			flag->mlc = 1;
		}
		else if (flag->mlc && ptr >= str + 2 &&
				strncmp(ptr - 2, "*/", 2) == 0) {
			flag->mlc = 0;
			count[comment]++;
			//printf("%s", str);
		}
		else if (!flag->mlc && !flag->slc && !flag->chr &&
				*ptr == '"' && flag->escapechar % 2 == 0) {
			flag->str ^= 1;
		}
		else if (!flag->mlc && !flag->slc && !flag->str &&
				*ptr == '\'' && flag->escapechar % 2 == 0) {
			flag->chr ^= 1;
		}
		else if (!flag->mlc && !flag->str && !flag->chr &&
				strncmp(ptr, "//", 2) == 0) {
			flag->slc = 1;
		}

		switch (*ptr) {
		case '\n':
			/* line continuation */
			if (flag->escapechar % 2 == 1) {
				if (flag->physical) {
					//printf("%s", str);
					count[physical]++;
				}
				break;
			}

			if (flag->mlc) {
				count[comment]++;
				//printf("%s", str);
			}
			else if (flag->slc) {
				count[comment]++;
				//printf("%s", str);
			}

			if (flag->pre) {
				printf("%s", str);
				count[logical]++;
			}
			if (flag->physical) {
				//printf("%s", str);
				count[physical]++;
			}

			flag->pre = 0;
			flag->slc = 0;

			flag->physical = 0;
			flag->logical = 0;

			break;
		case ';':
			if (!flag->pre && !flag->mlc && !flag->slc && !flag->str &&
					!flag->chr && flag->parentheses == 0) {
				printf("%s", str);
				count[logical]++;
				flag->logical = 0;
			}
		case ' ':
		case '\t':
			break;
		case '(':
			if (!flag->pre && !flag->mlc && !flag->slc && !flag->str &&
					!flag->chr)
				flag->parentheses++;
			/* fallthrough */
		case ')':
			if (!flag->pre && !flag->mlc && !flag->slc && !flag->str &&
					!flag->chr) {
				flag->physical = 1;
				if (*ptr == ')') {
					flag->parentheses--;
					if (flag->logical && flag->parentheses == 0 &&
							!lookahead(ptr + 1, ';')) {
						printf("%s", str);
						count[logical]++;
						flag->logical = 0;
						flag->parentheses = 0;
					}
					break;
				}
			}
			/* fallthrough */
		default:
			if (!flag->mlc && !flag->slc && !flag->str && !flag->chr)
				flag->logical = 1;
			/* fallthrough */
		case '{':
		case '}':
			if (!flag->mlc && !flag->slc)
				flag->physical = 1;
			break;
		}

		if (*ptr == '\\')
			flag->escapechar++;
		else
			flag->escapechar = 0;
	}
}

int sloc(FILE *file, unsigned long count[end]) {
	if (!file)
		return 1;
	for (int i = 0; i < end; i++)
		count[i] = 0;
	struct flag flag = { 0 };
	char buffer[128];
	while (fgets(buffer, sizeof(buffer), file)) {
		loc(buffer, count, &flag);
	}
	return 0;
}

int main(int argc, char **argv) {
	FILE *file = fopen(argv[1], "r");
	unsigned long count[end];
	sloc(file, count);
	printf("physical: %lu\n", count[physical]);
	printf("logical: %lu\n", count[logical]);
	printf("comment: %lu\n", count[comment]);
	return 0;
}
