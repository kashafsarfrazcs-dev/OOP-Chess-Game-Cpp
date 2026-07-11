#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

//  COLOR UTILITIES  (Windows + ANSI fallback)

void setColor(int fg, int bg = 0) {
#ifdef _WIN32
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(h, (WORD)(bg * 16 + fg));
#else
	int ansi_fg[] = { 30,34,32,36,31,35,33,37 };
	int ansi_bg[] = { 40,44,42,46,41,45,43,47 };
	int f = (fg < 8) ? ansi_fg[fg] : 37;
	int b = (bg < 8) ? ansi_bg[bg] : 40;
	cout << "\033[" << f << ";" << b << "m";
#endif
}

void resetColor() {
#ifdef _WIN32
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(h, 7);
#else
	cout << "\033[0m";
#endif
}

// ============================================================
//  BASE CLASS: Piece   (Encapsulation + Polymorphism)
// ============================================================
class Piece {
protected:
	char  color;   // 'W' = White, 'B' = Black
	char  symbol;  // piece letter

public:
	Piece(char c, char s) : color(c), symbol(s) {}
	virtual ~Piece() {}

	char getColor()  const { return color; }
	char getSymbol() const { return symbol; }

	// Pure virtual - each piece overrides with its own rules
	virtual bool isValidMove(int fromRow, int fromCol,
		int toRow, int toCol,
		Piece* board[8][8]) const = 0;

	virtual char getDisplay() const { return symbol; }
};

// ============================================================
//  DERIVED CLASSES  (Inheritance)
// ============================================================

class Rook : public Piece {
public:
	Rook(char c) : Piece(c, 'R') {}

	bool isValidMove(int fr, int fc, int tr, int tc,
		Piece* b[8][8]) const override {
		if (fr != tr && fc != tc) return false;
		int dr = (tr > fr) ? 1 : (tr < fr) ? -1 : 0;
		int dc = (tc > fc) ? 1 : (tc < fc) ? -1 : 0;
		int r = fr + dr, c = fc + dc;
		while (r != tr || c != tc) {
			if (b[r][c]) return false;
			r += dr; c += dc;
		}
		return (!b[tr][tc] || b[tr][tc]->getColor() != color);
	}
};

class Knight : public Piece {
public:
	Knight(char c) : Piece(c, 'N') {}

	bool isValidMove(int fr, int fc, int tr, int tc,
		Piece* b[8][8]) const override {
		int dr = abs(tr - fr), dc = abs(tc - fc);
		if (!((dr == 2 && dc == 1) || (dr == 1 && dc == 2)))
			return false;
		return (!b[tr][tc] || b[tr][tc]->getColor() != color);
	}
};

class Bishop : public Piece {
public:
	Bishop(char c) : Piece(c, 'B') {}

	bool isValidMove(int fr, int fc, int tr, int tc,
		Piece* b[8][8]) const override {
		if (abs(tr - fr) != abs(tc - fc)) return false;
		int dr = (tr > fr) ? 1 : -1;
		int dc = (tc > fc) ? 1 : -1;
		int r = fr + dr, c = fc + dc;
		while (r != tr || c != tc) {
			if (b[r][c]) return false;
			r += dr; c += dc;
		}
		return (!b[tr][tc] || b[tr][tc]->getColor() != color);
	}
};

class Queen : public Piece {
public:
	Queen(char c) : Piece(c, 'Q') {}

	bool isValidMove(int fr, int fc, int tr, int tc,
		Piece* b[8][8]) const override {
		bool straight = (fr == tr || fc == tc);
		bool diagonal = (abs(tr - fr) == abs(tc - fc));
		if (!straight && !diagonal) return false;
		int dr = (tr > fr) ? 1 : (tr < fr) ? -1 : 0;
		int dc = (tc > fc) ? 1 : (tc < fc) ? -1 : 0;
		int r = fr + dr, c = fc + dc;
		while (r != tr || c != tc) {
			if (b[r][c]) return false;
			r += dr; c += dc;
		}
		return (!b[tr][tc] || b[tr][tc]->getColor() != color);
	}
};

class King : public Piece {
public:
	King(char c) : Piece(c, 'K') {}

	bool isValidMove(int fr, int fc, int tr, int tc,
		Piece* b[8][8]) const override {
		if (abs(tr - fr) > 1 || abs(tc - fc) > 1) return false;
		return (!b[tr][tc] || b[tr][tc]->getColor() != color);
	}
};

class Pawn : public Piece {
public:
	Pawn(char c) : Piece(c, 'P') {}

	bool isValidMove(int fr, int fc, int tr, int tc,
		Piece* b[8][8]) const override {
		int dir = (color == 'W') ? -1 : 1;
		int startRow = (color == 'W') ? 6 : 1;

		if (tc == fc && tr == fr + dir && !b[tr][tc])
			return true;
		if (tc == fc && fr == startRow && tr == fr + 2 * dir
			&& !b[fr + dir][fc] && !b[tr][tc])
			return true;
		if (abs(tc - fc) == 1 && tr == fr + dir
			&& b[tr][tc] && b[tr][tc]->getColor() != color)
			return true;
		return false;
	}
};

// ============================================================
//  BOARD CLASS  (Composition)
// ============================================================
class Board {
private:
	Piece* grid[8][8];

	void clearBoard() {
		for (int r = 0; r < 8; r++)
			for (int c = 0; c < 8; c++)
				grid[r][c] = nullptr;
	}

	void placePieces(char color, int row, int pawnRow) {
		grid[row][0] = new Rook(color);
		grid[row][1] = new Knight(color);
		grid[row][2] = new Bishop(color);
		grid[row][3] = new Queen(color);
		grid[row][4] = new King(color);
		grid[row][5] = new Bishop(color);
		grid[row][6] = new Knight(color);
		grid[row][7] = new Rook(color);
		for (int c = 0; c < 8; c++)
			grid[pawnRow][c] = new Pawn(color);
	}

public:
	Board() {
		clearBoard();
		placePieces('B', 0, 1);
		placePieces('W', 7, 6);
	}

	~Board() {
		for (int r = 0; r < 8; r++)
			for (int c = 0; c < 8; c++)
				delete grid[r][c];
	}

	Piece* getPiece(int r, int c) const { return grid[r][c]; }

	void display(const string& whitePlayer, const string& blackPlayer,
		int whiteCaptured, int blackCaptured, int moveCount) const {

#ifdef _WIN32
		system("cls");
#else
		system("clear");
#endif

		setColor(6, 0);
		cout << "\n";
		cout << "  +==========================================+\n";
		cout << "  |     CHESS GAME  -  NUCES Faisalabad     |\n";
		cout << "  +==========================================+\n";
		resetColor();

		setColor(3, 0);
		cout << "  Player WHITE : " << whitePlayer
			<< "   [Captures: " << whiteCaptured << "]\n";
		cout << "  Player BLACK : " << blackPlayer
			<< "   [Captures: " << blackCaptured << "]\n";
		setColor(6, 0);
		cout << "  Move Number  : " << moveCount << "\n\n";
		resetColor();

		setColor(6, 0);
		cout << "       a    b    c    d    e    f    g    h\n";
		cout << "     +----+----+----+----+----+----+----+----+\n";
		resetColor();

		for (int r = 0; r < 8; r++) {
			setColor(6, 0);
			cout << "  " << (8 - r) << "  |";
			resetColor();

			for (int c = 0; c < 8; c++) {
				bool isLight = (r + c) % 2 == 0;

				// Light square = white bg(7), Dark square = blue bg(1/4)
				int bg = isLight ? 7 : 1;

				Piece* p = grid[r][c];
				if (!p) {
					setColor(0, bg);
					cout << "    ";
				}
				else {
					int fg = (p->getColor() == 'W') ? 0 : 4;
					if (!isLight && p->getColor() == 'W') fg = 15;
					setColor(fg, bg);
					cout << " ";
					// Color prefix
					if (p->getColor() == 'W') cout << "w";
					else                      cout << "b";
					cout << p->getDisplay() << " ";
				}
				resetColor();
				setColor(6, 0);
				cout << "|";
				resetColor();
			}
			setColor(6, 0);
			cout << " " << (8 - r) << "\n";
			cout << "     +----+----+----+----+----+----+----+----+\n";
			resetColor();
		}
		setColor(6, 0);
		cout << "       a    b    c    d    e    f    g    h\n\n";
		resetColor();

		// Legend
		setColor(3, 0);
		cout << "  PIECES: P=Pawn  R=Rook  N=Knight  B=Bishop  Q=Queen  K=King\n";
		cout << "  PREFIX: w=White piece   b=Black piece\n\n";
		resetColor();
	}

	bool parseInput(const string& input,
		int& fr, int& fc, int& tr, int& tc) const {
		if (input.size() < 5) return false;
		fc = tolower(input[0]) - 'a';
		fr = 8 - (input[1] - '0');
		tc = tolower(input[3]) - 'a';
		tr = 8 - (input[4] - '0');
		return (fc >= 0 && fc < 8 && fr >= 0 && fr < 8 &&
			tc >= 0 && tc < 8 && tr >= 0 && tr < 8);
	}

	// Returns: 0=invalid move, ' '=moved (no capture), letter=captured piece
	char makeMove(int fr, int fc, int tr, int tc) {
		Piece* src = grid[fr][fc];
		if (!src) return 0;
		if (!src->isValidMove(fr, fc, tr, tc, grid)) return 0;

		Piece* dst = grid[tr][tc];
		char captured = 0;
		if (dst) {
			captured = dst->getSymbol();
			delete dst;
		}
		grid[tr][tc] = src;
		grid[fr][fc] = nullptr;
		return captured ? captured : ' ';
	}

	bool isKingAlive(char color) const {
		for (int r = 0; r < 8; r++)
			for (int c = 0; c < 8; c++)
				if (grid[r][c] &&
					grid[r][c]->getColor() == color &&
					grid[r][c]->getSymbol() == 'K')
					return true;
		return false;
	}
};

// ============================================================
//  GAME CLASS
// ============================================================
class Game {
private:
	Board  board;
	string player1, player2;
	char   currentTurn;
	int    moveCount;
	int    whiteCaptured, blackCaptured;
	bool   gameOver;

	void printMsg(const string& msg, int col) {
		setColor(col, 0);
		cout << "  >> " << msg << "\n";
		resetColor();
	}

public:
	Game() : currentTurn('W'), moveCount(1),
		whiteCaptured(0), blackCaptured(0), gameOver(false) {}

	void start() {
		setColor(6, 0);
		cout << "\n  +==========================================+\n";
		cout << "  |    CHESS GAME - NUCES OOP Lab Project   |\n";
		cout << "  +==========================================+\n\n";
		resetColor();

		setColor(3, 0);
		cout << "  Enter Player 1 name (WHITE): ";
		resetColor();
		getline(cin, player1);
		if (player1.empty()) player1 = "Player1";

		setColor(3, 0);
		cout << "  Enter Player 2 name (BLACK): ";
		resetColor();
		getline(cin, player2);
		if (player2.empty()) player2 = "Player2";

		run();
	}

	void run() {
		while (!gameOver) {
			board.display(player1, player2,
				whiteCaptured, blackCaptured, moveCount);

			string curName = (currentTurn == 'W') ? player1 : player2;
			string colorStr = (currentTurn == 'W') ? "WHITE" : "BLACK";

			setColor(10, 0);
			cout << "  [" << colorStr << "] " << curName
				<< " - Enter move (e.g. e2 e4) or 'quit': ";
			resetColor();

			string input;
			getline(cin, input);

			if (input == "quit" || input == "exit") {
				printMsg("Game ended. Thanks for playing!", 14);
				break;
			}

			int fr, fc, tr, tc;
			if (!board.parseInput(input, fr, fc, tr, tc)) {
				printMsg("Bad format! Try:  e2 e4", 12);
				cout << "  Press Enter..."; cin.get();
				continue;
			}

			Piece* src = board.getPiece(fr, fc);
			if (!src) {
				printMsg("No piece there!", 12);
				cout << "  Press Enter..."; cin.get();
				continue;
			}
			if (src->getColor() != currentTurn) {
				printMsg("That is not your piece!", 12);
				cout << "  Press Enter..."; cin.get();
				continue;
			}

			// Check if destination has a king
			Piece* dst = board.getPiece(tr, tc);
			bool kingTarget = dst && dst->getSymbol() == 'K';

			char res = board.makeMove(fr, fc, tr, tc);
			if (res == 0) {
				printMsg("Illegal move for this piece!", 12);
				cout << "  Press Enter..."; cin.get();
				continue;
			}

			if (res != ' ') {
				if (currentTurn == 'W') whiteCaptured++;
				else                    blackCaptured++;
				setColor(13, 0);
				cout << "  Captured: " << res << "\n";
				resetColor();
			}

			if (kingTarget) {
				board.display(player1, player2,
					whiteCaptured, blackCaptured, moveCount);
				setColor(14, 0);
				cout << "\n  *** CHECKMATE! ***\n";
				cout << "  " << curName << " (" << colorStr << ") WINS THE GAME!\n";
				cout << "  Congratulations, " << curName << "!\n\n";
				resetColor();
				gameOver = true;
				break;
			}

			if (currentTurn == 'W') { currentTurn = 'B'; }
			else { currentTurn = 'W'; moveCount++; }
		}

		setColor(13, 0);
		cout << "  *** GAME OVER ***\n";
		cout << "  Thank you for playing! - NUCES OOP Project\n\n";
		resetColor();
	}
};

// ============================================================
//  MAIN
// ============================================================
int main() {
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	SetConsoleTitle("Chess Game - NUCES OOP Project");
#endif

	Game game;
	game.start();
	system("pause");
	return 0;
}