/*
  Beginner-friendly console Chess (C99)

  PHASE 1:
    - Board representation using arrays
    - Standard setup
    - Custom setup (user input)
    - Side selection (w/b) for viewing perspective
    - Board display with coordinates

  PHASE 2:
    - Move input like: e2 e4
    - Basic movement validation for: Pawn, Rook, Bishop, Knight, Queen, King

  PHASE 3A:
    - Pawn promotion (Q/R/B/N) when a pawn reaches the last rank

  PHASE 3B: CASTLING
    - King moves 2 squares left/right, rook moves accordingly
    - Ignores check rules (simplified)

  NOT implemented (by requirement):
    - En passant

  Compile (GCC / MinGW):
    gcc -std=c99 -Wall -Wextra -pedantic chess.c -o chess
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define BOARD_SIZE 8
#define MAX_HISTORY 1000
#define MAX_MOVES 1000

/* Forward declaration required before best_move uses copy_board. */
void copy_board(char dest[BOARD_SIZE][BOARD_SIZE], char src[BOARD_SIZE][BOARD_SIZE]);

/*
  =========================
  PHASE LABELING
  =========================
  This file currently implements a complete console chess game.

  Phase 1 includes:
    - Board representation (8x8 char array)
    - Standard setup
    - Custom setup (user input)
    - Side selection (w/b) for viewing perspective
    - Board display with coordinates

  Phase 2 includes:
    - Move input like: e2 e4
    - Basic movement validation (no special rules)

  Phase 3A includes:
    - Pawn promotion (Q/R/B/N)

  Phase 3B includes:
    - Castling (no check rules)

  Phase 4 includes:
    - Check detection
    - Move safety (cannot move into check)
    - Checkmate + stalemate detection

  Phase 5 includes:
    - Help command (suggest a legal move)

  Phase 6 includes:
    - Undo (restore previous state)

  Phase 7 includes:
    - Move history display
*/
/* FINAL PHASE: COMPLETE CHESS GAME */
#define CURRENT_PHASE 7

/*
  =========================
PHASE 1: BOARD & SETUP
  =========================
*/

/*
  clear_board (Phase 1)
  Sets every square to '.' meaning empty.
*/
void clear_board(char board[BOARD_SIZE][BOARD_SIZE])
{
    int r, c;
    for (r = 0; r < BOARD_SIZE; r++) {
        for (c = 0; c < BOARD_SIZE; c++) {
            board[r][c] = '.';
        }
    }
}

/*
  standard_setup (Phase 1)
  Places pieces into the normal chess starting position.
  We store pieces as characters:
    White: 'P','N','B','R','Q','K'
    Black: 'p','n','b','r','q','k'
*/
void standard_setup(char board[BOARD_SIZE][BOARD_SIZE])
{
    clear_board(board);

    /* Row 0 is rank 8, row 7 is rank 1 */
    /* Black pieces (rank 8 and 7) */
    board[0][0] = 'r'; board[0][1] = 'n'; board[0][2] = 'b'; board[0][3] = 'q';
    board[0][4] = 'k'; board[0][5] = 'b'; board[0][6] = 'n'; board[0][7] = 'r';
    {
        int c;
        for (c = 0; c < BOARD_SIZE; c++) board[1][c] = 'p';
    }

    /* White pieces (rank 1 and 2) */
    {
        int c;
        for (c = 0; c < BOARD_SIZE; c++) board[6][c] = 'P';
    }
    board[7][0] = 'R'; board[7][1] = 'N'; board[7][2] = 'B'; board[7][3] = 'Q';
    board[7][4] = 'K'; board[7][5] = 'B'; board[7][6] = 'N'; board[7][7] = 'R';
}

/*
  is_valid_piece_char (Phase 1)
  Returns 1 if ch is a valid piece letter or '.' for empty.
*/
int is_valid_piece_char(char ch)
{
    if (ch == '.') return 1;

    switch (ch) {
        case 'P': case 'N': case 'B': case 'R': case 'Q': case 'K':
        case 'p': case 'n': case 'b': case 'r': case 'q': case 'k':
            return 1;
        default:
            return 0;
    }
}

/*
  read_line (Phase 1)
  Reads a full line into buffer, removes trailing newline if present.
  Returns 1 if a line was read, 0 on EOF.
*/
int read_line(char buffer[], int size)
{
    if (fgets(buffer, size, stdin) == NULL) {
        return 0;
    }

    /* Remove trailing newline */
    {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';
        if (len > 1 && buffer[len - 2] == '\r') buffer[len - 2] = '\0';
    }

    return 1;
}

/*
  load_custom_setup (Phase 1)
  Lets the user enter 8 rows (rank 8 to rank 1) with 8 characters each.
  Example row: rnbqkbnr
               pppppppp
               ........

  Returns 1 if success, 0 if input ended.
*/
void print_board(char board[BOARD_SIZE][BOARD_SIZE], char perspective);

int load_custom_setup(char board[BOARD_SIZE][BOARD_SIZE])
{
    int r;
    char line[64];

    clear_board(board);
    printf("Custom setup: enter 8 lines for ranks 8 down to 1.\n");
    printf("Use: PNBRQK for White, pnbrqk for Black, '.' for empty.\n\n");

    print_board(board, 'w');

    for (r = 0; r < BOARD_SIZE; r++) {
        int ok = 0;
        while (!ok) {
            int c;
            int row = r; /* r=0 means rank 8 */
            int rank_number = 8 - r;

            printf("Rank %d: ", rank_number);
            if (!read_line(line, (int)sizeof(line))) return 0;

            if ((int)strlen(line) != BOARD_SIZE) {
                printf("Invalid: must be exactly 8 characters.\n");
                continue;
            }

            ok = 1;
            for (c = 0; c < BOARD_SIZE; c++) {
                if (!is_valid_piece_char(line[c])) {
                    ok = 0;
                    break;
                }
            }

            if (!ok) {
                printf("Invalid: use only PNBRQK/pnbrqk or '.'\n");
                continue;
            }

            for (c = 0; c < BOARD_SIZE; c++) {
                board[row][c] = line[c];
            }
        }
        print_board(board, 'w');
    }

    return 1;
}

/*
  =========================
  PHASE 1: DISPLAY
  =========================
*/

/*
  print_board (Phase 1)
  Prints the board with coordinates.
  perspective:
    'w' prints with rank 8 at top and files a->h left-to-right.
    'b' prints flipped (rank 1 at top, files h->a left-to-right).
*/
void print_board(char board[BOARD_SIZE][BOARD_SIZE], char perspective)
{
    int r, c;

    if (perspective == 'b') {
        printf("\n    h g f e d c b a\n");
        for (r = BOARD_SIZE - 1; r >= 0; r--) {
            int rank = 8 - r; /* when r=7, rank=1 */
            printf(" %d  ", rank);
            for (c = BOARD_SIZE - 1; c >= 0; c--) {
                printf("%c ", board[r][c]);
            }
            printf(" %d\n", rank);
        }
        printf("    h g f e d c b a\n\n");
    } else {
        printf("\n    a b c d e f g h\n");
        for (r = 0; r < BOARD_SIZE; r++) {
            int rank = 8 - r;
            printf(" %d  ", rank);
            for (c = 0; c < BOARD_SIZE; c++) {
                printf("%c ", board[r][c]);
            }
            printf(" %d\n", rank);
        }
        printf("    a b c d e f g h\n\n");
    }
}

/*
  =========================
  PHASE 1: USER PROMPTS
  =========================
*/

/*
  choose_side (Phase 1)
  Asks the user to choose viewing perspective using a single letter:
    - w for White
    - b for Black

  The side here is only used to flip the board display.
  Returns 'w' or 'b'.
*/
char choose_side(void)
{
    char line[32];
    while (1) {
        printf("Choose side (w=White, b=Black).\n");
        printf("Side: ");
        if (!read_line(line, (int)sizeof(line))) return 'w';

        /* Main requirement: choose between w and b */
        if (strcmp(line, "w") == 0 || strcmp(line, "W") == 0) return 'w';
        if (strcmp(line, "b") == 0 || strcmp(line, "B") == 0) return 'b';

        printf("Invalid. Type w or b.\n\n");
    }
}

/*
  choose_setup_mode (Phase 1)
  Ask if standard or custom setup, using a single letter:
    - s for Standard
    - c for Custom

  Returns 's' or 'c'.
*/
char choose_setup_mode(void)
{
    char line[32];
    while (1) {
        printf("Setup mode (s=Standard, c=Custom).\n");
        printf("Setup: ");
        if (!read_line(line, (int)sizeof(line))) return 's';

        if (strcmp(line, "s") == 0 || strcmp(line, "S") == 0) return 's';
        if (strcmp(line, "c") == 0 || strcmp(line, "C") == 0) return 'c';

        printf("Invalid. Type s or c.\n\n");
    }
}

/*
  choose_opponent (Phase 1)
  Asks the user if they want to play against the computer.
  Returns 'y' or 'n'.
*/
char choose_opponent(void)
{
    char line[32];
    while (1) {
        printf("Play against computer? (y=Yes, n=No).\n");
        printf("Choice: ");
        if (!read_line(line, (int)sizeof(line))) return 'n';

        if (strcmp(line, "y") == 0 || strcmp(line, "Y") == 0) return 'y';
        if (strcmp(line, "n") == 0 || strcmp(line, "N") == 0) return 'n';

        printf("Invalid. Type y or n.\n\n");
    }
}

/*
  =========================
  PHASE 2: MOVE INPUT + MOVEMENT VALIDATION
  =========================
  Phase 2 adds the ability to type moves like: e2 e4
  and checks basic piece movement.
*/

/*
  abs_int (Phase 2)
  Returns the absolute value of an int.
  This is used to compare move distances like "2 squares" or "1 square".
*/
int abs_int(int x)
{
    if (x < 0) return -x;
    return x;
}

/*
  piece_color (Phase 2)
  Returns:
    - 'w' if the piece is White (uppercase letter)
    - 'b' if the piece is Black (lowercase letter)
    - '.' if the square is empty

  This helps us enforce turns and prevent capturing your own piece.
*/
char piece_color(char p)
{
    if (p == '.') return '.';
    if (isupper((unsigned char)p)) return 'w';
    return 'b';
}

/*
  parse_square (Phase 2)
  Converts a square like "e2" into (row, col) indices for our board array.

  Rules:
    - File letter: a..h -> col 0..7
    - Rank number: 1..8 -> row 7..0

  Examples:
    - "a1" -> row 7, col 0
    - "h8" -> row 0, col 7

  Returns 1 if valid, 0 if invalid.
*/
int parse_square(const char s[], int *out_row, int *out_col)
{
    char file;
    char rank;
    int col;
    int row;

    if ((int)strlen(s) != 2) return 0;

    file = (char)tolower((unsigned char)s[0]);
    rank = s[1];

    if (file < 'a' || file > 'h') return 0;
    if (rank < '1' || rank > '8') return 0;

    col = file - 'a';
    row = 8 - (rank - '0'); /* rank '8' -> row 0, rank '1' -> row 7 */

    *out_row = row;
    *out_col = col;
    return 1;
}

/*
  is_path_clear (Phase 2)
  For rook/bishop/queen moves, checks that every square BETWEEN
  (r1,c1) and (r2,c2) is empty.

  The caller should only use this when the move is already known to be
  straight (rook-like) or diagonal (bishop-like).

  Returns 1 if clear, 0 if blocked.
*/
int is_path_clear(char board[BOARD_SIZE][BOARD_SIZE], int r1, int c1, int r2, int c2)
{
    int dr = 0;
    int dc = 0;
    int r, c;

    if (r2 > r1) dr = 1;
    else if (r2 < r1) dr = -1;

    if (c2 > c1) dc = 1;
    else if (c2 < c1) dc = -1;

    r = r1 + dr;
    c = c1 + dc;

    while (!(r == r2 && c == c2)) {
        if (board[r][c] != '.') return 0;
        r += dr;
        c += dc;
    }

    return 1;
}

/*
  is_pawn_move_valid (Phase 2)
  Checks basic pawn movement:
    - Move forward 1 if empty
    - Move forward 2 from the starting row if both squares are empty
    - Capture diagonally by 1 if an enemy piece is on the target square

  Not implemented:
    - En passant
    - Promotion

  Returns 1 if valid, 0 if invalid (with an error message).
*/
int is_pawn_move_valid(char board[BOARD_SIZE][BOARD_SIZE],
                       int r1, int c1, int r2, int c2,
                       char mover_color,
                       char error[], int error_size)
{
    int dir;
    int start_row;
    int dr = r2 - r1;
    int dc = c2 - c1;
    char target = board[r2][c2];

    if (mover_color == 'w') {
        dir = -1;
        start_row = 6; /* rank 2 */
    } else {
        dir = 1;
        start_row = 1; /* rank 7 */
    }

    /* Diagonal capture */
    if (dr == dir && abs_int(dc) == 1) {
        if (target == '.') {
            snprintf(error, (size_t)error_size, "Pawn capture must capture a piece.");
            return 0;
        }
        return 1;
    }

    /* Forward move must stay in the same file */
    if (dc != 0) {
        snprintf(error, (size_t)error_size, "Pawn moves forward (straight) unless capturing.");
        return 0;
    }

    /* Forward 1 */
    if (dr == dir) {
        if (target != '.') {
            snprintf(error, (size_t)error_size, "Pawn cannot move forward into an occupied square.");
            return 0;
        }
        return 1;
    }

    /* Forward 2 from start */
    if (dr == 2 * dir && r1 == start_row) {
        int mid_row = r1 + dir;
        if (board[mid_row][c1] != '.' || target != '.') {
            snprintf(error, (size_t)error_size, "Pawn double-step is blocked.");
            return 0;
        }
        return 1;
    }

    snprintf(error, (size_t)error_size, "Illegal pawn move.");
    return 0;
}

/*
  is_rook_move_valid (Phase 2)
  Rook moves any number of squares in a straight line (same row OR same column),
  as long as the path is clear.
*/
int is_rook_move_valid(char board[BOARD_SIZE][BOARD_SIZE],
                       int r1, int c1, int r2, int c2,
                       char error[], int error_size)
{
    int dr = r2 - r1;
    int dc = c2 - c1;

    if (!(dr == 0 || dc == 0)) {
        snprintf(error, (size_t)error_size, "Rook must move in a straight line.");
        return 0;
    }

    if (!is_path_clear(board, r1, c1, r2, c2)) {
        snprintf(error, (size_t)error_size, "Rook path is blocked.");
        return 0;
    }

    return 1;
}

/*
  is_bishop_move_valid (Phase 2)
  Bishop moves any number of squares diagonally,
  as long as the path is clear.
*/
int is_bishop_move_valid(char board[BOARD_SIZE][BOARD_SIZE],
                         int r1, int c1, int r2, int c2,
                         char error[], int error_size)
{
    int dr = r2 - r1;
    int dc = c2 - c1;

    if (abs_int(dr) != abs_int(dc)) {
        snprintf(error, (size_t)error_size, "Bishop must move diagonally.");
        return 0;
    }

    if (!is_path_clear(board, r1, c1, r2, c2)) {
        snprintf(error, (size_t)error_size, "Bishop path is blocked.");
        return 0;
    }

    return 1;
}

/*
  is_knight_move_valid (Phase 2)
  Knight moves in an L-shape:
    - 2 squares in one direction and 1 square perpendicular

  Knights can jump over pieces, so we do NOT check path.
*/
int is_knight_move_valid(int r1, int c1, int r2, int c2,
                         char error[], int error_size)
{
    int dr = abs_int(r2 - r1);
    int dc = abs_int(c2 - c1);

    if (!((dr == 2 && dc == 1) || (dr == 1 && dc == 2))) {
        snprintf(error, (size_t)error_size, "Knight must move in an L-shape (2+1).");
        return 0;
    }

    return 1;
}

/*
  is_queen_move_valid (Phase 2)
  Queen moves like a rook OR a bishop.
*/
int is_queen_move_valid(char board[BOARD_SIZE][BOARD_SIZE],
                        int r1, int c1, int r2, int c2,
                        char error[], int error_size)
{
    int dr = r2 - r1;
    int dc = c2 - c1;

    int straight = (dr == 0 || dc == 0);
    int diagonal = (abs_int(dr) == abs_int(dc));

    if (!(straight || diagonal)) {
        snprintf(error, (size_t)error_size, "Queen must move straight or diagonally.");
        return 0;
    }

    if (!is_path_clear(board, r1, c1, r2, c2)) {
        snprintf(error, (size_t)error_size, "Queen path is blocked.");
        return 0;
    }

    return 1;
}

/*
  is_king_move_valid (Phase 2)
  King moves exactly 1 square in any direction.

  Not implemented:
    - Castling
    - Check rules (king can move into check in this simple version)
*/
int is_king_move_valid(int r1, int c1, int r2, int c2,
                       char error[], int error_size)
{
    int dr = abs_int(r2 - r1);
    int dc = abs_int(c2 - c1);

    if (dr > 1 || dc > 1) {
        snprintf(error, (size_t)error_size, "King moves only 1 square (no castling).");
        return 0;
    }

    return 1;
}

/*
  is_move_valid (Phase 2)
  This is the main move checker.

  It checks:
    - There is a piece on the from-square
    - The piece belongs to the player whose turn it is
    - You are not capturing your own piece
    - The piece's movement rules (pawn/rook/bishop/knight/queen/king)

  It does NOT check:
    - Check/checkmate
    - Special rules (castling, en passant, promotion)

  Returns 1 if valid, 0 if invalid (with an error message).
*/
int is_move_valid(char board[BOARD_SIZE][BOARD_SIZE],
                  int r1, int c1, int r2, int c2,
                  char current_turn,
                  char error[], int error_size)
{
    char p = board[r1][c1];
    char target = board[r2][c2];
    char mover_color = piece_color(p);
    char target_color = piece_color(target);

    if (r1 == r2 && c1 == c2) {
        snprintf(error, (size_t)error_size, "From and to squares are the same.");
        return 0;
    }

    if (p == '.') {
        snprintf(error, (size_t)error_size, "No piece on the from-square.");
        return 0;
    }

    if (mover_color != current_turn) {
        snprintf(error, (size_t)error_size, "That is not your piece.");
        return 0;
    }

    if (target != '.' && target_color == mover_color) {
        snprintf(error, (size_t)error_size, "You cannot capture your own piece.");
        return 0;
    }

    /* Piece movement rules */
    switch ((char)tolower((unsigned char)p)) {
        case 'p':
            return is_pawn_move_valid(board, r1, c1, r2, c2, mover_color, error, error_size);
        case 'r':
            return is_rook_move_valid(board, r1, c1, r2, c2, error, error_size);
        case 'b':
            return is_bishop_move_valid(board, r1, c1, r2, c2, error, error_size);
        case 'n':
            return is_knight_move_valid(r1, c1, r2, c2, error, error_size);
        case 'q':
            return is_queen_move_valid(board, r1, c1, r2, c2, error, error_size);
        case 'k':
            return is_king_move_valid(r1, c1, r2, c2, error, error_size);
        default:
            snprintf(error, (size_t)error_size, "Unknown piece type.");
            return 0;
    }
}

/*
  apply_move (Phase 2)
  Updates the board array by moving the piece from (r1,c1) to (r2,c2).
  Captures are handled automatically by overwriting the target square.
*/
void apply_move(char board[BOARD_SIZE][BOARD_SIZE], int r1, int c1, int r2, int c2)
{
    board[r2][c2] = board[r1][c1];
    board[r1][c1] = '.';
}

/*
  =========================
  PHASE 3: PAWN PROMOTION
  =========================
*/

/*
  promote_pawn (Phase 3)
  After a move is applied, this function checks if the piece on (row,col)
  is a pawn that reached the last rank:
    - White pawn 'P' reaches row 0
    - Black pawn 'p' reaches row 7

  If so, it asks the user to choose a promotion piece:
    Q (Queen), R (Rook), B (Bishop), N (Knight)

  Then it replaces the pawn with the chosen piece.

  Notes (by requirement):
    - This function does not implement check/checkmate.
    - Only pawn promotion is added in Phase 3.
*/
void promote_pawn(char board[BOARD_SIZE][BOARD_SIZE], int row, int col)
{
    char p = board[row][col];

    /* Only promote pawns that reached the last rank */
    if (p == 'P' && row == 0) {
        /* White promotion */
    } else if (p == 'p' && row == 7) {
        /* Black promotion */
    } else {
        return; /* Not a promotable pawn */
    }

    while (1) {
        char line[32];
        char choice;

        printf("Pawn promotion! Choose Q/R/B/N: ");
        if (!read_line(line, (int)sizeof(line))) return;

        if ((int)strlen(line) < 1) continue;

        choice = (char)toupper((unsigned char)line[0]);
        if (choice == 'Q' || choice == 'R' || choice == 'B' || choice == 'N') {
            if (p == 'P') {
                board[row][col] = choice; /* White uses uppercase */
            } else {
                board[row][col] = (char)tolower((unsigned char)choice); /* Black uses lowercase */
            }
            return;
        }

        printf("Invalid choice. Please type Q, R, B, or N.\n");
    }
}

/*
  =========================
  PHASE 3B: CASTLING (move tracking + castling handler)
  =========================
*/

/*
  Phase 3B variables (Castling flags)
  These flags start at 0 and become 1 once the piece has moved.
  (Phase 6/7 undo restores these flags from history.)

  We track:
    - Whether each king has moved
    - Whether each rook (left/right) has moved

  Meaning of left/right rooks:
    - "left rook"  is the rook that starts on the a-file (a1 / a8)
    - "right rook" is the rook that starts on the h-file (h1 / h8)
*/
int white_king_moved = 0;
int black_king_moved = 0;
int white_rook_left_moved = 0;
int white_rook_right_moved = 0;
int black_rook_left_moved = 0;
int black_rook_right_moved = 0;

/*
  handle_castling (Phase 3B)
  Checks for a castling attempt and performs it if valid.

  A castling attempt is:
    - The moving piece is a king
    - The king moves 2 squares horizontally on the same row

  Conditions (simplified, by requirement):
    - King has NOT moved before
    - Corresponding rook has NOT moved before
    - All squares between king and rook are empty
    - We IGNORE check rules (king may castle through check)

  Return values:
    - 1: castling was performed (king and rook moved)
    - 0: not a castling attempt (caller should handle as a normal move)
    - -1: castling was attempted but is invalid (error message is filled)
*/
int handle_castling(char board[BOARD_SIZE][BOARD_SIZE],
                    int r1, int c1, int r2, int c2,
                    char current_turn,
                    char error[], int error_size)
{
    char p = board[r1][c1];
    int dr = r2 - r1;
    int dc = c2 - c1;

    /* Must be a king moving two squares horizontally */
    if ((char)tolower((unsigned char)p) != 'k') return 0;
    if (dr != 0) return 0;
    if (abs_int(dc) != 2) return 0;

    /* Must be the current player's king */
    if (piece_color(p) != current_turn) {
        snprintf(error, (size_t)error_size, "That is not your king.");
        return -1;
    }

    /* We only allow castling from the normal starting squares (simple) */
    if (current_turn == 'w') {
        if (!(r1 == 7 && c1 == 4)) {
            snprintf(error, (size_t)error_size, "White must castle from e1.");
            return -1;
        }
        if (white_king_moved) {
            snprintf(error, (size_t)error_size, "White king already moved; cannot castle.");
            return -1;
        }

        /* Kingside: e1 -> g1 (c2 = 6), rook h1 -> f1 */
        if (c2 == 6) {
            if (white_rook_right_moved) {
                snprintf(error, (size_t)error_size, "Right rook already moved; cannot castle kingside.");
                return -1;
            }
            if (board[7][7] != 'R') {
                snprintf(error, (size_t)error_size, "No rook on h1 for kingside castling.");
                return -1;
            }
            if (board[7][5] != '.' || board[7][6] != '.') {
                snprintf(error, (size_t)error_size, "Squares between king and rook must be empty.");
                return -1;
            }

            /* Perform castling */
            board[7][6] = 'K';
            board[7][4] = '.';
            board[7][5] = 'R';
            board[7][7] = '.';

            white_king_moved = 1;
            white_rook_right_moved = 1;
            return 1;
        }

        /* Queenside: e1 -> c1 (c2 = 2), rook a1 -> d1 */
        if (c2 == 2) {
            if (white_rook_left_moved) {
                snprintf(error, (size_t)error_size, "Left rook already moved; cannot castle queenside.");
                return -1;
            }
            if (board[7][0] != 'R') {
                snprintf(error, (size_t)error_size, "No rook on a1 for queenside castling.");
                return -1;
            }
            if (board[7][1] != '.' || board[7][2] != '.' || board[7][3] != '.') {
                snprintf(error, (size_t)error_size, "Squares between king and rook must be empty.");
                return -1;
            }

            /* Perform castling */
            board[7][2] = 'K';
            board[7][4] = '.';
            board[7][3] = 'R';
            board[7][0] = '.';

            white_king_moved = 1;
            white_rook_left_moved = 1;
            return 1;
        }

        snprintf(error, (size_t)error_size, "Invalid castling destination.");
        return -1;
    }

    /* Black */
    if (!(r1 == 0 && c1 == 4)) {
        snprintf(error, (size_t)error_size, "Black must castle from e8.");
        return -1;
    }
    if (black_king_moved) {
        snprintf(error, (size_t)error_size, "Black king already moved; cannot castle.");
        return -1;
    }

    /* Kingside: e8 -> g8, rook h8 -> f8 */
    if (c2 == 6) {
        if (black_rook_right_moved) {
            snprintf(error, (size_t)error_size, "Right rook already moved; cannot castle kingside.");
            return -1;
        }
        if (board[0][7] != 'r') {
            snprintf(error, (size_t)error_size, "No rook on h8 for kingside castling.");
            return -1;
        }
        if (board[0][5] != '.' || board[0][6] != '.') {
            snprintf(error, (size_t)error_size, "Squares between king and rook must be empty.");
            return -1;
        }

        /* Perform castling */
        board[0][6] = 'k';
        board[0][4] = '.';
        board[0][5] = 'r';
        board[0][7] = '.';

        black_king_moved = 1;
        black_rook_right_moved = 1;
        return 1;
    }

    /* Queenside: e8 -> c8, rook a8 -> d8 */
    if (c2 == 2) {
        if (black_rook_left_moved) {
            snprintf(error, (size_t)error_size, "Left rook already moved; cannot castle queenside.");
            return -1;
        }
        if (board[0][0] != 'r') {
            snprintf(error, (size_t)error_size, "No rook on a8 for queenside castling.");
            return -1;
        }
        if (board[0][1] != '.' || board[0][2] != '.' || board[0][3] != '.') {
            snprintf(error, (size_t)error_size, "Squares between king and rook must be empty.");
            return -1;
        }

        /* Perform castling */
        board[0][2] = 'k';
        board[0][4] = '.';
        board[0][3] = 'r';
        board[0][0] = '.';

        black_king_moved = 1;
        black_rook_left_moved = 1;
        return 1;
    }

    snprintf(error, (size_t)error_size, "Invalid castling destination.");
    return -1;
}

/*
  =========================
  PHASE 4: CHECK + CHECKMATE
  =========================
*/

/*
  is_king_in_check (Phase 4)
  Finds the king of current_turn and checks if any opponent piece can attack it.

  This uses the existing is_move_valid() movement rules to test attacks.

  Returns 1 if the king is in check, else 0.
*/
int is_king_in_check(char board[BOARD_SIZE][BOARD_SIZE], char current_turn)
{
    int r, c;
    int king_row = -1;
    int king_col = -1;
    char king_piece = (current_turn == 'w') ? 'K' : 'k';
    char opponent = (current_turn == 'w') ? 'b' : 'w';

    /* Find the king */
    for (r = 0; r < BOARD_SIZE; r++) {
        for (c = 0; c < BOARD_SIZE; c++) {
            if (board[r][c] == king_piece) {
                king_row = r;
                king_col = c;
                break;
            }
        }
        if (king_row != -1) break;
    }

    if (king_row == -1) return 0; /* Safety fallback */

    /* Check if any opponent piece can move to the king's square */
    for (r = 0; r < BOARD_SIZE; r++) {
        for (c = 0; c < BOARD_SIZE; c++) {
            if (piece_color(board[r][c]) == opponent) {
                char error[128];
                error[0] = '\0';
                if (is_move_valid(board, r, c, king_row, king_col, opponent, error, (int)sizeof(error))) {
                    return 1;
                }
            }
        }
    }

    return 0;
}

/*
  would_cause_check (Phase 4)
  Copies the board, simulates the move, then checks if the current player's king
  would be in check.

  This is the key "move safety" rule: you may not make a move that leaves your own
  king in check.

  Returns 1 if the move is illegal (king would be in check), else 0.
*/
int would_cause_check(char board[BOARD_SIZE][BOARD_SIZE],
                      int r1, int c1, int r2, int c2,
                      char current_turn)
{
    char temp[BOARD_SIZE][BOARD_SIZE];
    int r, c;

    /* Copy board */
    for (r = 0; r < BOARD_SIZE; r++) {
        for (c = 0; c < BOARD_SIZE; c++) {
            temp[r][c] = board[r][c];
        }
    }

    /* Simulate the move */
    apply_move(temp, r1, c1, r2, c2);

    return is_king_in_check(temp, current_turn);
}

/*
  is_checkmate (Phase 4C)
  Returns 1 if the current player is in check and has no legal move to escape.
  Otherwise returns 0.

  This works by brute force: try every move and see if any legal move avoids check.
*/
int is_checkmate(char board[BOARD_SIZE][BOARD_SIZE], char current_turn)
{
    int r1, c1, r2, c2;

    if (!is_king_in_check(board, current_turn)) {
        return 0;
    }

    /* Try every possible move for current player */
    for (r1 = 0; r1 < BOARD_SIZE; r1++) {
        for (c1 = 0; c1 < BOARD_SIZE; c1++) {
            if (piece_color(board[r1][c1]) != current_turn) continue;

            for (r2 = 0; r2 < BOARD_SIZE; r2++) {
                for (c2 = 0; c2 < BOARD_SIZE; c2++) {
                    char error[128];
                    error[0] = '\0';

                    if (is_move_valid(board, r1, c1, r2, c2, current_turn, error, (int)sizeof(error))) {
                        if (!would_cause_check(board, r1, c1, r2, c2, current_turn)) {
                            return 0; /* Found a legal escape move */
                        }
                    }
                }
            }
        }
    }

    return 1;
}

/*
  is_stalemate (Phase 4C)
  Returns 1 if the current player is NOT in check and has no legal moves.
  Otherwise returns 0.
*/
int is_stalemate(char board[BOARD_SIZE][BOARD_SIZE], char current_turn)
{
    int r1, c1, r2, c2;

    if (is_king_in_check(board, current_turn)) {
        return 0;
    }

    /* Try every possible move for current player */
    for (r1 = 0; r1 < BOARD_SIZE; r1++) {
        for (c1 = 0; c1 < BOARD_SIZE; c1++) {
            if (piece_color(board[r1][c1]) != current_turn) continue;

            for (r2 = 0; r2 < BOARD_SIZE; r2++) {
                for (c2 = 0; c2 < BOARD_SIZE; c2++) {
                    char error[128];
                    error[0] = '\0';

                    if (is_move_valid(board, r1, c1, r2, c2, current_turn, error, (int)sizeof(error))) {
                        if (!would_cause_check(board, r1, c1, r2, c2, current_turn)) {
                            return 0; /* Found a legal move */
                        }
                    }
                }
            }
        }
    }

    return 1;
}

/*
  print_turn_status (Final)
  After the turn switches, report Check / Checkmate / Stalemate for the side to move.

  Returns 1 if the game should end (checkmate or stalemate), else 0.
*/
int print_turn_status(char board[BOARD_SIZE][BOARD_SIZE], char turn)
{
    if (is_king_in_check(board, turn)) {
        printf("Check!\n");
    }

    if (is_checkmate(board, turn)) {
        printf("Checkmate!\n");
        return 1;
    }

    if (is_stalemate(board, turn)) {
        printf("Stalemate!\n");
        return 1;
    }

    return 0;
}

/*
  =========================
  PHASE ADVANCED: AI + IMPROVED HELP
  =========================
*/

/*
  evaluate_board (Advanced)
  Adds up simple material values:
    Pawn=1, Knight/Bishop=3, Rook=5, Queen=9
  White pieces add to the score, black pieces subtract.

  The king is not counted (0).
*/
int evaluate_board(char board[BOARD_SIZE][BOARD_SIZE])
{
    int r, c;
    int score = 0;

    for (r = 0; r < BOARD_SIZE; r++) {
        for (c = 0; c < BOARD_SIZE; c++) {
            char p = board[r][c];
            int value = 0;

            switch ((char)tolower((unsigned char)p)) {
                case 'p': value = 1; break;
                case 'n': value = 3; break;
                case 'b': value = 3; break;
                case 'r': value = 5; break;
                case 'q': value = 9; break;
                default: value = 0; break;
            }

            if (value != 0) {
                if (isupper((unsigned char)p)) score += value;
                else score -= value;
            }
        }
    }

    return score;
}

/*
  best_move (Advanced)
  Finds the best legal move for current_turn by trying every move and scoring
  the resulting board using evaluate_board().

  - Uses is_move_valid() + would_cause_check() to filter legal moves.
  - Uses a copy board for safe simulation (does not change the real board).

  For selection:
    - White maximizes score
    - Black minimizes score

  If print_result is 1, prints: Best move: e2 e4

  Returns 1 if a move was found, else 0.
*/
int best_move(char board[BOARD_SIZE][BOARD_SIZE], char current_turn,
              int *out_r1, int *out_c1, int *out_r2, int *out_c2,
              int print_result)
{
    int r1, c1, r2, c2;
    int found = 0;
    int best_score = (current_turn == 'w') ? -999999 : 999999;

    for (r1 = 0; r1 < BOARD_SIZE; r1++) {
        for (c1 = 0; c1 < BOARD_SIZE; c1++) {
            if (piece_color(board[r1][c1]) != current_turn) continue;

            for (r2 = 0; r2 < BOARD_SIZE; r2++) {
                for (c2 = 0; c2 < BOARD_SIZE; c2++) {
                    char error[128];
                    char temp[BOARD_SIZE][BOARD_SIZE];
                    int score;

                    error[0] = '\0';

                    if (!is_move_valid(board, r1, c1, r2, c2, current_turn, error, (int)sizeof(error))) {
                        continue;
                    }

                    if (would_cause_check(board, r1, c1, r2, c2, current_turn)) {
                        continue;
                    }

                    /* Safe simulation */
                    copy_board(temp, board);
                    apply_move(temp, r1, c1, r2, c2);
                    score = evaluate_board(temp);

                    if (!found) {
                        found = 1;
                        best_score = score;
                        if (out_r1) *out_r1 = r1;
                        if (out_c1) *out_c1 = c1;
                        if (out_r2) *out_r2 = r2;
                        if (out_c2) *out_c2 = c2;
                    } else {
                        if (current_turn == 'w') {
                            if (score > best_score) {
                                best_score = score;
                                if (out_r1) *out_r1 = r1;
                                if (out_c1) *out_c1 = c1;
                                if (out_r2) *out_r2 = r2;
                                if (out_c2) *out_c2 = c2;
                            }
                        } else {
                            if (score < best_score) {
                                best_score = score;
                                if (out_r1) *out_r1 = r1;
                                if (out_c1) *out_c1 = c1;
                                if (out_r2) *out_r2 = r2;
                                if (out_c2) *out_c2 = c2;
                            }
                        }
                    }
                }
            }
        }
    }

    if (found && print_result) {
        char from_file = (char)('a' + *out_c1);
        char from_rank = (char)('8' - *out_r1);
        char to_file = (char)('a' + *out_c2);
        char to_rank = (char)('8' - *out_r2);
        printf("Best move: %c%c %c%c\n", from_file, from_rank, to_file, to_rank);
    }

    if (!found && print_result) {
        printf("No legal moves available.\n");
    }

    return found;
}

/*
  =========================
  PHASE 5: HELP (SUGGEST MOVE)
  =========================
*/

/*
  suggest_top_3_moves (Phase 5)
  Suggests the top 3 legal moves for the current player based on material score.
*/
typedef struct {
    int r1, c1, r2, c2;
    int score;
} MoveScore;

int compare_moves(const void *a, const void *b) {
    MoveScore *ma = (MoveScore *)a;
    MoveScore *mb = (MoveScore *)b;
    return mb->score - ma->score;
}

void suggest_top_3_moves(char board[BOARD_SIZE][BOARD_SIZE], char current_turn)
{
    int r1, c1, r2, c2;
    MoveScore moves[1024];
    int move_count = 0;

    for (r1 = 0; r1 < BOARD_SIZE; r1++) {
        for (c1 = 0; c1 < BOARD_SIZE; c1++) {
            if (piece_color(board[r1][c1]) != current_turn) continue;

            for (r2 = 0; r2 < BOARD_SIZE; r2++) {
                for (c2 = 0; c2 < BOARD_SIZE; c2++) {
                    char error[128];
                    char temp[BOARD_SIZE][BOARD_SIZE];
                    
                    error[0] = '\0';
                    if (!is_move_valid(board, r1, c1, r2, c2, current_turn, error, sizeof(error))) {
                        continue;
                    }
                    if (would_cause_check(board, r1, c1, r2, c2, current_turn)) {
                        continue;
                    }

                    copy_board(temp, board);
                    apply_move(temp, r1, c1, r2, c2);
                    
                    int score = evaluate_board(temp);
                    if (current_turn == 'b') score = -score;

                    moves[move_count].r1 = r1;
                    moves[move_count].c1 = c1;
                    moves[move_count].r2 = r2;
                    moves[move_count].c2 = c2;
                    moves[move_count].score = score;
                    move_count++;
                }
            }
        }
    }

    if (move_count == 0) {
        printf("No legal moves available.\n");
        return;
    }

    qsort(moves, move_count, sizeof(MoveScore), compare_moves);

    printf("\nBest Moves:\n");
    int limit = (move_count < 3) ? move_count : 3;
    for (int i = 0; i < limit; i++) {
        char f1 = 'a' + moves[i].c1;
        char rk1 = '8' - moves[i].r1;
        char f2 = 'a' + moves[i].c2;
        char rk2 = '8' - moves[i].r2;
        printf("%d. %c%c -> %c%c\n", i + 1, f1, rk1, f2, rk2);
    }
    printf("\n");
}

/*
  =========================
  PHASE 6: UNDO + HISTORY
  =========================
*/

/*
  =========================
  PHASE 7: MOVE HISTORY DISPLAY
  =========================
*/

/*
  copy_board (Phase 6)
  Copies the board from src into dest.
*/
void copy_board(char dest[BOARD_SIZE][BOARD_SIZE], char src[BOARD_SIZE][BOARD_SIZE])
{
    int r, c;
    for (r = 0; r < BOARD_SIZE; r++) {
        for (c = 0; c < BOARD_SIZE; c++) {
            dest[r][c] = src[r][c];
        }
    }
}

/*
  main (Final)
  Program flow:
    INPUT -> VALIDATION -> APPLY -> STATE UPDATE -> OUTPUT
*/

/*
  Detailed phases (kept for learning):
    - Phase 1: setup + display
    - Phase 2: move input + validation
    - Phase 3: promotion + castling
    - Phase 4: check + checkmate + stalemate
    - Phase 5: help (suggest move)
    - Phase 6: undo + board history
    - Phase 7: move history display

  Rules:
    - If the move is invalid, print an error and ask again.
    - Do NOT change turn on invalid moves.
*/
int main(void)
{
    char board[BOARD_SIZE][BOARD_SIZE];
    char perspective;
    char setup_mode;

    char line[128];
    char from[8];
    char to[8];

    char turn = 'w';

    /* History system (Phase 6/7): store previous states so undo is complete. */
    char history[MAX_HISTORY][BOARD_SIZE][BOARD_SIZE];
    int history_count = 0;

    /* Castling flags are part of the game state, so we also store them in history. */
    int history_white_king_moved[MAX_HISTORY];
    int history_black_king_moved[MAX_HISTORY];
    int history_white_rook_left_moved[MAX_HISTORY];
    int history_white_rook_right_moved[MAX_HISTORY];
    int history_black_rook_left_moved[MAX_HISTORY];
    int history_black_rook_right_moved[MAX_HISTORY];

    char move_history[MAX_MOVES][16];
    int move_count = 0;

    /* Simple AI settings (Advanced) */
    int ai_enabled = 0;
    char ai_side = 'b';

    printf("Console Chess (C99)\n");
    printf("Example Moves: e2 e4\n");
    printf("Type 'quit' to exit.\n\n");

    perspective = choose_side();
    setup_mode = choose_setup_mode();

    /* --- NEW CODE BEGINS HERE --- */
    char play_computer = choose_opponent();
    if (play_computer == 'y') {
        ai_enabled = 1;
        /* The computer will play the opposite side of your viewing perspective */
        ai_side = (perspective == 'w') ? 'b' : 'w';
        printf("\n=> You are playing against the computer (%s).\n", (ai_side == 'w') ? "White" : "Black");
    }
    /* --- NEW CODE ENDS HERE --- */

    if (setup_mode == 's') {
        standard_setup(board);
    } else {
        if (!load_custom_setup(board)) {
            printf("Input ended. Exiting.\n");
            return 0;
        }
    }

    print_board(board, perspective);

    while (1) {
        int r1, c1, r2, c2;
        int parsed;
        char error[128];

        /* INPUT */
        if (ai_enabled && turn == ai_side) {
            printf("\nComputer is thinking...\n");
            int ar1, ac1, ar2, ac2;
            if (!best_move(board, turn, &ar1, &ac1, &ar2, &ac2, 1)) {
                /* No move available (should normally be handled by checkmate/stalemate). */
                break;
            }

            {
                char from_file = (char)('a' + ac1);
                char from_rank = (char)('8' - ar1);
                char to_file = (char)('a' + ac2);
                char to_rank = (char)('8' - ar2);
                snprintf(line, (size_t)sizeof(line), "%c%c %c%c", from_file, from_rank, to_file, to_rank);
            }
        } else {
            printf("%s to move. Enter move (e2 e4): ", (turn == 'w') ? "White" : "Black");
            if (!read_line(line, (int)sizeof(line))) {
                printf("Input ended. Exiting.\n");
                break;
            }
        }

        /* COMMANDS (do not consume a turn) */
        if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) {
            printf("Goodbye.\n");
            break;
        }

        if (strcmp(line, "help") == 0 || strcmp(line, "h") == 0 || strcmp(line, "H") == 0) {
            suggest_top_3_moves(board, turn);
            continue;
        }

        if (strcmp(line, "newgame") == 0) {
            standard_setup(board);
            turn = 'w';
            history_count = 0;
            move_count = 0;
            white_king_moved = 0;
            black_king_moved = 0;
            white_rook_left_moved = 0;
            white_rook_right_moved = 0;
            black_rook_left_moved = 0;
            black_rook_right_moved = 0;
            print_board(board, perspective);
            printf("Started a new game!\n");
            continue;
        }

        /* AI commands */
        {
            char cmd[8], arg1[8], arg2[8];
            int n = sscanf(line, "%7s %7s %7s", cmd, arg1, arg2);

            if (n == 2 && strcmp(cmd, "ai") == 0 && strcmp(arg1, "off") == 0) {
                ai_enabled = 0;
                continue;
            }

            if (n == 3 && strcmp(cmd, "ai") == 0 && strcmp(arg1, "on") == 0 && (arg2[0] == 'w' || arg2[0] == 'b') && arg2[1] == '\0') {
                ai_enabled = 1;
                ai_side = arg2[0];
                continue;
            }

            if (n == 1 && strcmp(cmd, "ai") == 0) {
                best_move(board, turn, &r1, &c1, &r2, &c2, 1);
                continue;
            }
        }

        if (strcmp(line, "undo") == 0) {
            if (history_count > 0) {
                history_count--;
                if (move_count > 0) move_count--;

                /* Restore full state (board + castling flags) */
                copy_board(board, history[history_count]);
                white_king_moved = history_white_king_moved[history_count];
                black_king_moved = history_black_king_moved[history_count];
                white_rook_left_moved = history_white_rook_left_moved[history_count];
                white_rook_right_moved = history_white_rook_right_moved[history_count];
                black_rook_left_moved = history_black_rook_left_moved[history_count];
                black_rook_right_moved = history_black_rook_right_moved[history_count];

                turn = (turn == 'w') ? 'b' : 'w';
                print_board(board, perspective);
            } else {
                printf("No moves to undo.\n");
            }
            continue;
        }

        if (strcmp(line, "history") == 0) {
            if (move_count == 0) {
                printf("No moves yet.\n");
            } else {
                int i;
                for (i = 0; i < move_count; i++) {
                    printf("%d. %s\n", i + 1, move_history[i]);
                }
            }
            continue;
        }

        /* VALIDATION: parse a move like "e2 e4" */
        from[0] = '\0';
        to[0] = '\0';
        parsed = sscanf(line, "%7s %7s", from, to);
        if (parsed != 2) {
            printf("Invalid input. Use: e2 e4\n");
            continue;
        }

        if (!parse_square(from, &r1, &c1) || !parse_square(to, &r2, &c2)) {
            printf("Invalid square. Use a1..h8 (example: e2).\n");
            continue;
        }

        error[0] = '\0';

        /* Flow (Phase 3B): try castling first; if not castling, validate as a normal move. */
        /* Phase 3B: try castling first (king moves 2 squares horizontally) */
        {
            char board_before_castle[BOARD_SIZE][BOARD_SIZE];
            int wk_before = white_king_moved;
            int bk_before = black_king_moved;
            int wrl_before = white_rook_left_moved;
            int wrr_before = white_rook_right_moved;
            int brl_before = black_rook_left_moved;
            int brr_before = black_rook_right_moved;

            copy_board(board_before_castle, board);

            int castle_result = handle_castling(board, r1, c1, r2, c2, turn, error, (int)sizeof(error));
            if (castle_result == 1) {
                if (history_count < MAX_HISTORY) {
                    copy_board(history[history_count], board_before_castle);
                    history_white_king_moved[history_count] = wk_before;
                    history_black_king_moved[history_count] = bk_before;
                    history_white_rook_left_moved[history_count] = wrl_before;
                    history_white_rook_right_moved[history_count] = wrr_before;
                    history_black_rook_left_moved[history_count] = brl_before;
                    history_black_rook_right_moved[history_count] = brr_before;
                    history_count++;
                }

                if (move_count < MAX_MOVES) {
                    char from_file = (char)('a' + c1);
                    char from_rank = (char)('8' - r1);
                    char to_file = (char)('a' + c2);
                    char to_rank = (char)('8' - r2);
                    snprintf(move_history[move_count], (size_t)sizeof(move_history[move_count]), "%c%c %c%c", from_file, from_rank, to_file, to_rank);
                    move_count++;
                }

                /* Castling performed: change turn and show board */
                turn = (turn == 'w') ? 'b' : 'w';

                if (print_turn_status(board, turn)) {
                    break;
                }

                print_board(board, perspective);
                continue;
            }
            if (castle_result == -1) {
                printf("Illegal move: %s\n", error);
                continue; /* IMPORTANT: do not change turn */
            }
        }

        /* Normal move validation (Phase 2 rules) */
        if (!is_move_valid(board, r1, c1, r2, c2, turn, error, (int)sizeof(error))) {
            printf("Illegal move: %s\n", error);
            continue; /* IMPORTANT: do not change turn */
        }

        if (would_cause_check(board, r1, c1, r2, c2, turn)) {
            printf("Illegal move: King would be in check.\n");
            continue;
        }

        /* APPLY: save history, apply move, promotion, then update castling flags */
        {
            char moved_piece = board[r1][c1];
            char captured_piece = board[r2][c2];  /* used for rook-capture tracking */

            /* Save board + castling flags for undo (state BEFORE the move) */
            if (history_count < MAX_HISTORY) {
                copy_board(history[history_count], board);
                history_white_king_moved[history_count] = white_king_moved;
                history_black_king_moved[history_count] = black_king_moved;
                history_white_rook_left_moved[history_count] = white_rook_left_moved;
                history_white_rook_right_moved[history_count] = white_rook_right_moved;
                history_black_rook_left_moved[history_count] = black_rook_left_moved;
                history_black_rook_right_moved[history_count] = black_rook_right_moved;
                history_count++;
            }

            /* Save move history string (Phase 7) */
            if (move_count < MAX_MOVES) {
                char from_file = (char)('a' + c1);
                char from_rank = (char)('8' - r1);
                char to_file = (char)('a' + c2);
                char to_rank = (char)('8' - r2);
                snprintf(move_history[move_count], (size_t)sizeof(move_history[move_count]), "%c%c %c%c", from_file, from_rank, to_file, to_rank);
                move_count++;
            }

            apply_move(board, r1, c1, r2, c2);
            promote_pawn(board, r2, c2);

            /* Update castling flags based on moved pieces and captured rooks */
            if (captured_piece == 'R' && r2 == 7 && c2 == 0) white_rook_left_moved = 1;
            if (captured_piece == 'R' && r2 == 7 && c2 == 7) white_rook_right_moved = 1;
            if (captured_piece == 'r' && r2 == 0 && c2 == 0) black_rook_left_moved = 1;
            if (captured_piece == 'r' && r2 == 0 && c2 == 7) black_rook_right_moved = 1;

            if (moved_piece == 'K') white_king_moved = 1;
            if (moved_piece == 'k') black_king_moved = 1;

            if (moved_piece == 'R' && r1 == 7 && c1 == 0) white_rook_left_moved = 1;
            if (moved_piece == 'R' && r1 == 7 && c1 == 7) white_rook_right_moved = 1;
            if (moved_piece == 'r' && r1 == 0 && c1 == 0) black_rook_left_moved = 1;
            if (moved_piece == 'r' && r1 == 0 && c1 == 7) black_rook_right_moved = 1;
        }

        /* STATE UPDATE -> OUTPUT */
        turn = (turn == 'w') ? 'b' : 'w';

        if (print_turn_status(board, turn)) {
            break;
        }

        print_board(board, perspective);
    }

    return 0;
}