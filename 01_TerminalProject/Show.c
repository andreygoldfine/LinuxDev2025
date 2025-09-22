#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    NUM_ARGS = 2,
    PROG_ARG = 0,
    FILE_ARG = 1,

    DX = 2,
    DY = 2,

    ESC = 27,

    MIN_CAPACITY = 16,
};

int min(int a, int b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

int main(int argc, char *argv[])
{
    /* checking the arguments */
    if (argc < NUM_ARGS) {
        printf("File name is missing. Use %s <filename>.\n", argv[PROG_ARG]);
        exit(1);
    } else if (argc > NUM_ARGS) {
        printf("Too many arguments. Use %s <filename>.\n", argv[PROG_ARG]);
        exit(1);
    }

    /* trying to open the file */
    FILE *file = fopen(argv[FILE_ARG], "r");
    if (NULL == file) {
        printf("Cannot open the file.");
        exit(1);
    }

    /* some ncurses initialization */
    WINDOW *frame, *win;
    int c = 0;
    initscr();
    noecho();
    cbreak();
    refresh();

    /* drawing the frame */
    size_t frame_height = LINES - 2*DY;
    size_t frame_width = COLS - 2*DX;
    frame = newwin(frame_height, frame_width, DY, DX);
    box(frame, 0, 0);
    mvwaddstr(frame, 0, 3, argv[FILE_ARG]);
    wrefresh(frame);

    /* creating the main window */
    size_t win_height = frame_height - 2;
    size_t win_width = frame_width - 2;
    win = newwin(win_height, win_width, DY+1, DX+1);
    keypad(win, TRUE);
    scrollok(win, TRUE);

    /* MALLOC: allocate memory to store lines */
    size_t lines_cap = MIN_CAPACITY;
    char **lines = malloc(lines_cap * sizeof(char *));

    size_t lines_size = 0;
    size_t max_line_len = 0;
    size_t line_len = 0;
    size_t written_bytes;

    /* MALLOC: memory allocation happens in getline (buffer ptr is NULL) */
    while ((written_bytes = getline(&lines[lines_size], &line_len, file)) != (size_t)-1) {
        lines_size += 1;
        if (lines_size == lines_cap) {
            lines_cap *= 2;
            lines = realloc(lines, lines_cap * sizeof(char *));
            for (size_t i = lines_size; i < lines_cap; ++i) {
                lines[i] = NULL;
            }
        }
        if (written_bytes > max_line_len) {
            max_line_len = written_bytes;
        }
    }

    size_t row = 0;
    size_t col = 0;
    int escape = FALSE;

    /* redrawing window after each action */
    while (TRUE) {
        werase(win);
        size_t row_limit = min(row + frame_height - DY + 1, lines_size) - 1;
        for (size_t i = row; i < row_limit; ++i) {
            size_t col_limit = min(col + frame_width - DX, strlen(lines[i])) - 1;
            for (size_t j = col; j < col_limit; ++j) {
                wprintw(win, "%c", lines[i][j]);
            }
            wprintw(win, "\n");
        }
        wrefresh(win);

        /* actions handling */
        switch (c = wgetch(win)) {
            case KEY_DOWN:
            case 'j':
                if (row < lines_size - 1) {
                    row += 1;
                }
                break;
            case KEY_UP:
            case 'k':
                if (row > 0) {
                    row -= 1;
                }
                break;
            case KEY_RIGHT:
            case 'l':
                if (col < max_line_len - 2) {
                    col += 1;
                }
                break;
            case KEY_LEFT:
            case 'h':
                if (col > 0) {
                    col -= 1;
                }
                break;
            case ' ':
            case ESC:
            case 'q':
                escape = TRUE;
                break;
        }

        if (escape) {
            break;
        }
    }

    /* removing the windows */
    delwin(frame);
    delwin(win);
    endwin();

    /* FREE: free the allocated buffers */
    for (size_t i = 0; i < lines_cap; ++i) {
        free(lines[i]);
    }
    free(lines);

    return 0;
}
