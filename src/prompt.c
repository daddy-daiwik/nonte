/**************************************************************************
 *   prompt.c  --  This file is part of GNU nano.                         *
 *                                                                        *
 *   Copyright (C) 1999-2011, 2013-2026 Free Software Foundation, Inc.    *
 *   Copyright (C) 2016, 2018, 2020-2022, 2025 Benno Schulenberg          *
 *                                                                        *
 *   GNU nano is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published    *
 *   by the Free Software Foundation, either version 3 of the License,    *
 *   or (at your option) any later version.                               *
 *                                                                        *
 *   GNU nano is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty          *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.              *
 *   See the GNU General Public License for more details.                 *
 *                                                                        *
 *   You should have received a copy of the GNU General Public License    *
 *   along with this program.  If not, see https://gnu.org/licenses/.     *
 *                                                                        *
 **************************************************************************/

#include "prototypes.h"

#include <string.h>

static char *prompt = NULL;
		/* The prompt string used for status-bar questions. */
static size_t typing_x = HIGHEST_POSITIVE;
		/* The cursor position in answer. */

/* Move to the beginning of the answer. */
void do_statusbar_home(void)
{
	typing_x = 0;
}

/* Move to the end of the answer. */
void do_statusbar_end(void)
{
	typing_x = strlen(answer);
}

#ifndef NANO_TINY
/* Move to the previous word in the answer. */
void do_statusbar_prev_word(void)
{
	bool seen_a_word = FALSE, step_forward = FALSE;

	/* Move backward until we pass over the start of a word. */
	while (typing_x != 0) {
		typing_x = step_left(answer, typing_x);

		if (is_word_char(answer + typing_x, FALSE))
			seen_a_word = TRUE;
#ifdef ENABLE_UTF8
		else if (is_zerowidth(answer + typing_x))
			; /* skip */
#endif
		else if (seen_a_word) {
			/* This is space now: we've overshot the start of the word. */
			step_forward = TRUE;
			break;
		}
	}

	if (step_forward)
		/* Move one character forward again to sit on the start of the word. */
		typing_x = step_right(answer, typing_x);
}

/* Move to the next word in the answer. */
void do_statusbar_next_word(void)
{
	bool seen_space = !is_word_char(answer + typing_x, FALSE);
	bool seen_word = !seen_space;

	/* Move forward until we reach either the end or the start of a word,
	 * depending on whether the AFTER_ENDS flag is set or not. */
	while (answer[typing_x]) {
		typing_x = step_right(answer, typing_x);

		if (ISSET(AFTER_ENDS)) {
			/* If this is a word character, continue; else it's a separator,
			 * and if we've already seen a word, then it's a word end. */
			if (is_word_char(answer + typing_x, FALSE))
				seen_word = TRUE;
#ifdef ENABLE_UTF8
			else if (is_zerowidth(answer + typing_x))
				; /* skip */
#endif
			else if (seen_word)
				break;
		} else {
#ifdef ENABLE_UTF8
			if (is_zerowidth(answer + typing_x))
				; /* skip */
			else
#endif
			/* If this is not a word character, then it's a separator; else
			 * if we've already seen a separator, then it's a word start. */
			if (!is_word_char(answer + typing_x, FALSE))
				seen_space = TRUE;
			else if (seen_space)
				break;
		}
	}
}
#endif /* !NANO_TINY */

/* Move left one character in the answer. */
void do_statusbar_left(void)
{
	if (typing_x > 0) {
		typing_x = step_left(answer, typing_x);
#ifdef ENABLE_UTF8
		while (typing_x > 0 && is_zerowidth(answer + typing_x))
			typing_x = step_left(answer, typing_x);
#endif
	}
}

/* Move right one character in the answer. */
void do_statusbar_right(void)
{
	if (answer[typing_x]) {
		typing_x = step_right(answer, typing_x);
#ifdef ENABLE_UTF8
		while (answer[typing_x] && is_zerowidth(answer + typing_x))
			typing_x = step_right(answer, typing_x);
#endif
	}
}

/* Backspace over one character in the answer. */
void do_statusbar_backspace(void)
{
	if (typing_x > 0) {
		size_t was_x = typing_x;

		typing_x = step_left(answer, typing_x);
		memmove(answer + typing_x, answer + was_x, strlen(answer) - was_x + 1);
	}
}

/* Delete one character in the answer. */
void do_statusbar_delete(void)
{
	if (answer[typing_x]) {
		int charlen = char_length(answer + typing_x);

		memmove(answer + typing_x, answer + typing_x + charlen,
						strlen(answer) - typing_x - charlen + 1);
#ifdef ENABLE_UTF8
		if (is_zerowidth(answer + typing_x))
			do_statusbar_delete();
#endif
	}
}

/* Zap the part of the answer after the cursor, or the whole answer. */
void lop_the_answer(void)
{
	if (answer[typing_x] == '\0')
		typing_x = 0;

	answer[typing_x] = '\0';
}

#ifndef NANO_TINY
/* Copy the current answer (if any) into the cutbuffer. */
void copy_the_answer(void)
{
	if (*answer) {
		free_lines(cutbuffer);
		cutbuffer = make_new_node(NULL);
		cutbuffer->data = copy_of(answer);
		typing_x = 0;
	}
}

/* Paste the first line of the cutbuffer into the current answer. */
void paste_into_answer(void)
{
	size_t pastelen = strlen(cutbuffer->data);

	answer = nrealloc(answer, strlen(answer) + pastelen + 1);
	memmove(answer + typing_x + pastelen, answer + typing_x, strlen(answer) - typing_x + 1);
	strncpy(answer + typing_x, cutbuffer->data, pastelen);

	typing_x += pastelen;
}
#endif

#ifdef ENABLE_MOUSE
/* Handle a mouse click in the prompt bar or the help lines. */
int process_prompt_click(void)
{
	int click_row, click_col;
	int retval = get_mouseinput(&click_row, &click_col);  /* Handles shortcuts. */

	/* When the click is in the prompt bar, position the cursor. */
	if (retval == 0 && wmouse_trafo(footwin, &click_row, &click_col, FALSE)) {
		size_t start_col = breadth(prompt) + 2;

		if (click_col >= start_col)
			typing_x = actual_x(answer, get_statusbar_page_start(start_col, start_col +
								wideness(answer, typing_x)) + click_col - start_col);
		else
			typing_x = 0;
	}

	return retval;
}
#endif

/* Insert the given short burst of bytes into the answer. */
void inject_into_answer(char *burst, size_t count)
{
	/* First encode any embedded NUL byte as 0x0A. */
	for (size_t index = 0; index < count; index++)
		if (burst[index] == '\0')
			burst[index] = '\n';

	answer = nrealloc(answer, strlen(answer) + count + 1);
	memmove(answer + typing_x + count, answer + typing_x, strlen(answer) - typing_x + 1);
	strncpy(answer + typing_x, burst, count);

	typing_x += count;
}

/* Get a verbatim keystroke and insert it into the answer. */
void do_statusbar_verbatim_input(void)
{
	size_t count = 1;
	char *bytes;

	bytes = get_verbatim_kbinput(footwin, &count);

	if (0 < count && count < 999)
		inject_into_answer(bytes, count);
	else if (count == 0)
		beep();

	free(bytes);
}

/* Add the given input to the input buffer when it's a normal byte,
 * and inject the gathered bytes into the answer when ready. */
void absorb_character(int input, functionptrtype function)
{
	static char *puddle = NULL;
		/* The input buffer. */
	static size_t capacity = 8;
		/* The size of the input buffer; gets doubled whenever needed. */
	static size_t depth = 0;
		/* The length of the input buffer. */

	/* If not a command, discard anything that is not a normal character byte.
	 * Apart from that, only accept input when not in restricted mode, or when
	 * not at the "Write File" prompt, or when there is no filename yet. */
	if (!function) {
		if ((input < 0x20 && input != '\t') || meta_key || input > 0xFF)
			beep();
		else if (!ISSET(RESTRICTED) || currmenu != MWRITEFILE || openfile->filename[0] == '\0') {
			/* When the input buffer (plus room for terminating NUL) is full,
			 * extend it; otherwise, if it does not exist yet, create it. */
			if (depth + 1 == capacity) {
				capacity = 2 * capacity;
				puddle = nrealloc(puddle, capacity);
			} else if (!puddle)
				puddle = nmalloc(capacity);

			puddle[depth++] = (char)input;
		}
	}

	/* If there are gathered bytes and we have a command or no other key codes
	 * are waiting, it's time to insert these bytes into the answer. */
	if (depth > 0 && (function || waiting_keycodes() == 0)) {
		puddle[depth] = '\0';
		inject_into_answer(puddle, depth);
		depth = 0;
	}
}

/* Handle any editing shortcut, and return TRUE when handled. */
bool handle_editing(functionptrtype function)
{
	if (function == do_left)
		do_statusbar_left();
	else if (function == do_right)
		do_statusbar_right();
#ifndef NANO_TINY
	else if (function == to_prev_word)
		do_statusbar_prev_word();
	else if (function == to_next_word)
		do_statusbar_next_word();
#endif
	else if (function == do_home)
		do_statusbar_home();
	else if (function == do_end)
		do_statusbar_end();
	/* When in restricted mode at the "Write File" prompt and the
	 * filename isn't blank, disallow any input and deletion. */
	else if (ISSET(RESTRICTED) && currmenu == MWRITEFILE && openfile->filename[0] &&
							(function == do_verbatim_input ||
							function == do_delete || function == do_backspace ||
							function == cut_text || function == paste_text))
		;
	else if (function == do_verbatim_input)
		do_statusbar_verbatim_input();
	else if (function == do_delete)
		do_statusbar_delete();
	else if (function == do_backspace)
		do_statusbar_backspace();
	else if (function == cut_text)
		lop_the_answer();
#ifndef NANO_TINY
	else if (function == copy_text)
		copy_the_answer();
	else if (function == paste_text) {
		if (cutbuffer)
			paste_into_answer();
	}
#endif
	else
		return FALSE;

	/* Don't handle any handled function again. */
	return TRUE;
}

/* Return the column number of the first character of the answer that is
 * displayed in the status bar when the cursor is at the given column,
 * with the available room for the answer starting at base.  Note that
 * (0 <= column - get_statusbar_page_start(column) < COLS). */
size_t get_statusbar_page_start(size_t base, size_t column)
{
	if (column == base || column < COLS - 1)
		return 0;
	else if (COLS > base + 2)
		return column - base - 1 - (column - base - 1) % (COLS - base - 2);
	else
		return column - 2;
}

/* Reinitialize the cursor position in the answer. */
void put_cursor_at_end_of_answer(void)
{
	typing_x = HIGHEST_POSITIVE;
}

/* Redraw the prompt bar and place the cursor at the right spot. */
void draw_the_promptbar(void)
{
	size_t base = breadth(prompt) + 2;
	size_t column = base + wideness(answer, typing_x);
	size_t the_page, end_page;
	char *expanded;

	the_page = get_statusbar_page_start(base, column);
	end_page = get_statusbar_page_start(base, base + breadth(answer) - 1);

	/* Color the prompt bar over its full width. */
	wattron(footwin, interface_color_pair[PROMPT_BAR]);
	mvwprintw(footwin, 0, 0, "%*s", COLS, " ");

	mvwaddstr(footwin, 0, 0, prompt);
	waddch(footwin, ':');
	waddch(footwin, (the_page == 0) ? ' ' : '<');

	expanded = display_string(answer, the_page, COLS - base, FALSE, TRUE);
	waddstr(footwin, expanded);
	free(expanded);

	if (the_page < end_page && base + breadth(answer) - the_page > COLS)
		mvwaddch(footwin, 0, COLS - 1, '>');

	wattroff(footwin, interface_color_pair[PROMPT_BAR]);

#if defined(NCURSES_VERSION_PATCH) && (NCURSES_VERSION_PATCH < 20210220)
	/* Work around a cursor-misplacement bug -- https://sv.gnu.org/bugs/?59808. */
	if (ISSET(NO_HELP)) {
		wmove(footwin, 0, 0);
		wrefresh(footwin);
	}
#endif

	/* Place the cursor at the right spot. */
	wmove(footwin, 0, column - the_page);

	wnoutrefresh(footwin);
}

#ifndef NANO_TINY
/* Remove or add the pipe character at the answer's head. */
void add_or_remove_pipe_symbol_from_answer(void)
{
	if (*answer == '|') {
		memmove(answer, answer + 1, strlen(answer));
		if (typing_x > 0)
			typing_x--;
	} else {
		answer = nrealloc(answer, strlen(answer) + 2);
		memmove(answer + 1, answer, strlen(answer) + 1);
		*answer = '|';
		typing_x++;
	}
}
#endif

/* Get a string of input at the status-bar prompt. */
functionptrtype acquire_an_answer(int *actual, bool *listed,
					linestruct **history_list, void (*refresh_func)(void))
{
#ifdef ENABLE_HISTORIES
	char *stored_string = NULL;
		/* Whatever the answer was before the user foraged into history. */
#ifdef ENABLE_TABCOMP
	bool previous_was_tab = FALSE;
		/* Whether the previous keystroke was an attempt at tab completion. */
	size_t fragment_length = 0;
		/* The length of the fragment that the user tries to tab complete. */
#endif
#endif
#ifndef NANO_TINY
	bool bracketed_paste = FALSE;
#endif
	const keystruct *shortcut;
	functionptrtype function;
	int input;

	if (typing_x > strlen(answer))
		typing_x = strlen(answer);

	while (TRUE) {
		draw_the_promptbar();

		/* Read in one keystroke. */
		input = get_kbinput(footwin, VISIBLE);

#ifndef NANO_TINY
		/* If the window size changed, go reformat the prompt string. */
		if (input == THE_WINDOW_RESIZED) {
			*actual = THE_WINDOW_RESIZED;
#ifdef ENABLE_HISTORIES
			free(stored_string);
#endif
			return NULL;
		}
		if (input == START_OF_PASTE || input == END_OF_PASTE)
			bracketed_paste = (input == START_OF_PASTE);
#endif
#ifdef ENABLE_MOUSE
		/* For a click on a shortcut, read in the resulting keycode. */
		if (input == KEY_MOUSE && process_prompt_click() == 1)
			input = get_kbinput(footwin, BLIND);
		if (input == KEY_MOUSE)
			continue;
#endif

		/* Check for a shortcut in the current list. */
		shortcut = get_shortcut(input);
		function = (shortcut ? shortcut->func : NULL);
#ifndef NANO_TINY
		/* Tabs in an external paste are not commands. */
		if (input == '\t' && bracketed_paste)
			function = NULL;
#endif
		/* When it's a normal character, add it to the answer. */
		absorb_character(input, function);

#ifndef NANO_TINY
		/* Ignore any commands inside an external paste. */
		if (bracketed_paste) {
			if (function && function != do_nothing)
				beep();
			continue;
		}
#endif

		if (function == do_cancel || function == do_enter)
			break;

#ifdef ENABLE_TABCOMP
		if (function == do_tab) {
#ifdef ENABLE_HISTORIES
			if (history_list) {
				if (!previous_was_tab)
					fragment_length = strlen(answer);

				if (fragment_length > 0) {
					answer = get_history_completion(history_list, answer, fragment_length);
					typing_x = strlen(answer);
				}
			} else
#endif
			/* Allow tab completion of filenames, but not in restricted mode. */
			if ((currmenu & (MINSERTFILE|MWRITEFILE|MGOTODIR)) && !ISSET(RESTRICTED))
				answer = input_tab(answer, &typing_x, refresh_func, listed);
		} else
#endif
#ifdef ENABLE_HISTORIES
		if (function == get_older_item && history_list) {
			/* If this is the first step into history, start at the bottom. */
			if (stored_string == NULL)
				reset_history_pointer_for(*history_list);

			/* When moving up from the bottom, remember the current answer. */
			if ((*history_list)->next == NULL)
				stored_string = mallocstrcpy(stored_string, answer);

			/* If there is an older item, move to it and copy its string. */
			if ((*history_list)->prev) {
				*history_list = (*history_list)->prev;
				answer = mallocstrcpy(answer, (*history_list)->data);
				typing_x = strlen(answer);
			}
		} else if (function == get_newer_item && history_list) {
			/* If there is a newer item, move to it and copy its string. */
			if ((*history_list)->next) {
				*history_list = (*history_list)->next;
				answer = mallocstrcpy(answer, (*history_list)->data);
				typing_x = strlen(answer);
			}

			/* When at the bottom of the history list, restore the old answer. */
			if ((*history_list)->next == NULL && stored_string && *answer == '\0') {
				answer = mallocstrcpy(answer, stored_string);
				typing_x = strlen(answer);
			}
		} else
#endif /* ENABLE_HISTORIES */
		if (function == do_help || function == full_refresh)
			function();
#ifndef NANO_TINY
		else if (function == do_toggle && shortcut->toggle == NO_HELP) {
			TOGGLE(NO_HELP);
			window_init();
			focusing = FALSE;
			refresh_func();
			bottombars(currmenu);
		} else if (function == do_nothing)
			;
#endif
#ifdef ENABLE_NANORC
		else if (function == (functionptrtype)implant)
			implant(shortcut->expansion);
#endif
		else if (function && !handle_editing(function)) {
			/* When it's a permissible shortcut, run it and done. */
			if (!ISSET(VIEW_MODE) || !changes_something(function)) {
#ifndef NANO_TINY
				/* When invoking a tool at the Execute prompt, stash an "answer". */
				if (currmenu == MEXECUTE)
					foretext = mallocstrcpy(foretext, answer);
#endif
				function();
				break;
			} else
				beep();
		}

#if defined(ENABLE_HISTORIES) && defined(ENABLE_TABCOMP)
		previous_was_tab = (function == do_tab);
#endif
	}

#ifndef NANO_TINY
	/* When an external command was run, clear a possibly stashed answer. */
	if (currmenu == MEXECUTE && function == do_enter)
		*foretext = '\0';
#endif
#ifdef ENABLE_HISTORIES
	/* If the history pointer was moved, point it at the bottom again. */
	if (stored_string) {
		reset_history_pointer_for(*history_list);
		free(stored_string);
	}
#endif

	*actual = input;

	return function;
}

/* Ask a question on the status bar.  Return 0 when text was entered,
 * -1 for a cancelled entry, -2 for a blank string, and the relevant
 * keycode when a valid shortcut key was pressed.  The 'provided'
 * parameter is the default answer for when simply Enter is typed. */
int do_prompt(int menu, const char *provided, linestruct **history_list,
				void (*refresh_func)(void), const char *msg, ...)
{
	functionptrtype function = NULL;
	bool listed = FALSE;
	va_list ap;
	int retval;
	/* Save a possible current status-bar x position and prompt. */
	size_t was_typing_x = typing_x;
	char *saved_prompt = prompt;

	bottombars(menu);

	if (answer != provided)
		answer = mallocstrcpy(answer, provided);

#ifndef NANO_TINY
  redo_theprompt:
#endif
	prompt = nmalloc((COLS * MAXCHARLEN) + 1);
	va_start(ap, msg);
	vsnprintf(prompt, COLS * MAXCHARLEN, msg, ap);
	va_end(ap);
	/* Reserve five columns for colon plus angles plus answer, ":<aa>". */
	prompt[actual_x(prompt, (COLS < 5) ? 0 : COLS - 5)] = '\0';

	lastmessage = VACUUM;

	function = acquire_an_answer(&retval, &listed, history_list, refresh_func);
	free(prompt);

#ifndef NANO_TINY
	if (retval == THE_WINDOW_RESIZED)
		goto redo_theprompt;
#endif

	/* Restore a possible previous prompt and maybe the typing position. */
	prompt = saved_prompt;
	if (function == do_cancel || function == do_enter ||
#ifdef ENABLE_BROWSER
				function == to_first_file || function == to_last_file ||
#endif
				function == to_first_line || function == to_last_line)
		typing_x = was_typing_x;

	/* Set the proper return value for Cancel and Enter. */
	if (function == do_cancel)
		retval = -1;
	else if (function == do_enter)
		retval = (*answer == '\0') ? -2 : 0;

	if (lastmessage == VACUUM)
		wipe_statusbar();

#ifdef ENABLE_TABCOMP
	/* If possible filename completions are still listed, clear them off. */
	if (listed)
		refresh_func();
#endif

	return retval;
}

#define UNDECIDED  -2

/* Ask a simple Yes/No (and optionally All) question on the status bar
 * and return the choice -- either YES or NO or ALL or CANCEL. */
int ask_user(bool withall, const char *question)
{
	int choice = UNDECIDED;
	int width = 16;
	/* TRANSLATORS: For the next three strings, specify the starting letters
	 * of the translations for "Yes"/"No"/"All".  The first letter of each of
	 * these strings MUST be a single-byte letter; others may be multi-byte. */
	const char *yesstr = _("Yy");
	const char *nostr = _("Nn");
	const char *allstr = _("Aa");
	const keystruct *shortcut;
	functionptrtype function;

	while (choice == UNDECIDED) {
#ifdef ENABLE_NLS
		char letter[MAXCHARLEN + 1];
		int index = 0;
#endif
		int kbinput;

		if (!ISSET(NO_HELP)) {
			char shortstr[MAXCHARLEN + 2];
				/* Temporary string for (translated) " Y", " N" and " A". */
			const keystruct *cancelshortcut = first_sc_for(MYESNO, do_cancel);
				/* The keystroke that is bound to the Cancel function. */

			if (COLS < 32)
				width = COLS / 2;

			/* Clear the shortcut list from the bottom of the screen. */
			blank_bottombars();

			/* Now show the ones for "Yes", "No", "Cancel" and maybe "All". */
			sprintf(shortstr, " %c", yesstr[0]);
			wmove(footwin, 1, 0);
			post_one_key(shortstr, _("Yes"), width);

			shortstr[1] = nostr[0];
			wmove(footwin, 2, 0);
			post_one_key(shortstr, _("No"), width);

			if (withall) {
				shortstr[1] = allstr[0];
				wmove(footwin, 1, width);
				post_one_key(shortstr, _("All"), width);
			}

			wmove(footwin, 2, width);
			post_one_key(cancelshortcut->keystr, _("Cancel"), width);
		}

		/* Color the prompt bar over its full width and display the question. */
		wattron(footwin, interface_color_pair[PROMPT_BAR]);
		mvwprintw(footwin, 0, 0, "%*s", COLS, " ");
		mvwaddnstr(footwin, 0, 0, question, actual_x(question, COLS - 1));
		wattroff(footwin, interface_color_pair[PROMPT_BAR]);
		wnoutrefresh(footwin);

		currmenu = MYESNO;

		/* When not replacing, show the cursor while waiting for a key. */
		kbinput = get_kbinput(footwin, !withall);

#ifndef NANO_TINY
		if (kbinput == THE_WINDOW_RESIZED)
			continue;

		/* Accept first character of an external paste and ignore the rest. */
		if (kbinput == START_OF_PASTE) {
			kbinput = get_kbinput(footwin, BLIND);
			while (get_kbinput(footwin, BLIND) != END_OF_PASTE)
				;
		}
#endif

#ifdef ENABLE_NLS
		letter[index++] = (unsigned char)kbinput;
#ifdef ENABLE_UTF8
		/* If the received code is a UTF-8 starter byte, get also the
		 * continuation bytes and assemble them into one letter. */
		if (0xC0 <= kbinput && kbinput <= 0xF7 && using_utf8) {
			int extras = (kbinput / 16) % 4 + (kbinput <= 0xCF ? 1 : 0);

			while (extras <= waiting_keycodes() && extras-- > 0)
				letter[index++] = (unsigned char)get_kbinput(footwin, !withall);
		}
#endif
		letter[index] = '\0';

		/* See if the typed letter is in the Yes, No, or All strings. */
		if (strstr(yesstr, letter))
			choice = YES;
		else if (strstr(nostr, letter))
			choice = NO;
		else if (withall && strstr(allstr, letter))
			choice = ALL;
		else
#endif /* ENABLE_NLS */
		if (strchr("Yy", kbinput))
			choice = YES;
		else if (strchr("Nn", kbinput))
			choice = NO;
		else if (withall && strchr("Aa", kbinput))
			choice = ALL;

		if (choice != UNDECIDED)
			break;

		shortcut = get_shortcut(kbinput);
		function = (shortcut ? shortcut->func : NULL);

		if (function == do_cancel)
			choice = CANCEL;
		else if (function == full_refresh)
			full_refresh();
#ifndef NANO_TINY
		else if (function == do_toggle && shortcut->toggle == NO_HELP) {
			TOGGLE(NO_HELP);
			window_init();
			titlebar(NULL);
			focusing = FALSE;
			edit_refresh();
			focusing = TRUE;
		}
#endif
		/* Interpret ^N as "No", to allow exiting in anger, and ^Q or ^X too. */
		else if (kbinput == '\x0E' || (kbinput == '\x11' && !ISSET(MODERN_BINDINGS)) ||
									  (kbinput == '\x18' && ISSET(MODERN_BINDINGS))) {
			choice = NO;
			if (kbinput != '\x0E')  /* ^X^Q makes nano exit with an error. */
				final_status = 2;
		/* Also, interpret ^Y as "Yes, and  ^A as "All". */
		} else if (kbinput == '\x19')
			choice = YES;
		else if (kbinput == '\x01' && withall)
			choice = ALL;
#ifdef ENABLE_MOUSE
		else if (kbinput == KEY_MOUSE) {
			int mouse_x, mouse_y;
			/* We can click on the Yes/No/All shortcuts to select an answer. */
			if (get_mouseinput(&mouse_y, &mouse_x) == 0 &&
						wmouse_trafo(footwin, &mouse_y, &mouse_x, FALSE) &&
						mouse_x < (width * 2) && mouse_y > 0) {
				int x = mouse_x / width;
				int y = mouse_y - 1;

				/* x == 0 means Yes or No, y == 0 means Yes or All. */
				choice = -2 * x * y + x - y + 1;

				if (choice == ALL && !withall)
					choice = UNDECIDED;
			}
		}
#endif
		else
			beep();
	}

	return choice;
}

/* ======================== Command Palette ======================== */

typedef struct {
	void (*func)(void);
	char *name;
	char *shortcut;
	char *desc;
} CommandEntry;

static bool palette_casestr(const char *haystack, const char *needle)
{
	if (!needle || !*needle)
		return TRUE;
	if (!haystack || !*haystack)
		return FALSE;

	size_t nlen = strlen(needle);
	size_t hlen = strlen(haystack);
	if (nlen > hlen)
		return FALSE;

	for (size_t i = 0; i <= hlen - nlen; i++) {
		if (strncasecmp(haystack + i, needle, nlen) == 0)
			return TRUE;
	}
	return FALSE;
}

/* Display a searchable command palette window (like Ctrl+Shift+P in VS Code)
 * that lists available commands, their shortcuts, and descriptions. */
void do_command_palette(void)
{
	CommandEntry entries[128];
	int total_entries = 0;

	/* Collect MMAIN functions from allfuncs. */
	for (funcstruct *f = allfuncs; f != NULL && total_entries < 120; f = f->next) {
		if ((f->menus & MMAIN) && f->func != NULL && f->func != do_cancel &&
				f->func != do_command_palette) {
			bool duplicate = FALSE;
			for (int k = 0; k < total_entries; k++) {
				if (entries[k].func == f->func) {
					duplicate = TRUE;
					break;
				}
			}
			if (duplicate)
				continue;

			const keystruct *sc = first_sc_for(MMAIN, f->func);
			const char *tag = f->tag ? f->tag : "";
			const char *phrase = "";
#ifdef ENABLE_HELP
			if (f->phrase && f->phrase[0] && strcmp(f->phrase, "x") != 0)
				phrase = f->phrase;
#endif
			/* Fallbacks for clear descriptions */
			if (!phrase || !*phrase) {
				if (f->func == do_savefile)
					phrase = _("Save current file to disk");
				else if (f->func == do_open_in_new_buffer)
					phrase = _("Open a file in a new tab");
				else if (f->func == do_new_buffer)
					phrase = _("Create a new untitled buffer");
				else if (f->func == do_exit)
					phrase = _("Close buffer / Exit editor");
				else if (f->func == do_quit)
					phrase = _("Exit nano immediately");
				else if (f->func == cut_text)
					phrase = _("Cut current line or selection");
				else if (f->func == copy_text)
					phrase = _("Copy current line or selection");
				else if (f->func == paste_text)
					phrase = _("Paste text from clipboard");
				else if (f->func == do_undo)
					phrase = _("Undo the last editing action");
				else if (f->func == do_redo)
					phrase = _("Redo the last undone action");
#ifdef ENABLE_BROWSER
				else if (f->func == explorer_toggle)
					phrase = _("Show or hide the file sidebar");
				else if (f->func == do_workspace_select)
					phrase = _("Change explorer workspace directory");
#endif
#ifdef ENABLE_MULTIBUFFER
				else if (f->func == switch_to_next_buffer)
					phrase = _("Switch to the next tab");
				else if (f->func == switch_to_prev_buffer)
					phrase = _("Switch to the previous tab");
#endif
				else
					phrase = tag;
			}

			entries[total_entries].func = f->func;
			entries[total_entries].name = copy_of(tag);
			entries[total_entries].shortcut = copy_of(sc ? sc->keystr : "");
			entries[total_entries].desc = copy_of(phrase);
			total_entries++;
		}
	}

	int pwidth = (COLS > 76) ? 74 : (COLS - 4);
	if (pwidth < 28)
		pwidth = COLS;
	int pheight = (LINES > 20) ? 15 : (LINES - 4);
	if (pheight < 6)
		pheight = LINES;
	int start_y = (LINES > pheight) ? 1 : 0;
	int start_x = (COLS > pwidth) ? (COLS - pwidth) / 2 : 0;

	WINDOW *palwin = newwin(pheight, pwidth, start_y, start_x);
	if (palwin == NULL) {
		for (int i = 0; i < total_entries; i++) {
			free(entries[i].name);
			free(entries[i].shortcut);
			free(entries[i].desc);
		}
		return;
	}

	keypad(palwin, TRUE);
	wtimeout(palwin, -1);

	char query[128] = "";
	int query_len = 0;
	int selected_idx = 0;
	int scroll_offset = 0;
	void (*chosen_func)(void) = NULL;

	while (TRUE) {
		int matches[128];
		int match_count = 0;

		for (int i = 0; i < total_entries; i++) {
			if (query_len == 0 ||
					palette_casestr(entries[i].name, query) ||
					palette_casestr(entries[i].shortcut, query) ||
					palette_casestr(entries[i].desc, query)) {
				matches[match_count++] = i;
			}
		}

		if (selected_idx >= match_count)
			selected_idx = (match_count > 0) ? match_count - 1 : 0;
		if (selected_idx < scroll_offset)
			scroll_offset = selected_idx;
		int list_rows = pheight - 4;
		if (list_rows <= 0)
			list_rows = 1;
		if (selected_idx >= scroll_offset + list_rows)
			scroll_offset = selected_idx - list_rows + 1;

		/* Redraw palette window */
		werase(palwin);
		box(palwin, 0, 0);

		wattron(palwin, A_BOLD);
		mvwprintw(palwin, 0, 2, " Nonte Command Palette ");
		mvwprintw(palwin, 1, 2, "> %s", query);
		wattroff(palwin, A_BOLD);

		/* Input cursor indicator */
		wattron(palwin, A_REVERSE);
		waddch(palwin, ' ');
		wattroff(palwin, A_REVERSE);

		/* Divider line */
		wmove(palwin, 2, 1);
		whline(palwin, ACS_HLINE, pwidth - 2);

		/* List items */
		for (int r = 0; r < list_rows; r++) {
			int m = scroll_offset + r;
			int row = 3 + r;
			if (m >= match_count)
				break;

			CommandEntry *cmd = &entries[matches[m]];
			bool is_sel = (m == selected_idx);

			int name_w = 20;
			int sc_w = 8;
			int desc_w = pwidth - 4 - name_w - sc_w - 2;
			if (desc_w < 5)
				desc_w = 5;

			char name_buf[64], sc_buf[32], desc_buf[160];
			snprintf(name_buf, sizeof(name_buf), "%-*.*s", name_w, name_w, cmd->name);
			snprintf(sc_buf, sizeof(sc_buf), "%-*.*s", sc_w, sc_w, cmd->shortcut);
			snprintf(desc_buf, sizeof(desc_buf), "%-*.*s", desc_w, desc_w, cmd->desc);

			if (is_sel)
				wattron(palwin, A_REVERSE | A_BOLD);

			mvwprintw(palwin, row, 2, "%s %s %s", name_buf, sc_buf, desc_buf);

			if (is_sel)
				wattroff(palwin, A_REVERSE | A_BOLD);
		}

		/* Bottom status */
		if (match_count > 0)
			mvwprintw(palwin, pheight - 1, 2, " %i/%i [Enter: Run, Esc: Close] ",
					selected_idx + 1, match_count);
		else
			mvwprintw(palwin, pheight - 1, 2, " No matching commands [Esc: Close] ");

		wrefresh(palwin);

		int ch = wgetch(palwin);

		if (ch == 27 || ch == 3 || ch == 7) { /* Esc, Ctrl+C, Ctrl+G */
			chosen_func = NULL;
			break;
		} else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
			if (match_count > 0)
				chosen_func = entries[matches[selected_idx]].func;
			break;
		} else if (ch == KEY_UP || ch == 16) { /* Up, Ctrl+P */
			if (selected_idx > 0)
				selected_idx--;
		} else if (ch == KEY_DOWN || ch == 14) { /* Down, Ctrl+N */
			if (selected_idx + 1 < match_count)
				selected_idx++;
		} else if (ch == KEY_PPAGE) {
			selected_idx -= list_rows;
			if (selected_idx < 0)
				selected_idx = 0;
		} else if (ch == KEY_NPAGE) {
			selected_idx += list_rows;
			if (selected_idx >= match_count)
				selected_idx = (match_count > 0) ? match_count - 1 : 0;
		} else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b' || ch == 8) {
			if (query_len > 0) {
				query[--query_len] = '\0';
				selected_idx = 0;
				scroll_offset = 0;
			}
		} else if (ch == 21) { /* Ctrl+U clears query */
			query[0] = '\0';
			query_len = 0;
			selected_idx = 0;
			scroll_offset = 0;
		} else if (ch >= 0x20 && ch <= 0x7E) {
			if (query_len < (int)sizeof(query) - 2) {
				query[query_len++] = (char)ch;
				query[query_len] = '\0';
				selected_idx = 0;
				scroll_offset = 0;
			}
		}
	}

	for (int i = 0; i < total_entries; i++) {
		free(entries[i].name);
		free(entries[i].shortcut);
		free(entries[i].desc);
	}

	delwin(palwin);
	full_refresh();
	edit_refresh();

	if (chosen_func)
		chosen_func();
}

/* Feature 5: Quick open file by name (Ctrl+P). */
void do_quick_open(void)
{
	char *files[512];
	int file_count = 0;

	FILE *fp = popen("find . -maxdepth 5 -not -path '*/.*' -type f 2>/dev/null", "r");
	if (fp != NULL) {
		char linebuf[512];
		while (file_count < 500 && fgets(linebuf, sizeof(linebuf), fp)) {
			size_t l = strlen(linebuf);
			while (l > 0 && (linebuf[l - 1] == '\r' || linebuf[l - 1] == '\n'))
				linebuf[--l] = '\0';
			const char *p = linebuf;
			if (p[0] == '.' && p[1] == '/')
				p += 2;
			if (*p)
				files[file_count++] = copy_of(p);
		}
		pclose(fp);
	}

	if (file_count == 0) {
		statusline(AHEM, _("No files found in workspace"));
		return;
	}

	int pwidth = (COLS > 76) ? 74 : (COLS - 4);
	if (pwidth < 28)
		pwidth = COLS;
	int pheight = (LINES > 20) ? 15 : (LINES - 4);
	if (pheight < 6)
		pheight = LINES;
	int start_y = (LINES > pheight) ? 1 : 0;
	int start_x = (COLS > pwidth) ? (COLS - pwidth) / 2 : 0;

	WINDOW *palwin = newwin(pheight, pwidth, start_y, start_x);
	if (palwin == NULL) {
		for (int i = 0; i < file_count; i++)
			free(files[i]);
		return;
	}

	keypad(palwin, TRUE);
	wtimeout(palwin, -1);

	char query[128] = "";
	int query_len = 0;
	int selected_idx = 0;
	int scroll_offset = 0;
	char *chosen_file = NULL;

	while (TRUE) {
		int matches[512];
		int match_count = 0;

		for (int i = 0; i < file_count; i++) {
			if (query_len == 0 || palette_casestr(files[i], query))
				matches[match_count++] = i;
		}

		if (selected_idx >= match_count)
			selected_idx = (match_count > 0) ? match_count - 1 : 0;
		if (selected_idx < scroll_offset)
			scroll_offset = selected_idx;
		int list_rows = pheight - 4;
		if (list_rows <= 0)
			list_rows = 1;
		if (selected_idx >= scroll_offset + list_rows)
			scroll_offset = selected_idx - list_rows + 1;

		werase(palwin);
		box(palwin, 0, 0);

		wattron(palwin, A_BOLD);
		mvwprintw(palwin, 0, 2, " Quick Open File (Ctrl+P) ");
		mvwprintw(palwin, 1, 2, "> %s", query);
		wattroff(palwin, A_BOLD);

		wattron(palwin, A_REVERSE);
		waddch(palwin, ' ');
		wattroff(palwin, A_REVERSE);

		wmove(palwin, 2, 1);
		whline(palwin, ACS_HLINE, pwidth - 2);

		for (int r = 0; r < list_rows; r++) {
			int m = scroll_offset + r;
			int row = 3 + r;
			if (m >= match_count)
				break;

			const char *fname = files[matches[m]];
			bool is_sel = (m == selected_idx);

			if (is_sel)
				wattron(palwin, A_REVERSE | A_BOLD);

			mvwprintw(palwin, row, 2, "%-*.*s", pwidth - 4, pwidth - 4, fname);

			if (is_sel)
				wattroff(palwin, A_REVERSE | A_BOLD);
		}

		if (match_count > 0)
			mvwprintw(palwin, pheight - 1, 2, " %i/%i [Enter: Open, Esc: Close] ",
					selected_idx + 1, match_count);
		else
			mvwprintw(palwin, pheight - 1, 2, " No matching files [Esc: Close] ");

		wrefresh(palwin);

		int ch = wgetch(palwin);

		if (ch == 27 || ch == 3 || ch == 7) {
			chosen_file = NULL;
			break;
		} else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
			if (match_count > 0)
				chosen_file = copy_of(files[matches[selected_idx]]);
			break;
		} else if (ch == KEY_UP || ch == 16) {
			if (selected_idx > 0)
				selected_idx--;
		} else if (ch == KEY_DOWN || ch == 14) {
			if (selected_idx + 1 < match_count)
				selected_idx++;
		} else if (ch == KEY_PPAGE) {
			selected_idx -= list_rows;
			if (selected_idx < 0)
				selected_idx = 0;
		} else if (ch == KEY_NPAGE) {
			selected_idx += list_rows;
			if (selected_idx >= match_count)
				selected_idx = (match_count > 0) ? match_count - 1 : 0;
		} else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b' || ch == 8) {
			if (query_len > 0) {
				query[--query_len] = '\0';
				selected_idx = 0;
				scroll_offset = 0;
			}
		} else if (ch == 21) {
			query[0] = '\0';
			query_len = 0;
			selected_idx = 0;
			scroll_offset = 0;
		} else if (ch >= 0x20 && ch <= 0x7E) {
			if (query_len < (int)sizeof(query) - 2) {
				query[query_len++] = (char)ch;
				query[query_len] = '\0';
				selected_idx = 0;
				scroll_offset = 0;
			}
		}
	}

	for (int i = 0; i < file_count; i++)
		free(files[i]);

	delwin(palwin);
	full_refresh();
	edit_refresh();

	if (chosen_file) {
		open_buffer(chosen_file, TRUE);
		free(chosen_file);
	}
}

typedef struct GrepMatch {
	char *filepath;
	ssize_t lineno;
	char *content;
} GrepMatch;

/* Feature 9: Project-Wide Grep / Find in Files (Ctrl+Shift+F). */
void do_find_in_files(void)
{
	int pwidth = (COLS > 84) ? 80 : (COLS - 4);
	if (pwidth < 28)
		pwidth = COLS;
	int pheight = (LINES > 20) ? 16 : (LINES - 4);
	if (pheight < 6)
		pheight = LINES;
	int start_y = (LINES > pheight) ? 1 : 0;
	int start_x = (COLS > pwidth) ? (COLS - pwidth) / 2 : 0;

	WINDOW *palwin = newwin(pheight, pwidth, start_y, start_x);
	if (palwin == NULL)
		return;

	keypad(palwin, TRUE);
	wtimeout(palwin, -1);

	char query[128] = "";
	int query_len = 0;
	int selected_idx = 0;
	int scroll_offset = 0;

	GrepMatch matches[200];
	int match_count = 0;
	char last_searched[128] = "";

	while (TRUE) {
		if (strcmp(query, last_searched) != 0) {
			for (int i = 0; i < match_count; i++) {
				free(matches[i].filepath);
				free(matches[i].content);
			}
			match_count = 0;
			selected_idx = 0;
			scroll_offset = 0;
			snprintf(last_searched, sizeof(last_searched), "%s", query);

			if (query_len >= 2) {
				char cmd[512];
				snprintf(cmd, sizeof(cmd), "grep -rn -I --exclude-dir=.git --exclude-dir=node_modules -m 150 -e \"%s\" . 2>/dev/null", query);
				FILE *fp = popen(cmd, "r");
				if (fp != NULL) {
					char linebuf[512];
					while (match_count < 150 && fgets(linebuf, sizeof(linebuf), fp)) {
						size_t l = strlen(linebuf);
						while (l > 0 && (linebuf[l - 1] == '\r' || linebuf[l - 1] == '\n'))
							linebuf[--l] = '\0';
						char *fstart = linebuf;
						if (fstart[0] == '.' && fstart[1] == '/')
							fstart += 2;
						char *colon1 = strchr(fstart, ':');
						if (!colon1)
							continue;
						*colon1 = '\0';
						char *lstart = colon1 + 1;
						char *colon2 = strchr(lstart, ':');
						if (!colon2)
							continue;
						*colon2 = '\0';
						char *text = colon2 + 1;
						while (*text == ' ' || *text == '\t')
							text++;

						matches[match_count].filepath = copy_of(fstart);
						matches[match_count].lineno = atol(lstart);
						matches[match_count].content = copy_of(text);
						match_count++;
					}
					pclose(fp);
				}
			}
		}

		if (selected_idx >= match_count)
			selected_idx = (match_count > 0) ? match_count - 1 : 0;
		if (selected_idx < scroll_offset)
			scroll_offset = selected_idx;
		int list_rows = pheight - 4;
		if (list_rows <= 0)
			list_rows = 1;
		if (selected_idx >= scroll_offset + list_rows)
			scroll_offset = selected_idx - list_rows + 1;

		werase(palwin);
		box(palwin, 0, 0);

		wattron(palwin, A_BOLD);
		mvwprintw(palwin, 0, 2, " Find in Files (Ctrl+Shift+F) ");
		mvwprintw(palwin, 1, 2, "> %s", query);
		wattroff(palwin, A_BOLD);

		wattron(palwin, A_REVERSE);
		waddch(palwin, ' ');
		wattroff(palwin, A_REVERSE);

		wmove(palwin, 2, 1);
		whline(palwin, ACS_HLINE, pwidth - 2);

		for (int r = 0; r < list_rows; r++) {
			int m = scroll_offset + r;
			int row = 3 + r;
			if (m >= match_count)
				break;

			GrepMatch *gm = &matches[m];
			bool is_sel = (m == selected_idx);

			char display_item[256];
			snprintf(display_item, sizeof(display_item), "%s:%zd: %s", gm->filepath, gm->lineno, gm->content);

			if (is_sel)
				wattron(palwin, A_REVERSE | A_BOLD);

			mvwprintw(palwin, row, 2, "%-*.*s", pwidth - 4, pwidth - 4, display_item);

			if (is_sel)
				wattroff(palwin, A_REVERSE | A_BOLD);
		}

		if (match_count > 0)
			mvwprintw(palwin, pheight - 1, 2, " %i/%i [Enter: Jump, Esc: Close] ",
					selected_idx + 1, match_count);
		else if (query_len < 2)
			mvwprintw(palwin, pheight - 1, 2, " Type at least 2 chars to search [Esc: Close] ");
		else
			mvwprintw(palwin, pheight - 1, 2, " No matches found [Esc: Close] ");

		wrefresh(palwin);

		int ch = wgetch(palwin);

		if (ch == 27 || ch == 3 || ch == 7) {
			break;
		} else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
			if (match_count > 0) {
				char *fp_copy = copy_of(matches[selected_idx].filepath);
				ssize_t target_line = matches[selected_idx].lineno;
				for (int i = 0; i < match_count; i++) {
					free(matches[i].filepath);
					free(matches[i].content);
				}
				delwin(palwin);
				full_refresh();
				edit_refresh();
				open_buffer(fp_copy, TRUE);
				goto_line_posx(target_line, 0);
				statusline(INFO, _("Jumped to %s:%zd"), fp_copy, target_line);
				free(fp_copy);
				return;
			}
			break;
		} else if (ch == KEY_UP || ch == 16) {
			if (selected_idx > 0)
				selected_idx--;
		} else if (ch == KEY_DOWN || ch == 14) {
			if (selected_idx + 1 < match_count)
				selected_idx++;
		} else if (ch == KEY_PPAGE) {
			selected_idx -= list_rows;
			if (selected_idx < 0)
				selected_idx = 0;
		} else if (ch == KEY_NPAGE) {
			selected_idx += list_rows;
			if (selected_idx >= match_count)
				selected_idx = (match_count > 0) ? match_count - 1 : 0;
		} else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b' || ch == 8) {
			if (query_len > 0) {
				query[--query_len] = '\0';
				selected_idx = 0;
				scroll_offset = 0;
			}
		} else if (ch == 21) {
			query[0] = '\0';
			query_len = 0;
			selected_idx = 0;
			scroll_offset = 0;
		} else if (ch >= 0x20 && ch <= 0x7E) {
			if (query_len < (int)sizeof(query) - 2) {
				query[query_len++] = (char)ch;
				query[query_len] = '\0';
				selected_idx = 0;
				scroll_offset = 0;
			}
		}
	}

	for (int i = 0; i < match_count; i++) {
		free(matches[i].filepath);
		free(matches[i].content);
	}

	delwin(palwin);
	full_refresh();
	edit_refresh();
}
