/**
 * @file text_editor.c
 * @author CuBeatSystems
 * @author Shinichiro Nakamura
 * @copyright
 * ===============================================================
 * Natural Tiny Shell (NT-Shell) Version 0.3.1
 * ===============================================================
 * Copyright (c) 2010-2016 Shinichiro Nakamura
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "text_editor.h"

/**
 * @brief Initialize the text editor module.
 *
 * @param p A pointer to the text editor handler.
 */
void text_editor_init(text_editor_t *p)
{
	p->pos = 0;
	p->len = 0;
	p->buffer[p->len] = '\0';
}

/**
 * @brief Insert a character.
 *
 * @param p A pointer to the text editor handler.
 * @param c A character.
 */
int text_editor_insert(text_editor_t *p, unsigned int ch, char *utf8)
{
	int len;

	if (ch < 0x80) {
		utf8[0] = ch & 0x7F;
		len = 1;
	}
	else if (ch < 0x00000800) {
		utf8[1] = (0x80 | (ch & 0x3F));
		utf8[0] = (0xC0 | (ch >> 6));
		len = 2;
	}
	else if (ch < 0x00010000) {
		utf8[2] = (0x80 | (ch & 0x3F));
		utf8[1] = (0x80 | ((ch >> 6) & 0x3F));
		utf8[0] = (0xE0 | (ch >> 12));
		len = 3;
	}
	else if (ch < 0x00200000) {
		utf8[3] = (0x80 | (ch & 0x3F));
		utf8[2] = (0x80 | ((ch >> 6) & 0x3F));
		utf8[1] = (0x80 | ((ch >> 12) & 0x3F));
		utf8[0] = (0xF0 | (ch >> 18));
		len = 4;
	}
	else if (ch < 0x04000000) {
		utf8[4] = (0x80 | (ch & 0x3F));
		utf8[3] = (0x80 | ((ch >> 6) & 0x3F));
		utf8[2] = (0x80 | ((ch >> 12) & 0x3F));
		utf8[1] = (0x80 | ((ch >> 18) & 0x3F));
		utf8[0] = (0xF8 | (ch >> 24));
		len = 5;
	}
	else if (ch < 0x80000000) {
		utf8[5] = (0x80 | (ch & 0x3F));
		utf8[4] = (0x80 | ((ch >> 6) & 0x3F));
		utf8[3] = (0x80 | ((ch >> 12) & 0x3F));
		utf8[2] = (0x80 | ((ch >> 18) & 0x3F));
		utf8[1] = (0x80 | ((ch >> 24) & 0x3F));
		utf8[0] = (0xFC | (ch >> 30));
		len = 6;
	}
	else {
		return 0;
	}

	if (p->len >= (int)sizeof(p->buffer) - len)
		return 0;

	int n = p->len - p->pos + len;
	char *src = p->buffer + p->len + 0;
	char *des = p->buffer + p->len + len;
	for (int i = 0; i < n; i++) {
		*des = *src;
		des--;
		src--;
	}

	for (int i = 0; i < len; i++) {
		p->buffer[p->pos] = utf8[i];
		p->pos++;
		p->len++;
	}
	p->buffer[p->len] = '\0';
	return len;
}

/**
* @brief count bytes of a deleted character by backspace.
*
* @param str A pointer to the text.
* @param pos The end of the text.
*/
int bs_char_count(char *str, int pos)
{
	int num = 1, local_pos = pos;
	char *c;

	while (1) {
		c = str + local_pos;

		if ((*c & 0x80) != 0x80)
			return num;

		if (((*c & 0xE0) == 0xC0) && (num == 2)) {
			return num;
		}
		else if (((*c & 0xF0) == 0xE0) && (num == 3)) {
			return num;
		}
		else if (((*c & 0xF8) == 0xF0) && (num == 4)) {
			return num;
		}
		else if (((*c & 0xFC) == 0xF8) && (num == 5)) {
			return num;
		}
		else if (((*c & 0xFE) == 0xFC) && (num == 6)) {
			return num;
		}
		else {
			if (local_pos > 0) {
				local_pos--;
				num++;
			}
			else
				return 0;
		}
	}

	return 0;
}

/**
 * @brief Delete a character.
 *
 * @param p A pointer to the text editor handler.
 */
int text_editor_backspace(text_editor_t *p)
{
	int bs_char_num;

	if (0 < p->pos) {
		int n = p->len - p->pos;
		int i;
		char *src = p->buffer + p->pos - 0;
		char *des = p->buffer + p->pos - 1;

		bs_char_num = bs_char_count(p->buffer, p->pos - 1);
		p->pos -= bs_char_num;
		p->len -= bs_char_num;

		for (i = 0; i < n; i++) {
			*des = *src;
			des++;
			src++;
		}
		*(p->buffer + p->len) = '\0';
		return (bs_char_num > 2 ? 2 : 1);
	}
	return 0;
}

/**
 * @brief Delete a character.
 *
 * @param p A pointer to the text editor handler.
 */
int text_editor_delete(text_editor_t *p)
{
	if (p->pos < p->len) {
		int n = p->len - p->pos - 1;
		int i;
		char *src = p->buffer + p->pos + 1;
		char *des = p->buffer + p->pos + 0;
		p->len--;
		for (i = 0; i < n; i++) {
			*des = *src;
			des++;
			src++;
		}
		*(p->buffer + p->len) = '\0';
		return 1;
	}
	return 0;
}

/**
 * @brief Get the cursor position.
 *
 * @param p A pointer to the text editor handler.
 */
int text_editor_cursor_get_position(text_editor_t *p)
{
	return p->pos;
}

/**
 * @brief Move to the cursor to the head of the line.
 *
 * @param p A pointer to the text editor handler.
 */
int text_editor_cursor_head(text_editor_t *p)
{
	if (0 < p->pos) {
		p->pos = 0;
		return 1;
	}
	return 0;
}

/**
 * @brief Move to the cursor to the tail of the line.
 *
 * @param p A pointer to the text editor handler.
 */
int text_editor_cursor_tail(text_editor_t *p)
{
	if (p->pos < p->len) {
		p->pos = p->len;
		return 1;
	}
	return 0;
}

/**
 * @brief Move to the cursor to left.
 *
 * @param p A pointer to the text editor handler.
 */
int text_editor_cursor_left(text_editor_t *p)
{
	if (0 < p->pos) {
		p->pos--;
		return 1;
	}
	return 0;
}

/**
 * @brief Move to the cursor to right.
 *
 * @param p A pointer to the text editor handler.
 */
int text_editor_cursor_right(text_editor_t *p)
{
	if (p->pos < p->len) {
		p->pos++;
		return 1;
	}
	return 0;
}

/**
 * @brief Set the edit line.
 *
 * @param p A pointer to the text editor handler.
 * @param buf A text string.
 */
int text_editor_set_text(text_editor_t *p, char *buf)
{
	char *src = buf;
	char *des = p->buffer;
	int n = 0;
	while (*src) {
		*des = *src;
		des++;
		src++;
		n++;
		if ((int)sizeof(p->buffer) <= n - 1) {
			break;
		}
	}
	*des = '\0';
	p->len = n;
	p->pos = p->len;
	return n;
}

/**
 * @brief Get the edit line.
 *
 * @param p A pointer to the text editor handler.
 * @param buf A text string.
 * @param siz Size of the text string buffer.
 */
int text_editor_get_text(text_editor_t *p, char *buf, int siz)
{
	char *src = p->buffer;
	char *des = buf;
	int n = 0;
	while (*src) {
		*des++ = *src++;
		n++;
		if (siz <= n) {
			break;
		}
	}
	*des = '\0';
	return n;
}

/**
 * @brief Clear the text string.
 *
 * @param p A pointer to the text editor handler.
 */
void text_editor_clear(text_editor_t *p)
{
	p->pos = 0;
	p->len = 0;
	p->buffer[p->len] = '\0';
}
