#include "functions.h"


void menu(){
	HANDLE Buffer = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	//HANDLE buf2 = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_INTENSITY);

	CONSOLE_CURSOR_INFO cursor;
	cursor.bVisible = FALSE;
	cursor.dwSize = 1;
	DWORD wr = 0,inputCode=0;
	INPUT_RECORD input;
	SetConsoleCursorInfo(Buffer, &cursor);
	//SetConsoleCursorInfo(buf2, &cursor);
	COORD coord = { 25, 2 };

	coord.X = 80;
	coord.Y = 30;
	SetConsoleScreenBufferSize(Buffer, coord);
	//SetConsoleScreenBufferSize(buf2, coord);

	SetConsoleActiveScreenBuffer(Buffer);

	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r);
	MoveWindow(console, r.left, r.top, 1000, 1000, TRUE);




	SetConsoleTitle(L"Console Tetris 1.4");
	coord.X = 35;
	coord.Y = 2;
	DrawText(Buffer, coord, L"Tetris 1.4", VERT);
	
	/////////////////////////////////////
	coord.X = 0;
	coord.Y = 0;
	DrawHLine(Buffer, coord, 80);//HAut
	DrawVLine(Buffer, coord, 30);//Gauche

	coord.Y = 29;
	DrawHLine(Buffer, coord, 80);//bas

	coord.X = 79;
	coord.Y = 0;
	DrawVLine(Buffer, coord, 30);//droite


	coord.X = 0;
	coord.Y = 4;
	DrawHLine(Buffer, coord, 80);
	///////////////////////////////////////


	
	WORD Score = JAUNE,Ngame = VERT;
	coord.Y = 6;
	coord.X = 30;
	DrawText(Buffer, coord, L"-->", Ngame);
	while(true)
	{
		
		PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &input, 1, &wr);
		inputCode = 0;
		//la recup de la touche
		if (wr)
		{
			if (input.EventType == KEY_EVENT)
			{
				if (input.Event.KeyEvent.bKeyDown)
				{
					if (input.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
					{
						exit(0);
						break;
					}
					else
						inputCode = input.Event.KeyEvent.wVirtualKeyCode;
				}
			}

			FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
		}

		//Couleur du texte du menu selon la flèche appuiyée
		if ( inputCode == VK_DOWN || inputCode == VK_RIGHT)
		{
			Score = VERT;
			Ngame = JAUNE;
			coord.Y = 8;
			coord.X = 30;
			DrawText(Buffer, coord, L"-->", Score);
			coord.Y = 6;
			coord.X = 30;
			DrawText(Buffer, coord, L"   ");
		}else if(inputCode == VK_UP || inputCode == VK_LEFT)
		{
			Score = JAUNE; 
			Ngame = VERT;
			coord.Y = 6;
			coord.X = 30;
			DrawText(Buffer, coord, L"-->", Ngame);
			coord.Y = 8;
			coord.X = 30;
			DrawText(Buffer, coord, L"   ");
		}else if(inputCode == VK_RETURN && (Ngame==VERT || Score==VERT) )
		{
			break;
		}
		
		coord.X = 33;
		coord.Y = 6;
		DrawText(Buffer, coord, L"Nouvelle Partie", Ngame);

		
		coord.Y = 8;
		DrawText(Buffer, coord, L"Les Score", Score);
		coord.Y = 10;
		DrawText(Buffer, coord, L"[ESC] Quiter",ROUGE);

	}

	//Nouvelle partie du jeu
	

	if (Ngame == VERT)
	{
		start();

	}if(Score==VERT)
	{
		score();
		menu();
	}


	//fermeture des Handler
	SetConsoleActiveScreenBuffer(hStdout);
	CloseHandle(Buffer);
	
	//system("start Tetris.exe");
	//exit(0);
}



void DrawHLine(HANDLE console, COORD coord, WORD len, WORD attrs)
{
	DWORD wr;

	FillConsoleOutputCharacterA(console, ' ', len, coord, &wr);
	FillConsoleOutputAttribute(console, attrs, len, coord, &wr);
}

void DrawText(HANDLE console, COORD coord, wchar_t* text, WORD attrs)
{
	DWORD wr;
	DWORD len = lstrlenW(text);

	WriteConsoleOutputCharacter(console, text, len, coord, &wr);
	FillConsoleOutputAttribute(console, attrs, len, coord, &wr);
}
void DrawText(HANDLE console, COORD coord, string &text, WORD attrs)
{
	wchar_t *temp;
	temp = new wchar_t[text.length()];
	for (unsigned int i = 0; i < text.length();i++)
	{
		temp[i]= text[i];
	}
	DWORD len = text.length();
	DWORD wr;
	WriteConsoleOutputCharacter(console, temp, len, coord, &wr);
	FillConsoleOutputAttribute(console, attrs, len, coord, &wr);
	delete[]temp;
}
void DrawVLine(HANDLE console, COORD coord, WORD len, WORD attrs)
{
	DWORD wr;

	for (size_t i = 0; i < len; i++)
	{
		coord.Y++;
		WriteConsoleOutputCharacterA(console, " ", 1, coord, &wr);
		WriteConsoleOutputAttribute(console, &attrs, 1, coord, &wr);
	}
}
void score()
{
	SetConsoleTitle(L"Scores");
	HANDLE Buffer = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_INTENSITY);

	CONSOLE_CURSOR_INFO cursor;
	cursor.bVisible = FALSE;
	cursor.dwSize = 1;
	DWORD wr = 0, inputCode = 0;
	INPUT_RECORD input;
	SetConsoleCursorInfo(Buffer, &cursor);
	//SetConsoleCursorInfo(buf2, &cursor);
	COORD coord = { 25, 2 };

	coord.X = 80;
	coord.Y = 30;
	SetConsoleScreenBufferSize(Buffer, coord);
	//SetConsoleScreenBufferSize(buf2, coord);

	SetConsoleActiveScreenBuffer(Buffer);

	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r);
	MoveWindow(console, r.left, r.top, 1000, 1000, TRUE);
	coord.X = 35;
	coord.Y = 2;
	DrawText(Buffer, coord, L"Tetris 1.3", VERT);

	/////////////////////////////////////
	coord.X = 0;
	coord.Y = 0;
	DrawHLine(Buffer, coord, 80);//HAut
	DrawVLine(Buffer, coord, 30);//Gauche

	coord.Y = 29;
	DrawHLine(Buffer, coord, 80);//bas

	coord.X = 79;
	coord.Y = 0;
	DrawVLine(Buffer, coord, 30);//droite


	coord.X = 0;
	coord.Y = 4;
	DrawHLine(Buffer, coord, 80);//milieu
	///////////////////////////////////////
	
	//afficher les scores
	string temp[6] = { "NAN", "0", "NAN", "0", "NAN", "0" };
	GetScore(temp);
	coord.X = 30;

	int k = 1;
	coord.Y = 6;
	for (int i = 0; i < 5;i+=2)
	{
		
		DrawText(Buffer, coord, to_string(k)+"-"+temp[i]);k++;
	coord.X += temp[i].length()+4;
	DrawText(Buffer, coord, "-> "+temp[i+1]);
	coord.X = 30;
	coord.Y += 2;
	
	}
	WORD retour = VERT;


	while (true)
	{

		PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &input, 1, &wr);
		inputCode = 0;
		//la recup de la touche
		if (wr)
		{
			if (input.EventType == KEY_EVENT)
			{
				if (input.Event.KeyEvent.bKeyDown)
				{
					if (input.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
					{
						
						break;
					}
					else
						inputCode = input.Event.KeyEvent.wVirtualKeyCode;
				}
			}

			FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
		}
		coord.X = 33;
		coord.Y = 12;
		DrawText(Buffer, coord, L"Retour", retour);


		coord.Y = 14;
		DrawText(Buffer, coord, L"[ESC] Quiter", ROUGE);
		coord.Y = 12;
		coord.X = 30;
		DrawText(Buffer, coord, L"-->", retour);
		if(inputCode==VK_RETURN)
		{
			main();
		}

		
	}
	
	SetConsoleTextAttribute(hStdout, GRIS);
	SetConsoleActiveScreenBuffer(hStdout);
	CloseHandle(Buffer);
	cursor.bVisible = TRUE;

}
bool glbl_draw_points = true;


typedef struct FIGURE
{
	struct
	{
		BYTE x;
		BYTE y;
	}
	size;

	COORD position;
	WORD attr;
	BYTE c[4][4];
}
FIGURE, *PFIGURE;

void DrawFigure(HANDLE console, PFIGURE fig, bool black = false)
{
	DWORD wr;
	COORD c;
	WORD noattr = 0;

	for (WORD i = 0; i < fig->size.x; i++)
	{
		for (WORD j = 0; j < fig->size.y; j++)
		{
			if (fig->c[i][j] == 1)
			{
				c.X = i + fig->position.X;
				c.Y = j + fig->position.Y;

				if (black)
				{
					WriteConsoleOutputCharacter(console, L" ", 1, c, &wr);
					WriteConsoleOutputAttribute(console, &noattr, 1, c, &wr);
				}
				else
				{
					WriteConsoleOutputCharacter(console, glbl_draw_points ? L"" : L" ", 1, c, &wr);
					WriteConsoleOutputAttribute(console, &fig->attr, 1, c, &wr);
				}
			}
		}
	}
}

void DrawShadowFigure(HANDLE console, BYTE matrix[][30], PFIGURE fig, bool black = false, WORD attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
{
	WORD height = 28, noattr = 0;
	DWORD wr;

	for (WORD i = fig->position.X - 26; i < fig->size.x + fig->position.X - 26; i++)
	{
		for (WORD j = fig->position.Y + 1; j < 30; j++)
		{
			if (matrix[i][j] == 1 && j - 1 < height)
				height = j - 1;
		}
	}

	for (WORD i = fig->position.X; i < fig->size.x + fig->position.X; i++)
	{
		COORD c = { i, height };

		if (black)
		{
			WriteConsoleOutputCharacter(console, L" ", 1, c, &wr);
			WriteConsoleOutputAttribute(console, &noattr, 1, c, &wr);
		}
		else
		{
			if (glbl_draw_points)
			{
				WriteConsoleOutputCharacter(console, L"+", 1, c, &wr);
				WriteConsoleOutputAttribute(console, &attr, 1, c, &wr);
			}
		}
	}
}

void RotateFigure(PFIGURE fig)
{
	FIGURE newfig;
	newfig.size.x = fig->size.y;
	newfig.size.y = fig->size.x;
	newfig.attr = fig->attr;
	newfig.position.X = fig->position.X;
	newfig.position.Y = fig->position.Y;

	for (size_t i = 0; i < fig->size.x; i++)
	{
		for (size_t j = 0; j < fig->size.y; j++)
			newfig.c[j][i] = fig->c[i][fig->size.y - j - 1];
	}

	memcpy((char*)fig, (char*)&newfig, sizeof(FIGURE));
}

bool FillMatrix(BYTE matrix[][30], PFIGURE fig)
{
	for (size_t i = 0; i < fig->size.x; i++)
	{
		for (size_t j = 0; j < fig->size.y; j++)
		{
			if (fig->c[i][j] == 1)
				matrix[fig->position.X + i - 26][fig->position.Y + j] = 1;
		}
	}

	return fig->position.Y != 1;
}

bool GetMatrixHeight(BYTE matrix[][30], PFIGURE fig)
{
	for (WORD i = 0; i < fig->size.x; i++)
	{
		for (WORD j = fig->size.y; j > 0; j--)
		{
			if (fig->c[i][j - 1] == 1)
			{
				if (matrix[fig->position.X + i - 26][fig->position.Y + j] == 1)
					return true;
			}
		}
	}

	if (fig->position.Y + fig->size.y >= 29)
	{
		for (WORD i = 0; i < fig->size.x; i++)
		{
			if (fig->c[i][fig->size.y - 1] == 1)
				return true;
		}
	}

	return false;
}


bool GetMatrixRotHeight(BYTE matrix[][30], PFIGURE fig)
{
	FIGURE rotfig(*fig);
	RotateFigure(&rotfig);

	if (rotfig.position.Y + rotfig.size.y >= 30)
	{
		for (WORD i = 0; i < rotfig.size.x; i++)
		{
			if (rotfig.c[i][rotfig.size.y - 1] == 1)
				return true;
		}
	}

	return false;
}


bool CanMoveLeft(BYTE matrix[][30], PFIGURE fig)
{
	for (WORD i = 0; i < fig->size.x; i++)
	{
		for (WORD j = 0; j < fig->size.y; j++)
		{
			if (fig->c[i][j] == 1)
			{
				if (matrix[fig->position.X + i - 26 - 1][fig->position.Y + j] == 1)
					return false;
			}
		}
	}

	return true;
}

bool CanMoveRight(BYTE matrix[][30], PFIGURE fig)
{
	for (WORD i = fig->size.x; i > 0; i--)
	{
		for (WORD j = 0; j < fig->size.y; j++)
		{
			if (fig->c[i - 1][j] == 1)
			{
				if (matrix[fig->position.X + i - 26][fig->position.Y + j] == 1)
					return false;
			}
		}
	}
	return true;
}

WORD GetRightRotationCollision(BYTE matrix[][30], PFIGURE fig)
{
	FIGURE rotfig(*fig);
	RotateFigure(&rotfig);
	WORD coll = 0;

	for (WORD j = 0; j < rotfig.size.y; j++)
	{
		for (WORD i = 0; i < rotfig.size.x; i++)
		{
			if (rotfig.c[i][j] == 1)
			{
				if (matrix[rotfig.position.X + i - 26][rotfig.position.Y + j] == 1 && coll < rotfig.size.x - i)
					coll = rotfig.size.x - i;
			}
		}
	}

	return coll;
}

void MoveMatrixDown(HANDLE buf, BYTE matrix[][30], WORD height)
{
	wchar_t chr;
	DWORD wr;
	WORD attr;

	for (WORD j = height; j >= 1; j--)
	{
		for (WORD i = 0; i < 24; i++)
		{
			matrix[i][j] = matrix[i][j - 1];

			if (j > 1)
			{
				COORD c = { i + 26, j - 1 };
				ReadConsoleOutputCharacter(buf, &chr, 1, c, &wr);
				ReadConsoleOutputAttribute(buf, &attr, 1, c, &wr);
				c.Y = j;
				WriteConsoleOutputCharacter(buf, &chr, 1, c, &wr);
				WriteConsoleOutputAttribute(buf, &attr, 1, c, &wr);
			}
		}
	}
}

bool CheckMatrix(HANDLE buf, BYTE matrix[][30])
{
	bool row;
	for (WORD j = 1; j < 29; j++)
	{
		row = true;
		for (size_t i = 0; i < 24; i++)
		{
			if (matrix[i][j] == 0)
			{
				row = false;
				break;
			}
		}

		if (row)
		{
			for (size_t i = 0; i < 24; i++)
			{
				matrix[i][j] = 0;
			}

			COORD coord;
			coord.X = 26;
			coord.Y = j;
			DrawHLine(buf, coord, 24, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
			Sleep(200);
			DrawHLine(buf, coord, 24, 0);

			MoveMatrixDown(buf, matrix, j);
			return true;
		}
	}

	return false;
}

void DebugMatrix(HANDLE buf, BYTE matrix[][30])
{
	wchar_t bb[12];
	COORD crd = { 1, 1 };

	for (size_t j = 1; j < 29; j++)
	{
		for (size_t i = 0; i < 24; i++)
		{
			wsprintf(bb, L"%u", matrix[i][j]);
			DrawText(buf, crd, bb, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			crd.X += 1;
		}
		crd.Y += 1;
		crd.X = 1;
	}
}

void PrintCount(HANDLE buf, DWORD points)
{

	wchar_t bb[24];
	COORD coord;
	coord.X = 52;
	coord.Y = 6;

	if (points == 0)
		wsprintf(bb, L"Score: 0              ");
	else
	{
		wsprintf(bb, L"Score: %u", points);

	}
	DrawText(buf, coord, bb, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);


}

void PrintDifficulty(HANDLE buf, DWORD difficulty, DWORD next)
{
	wchar_t bb[36];
	COORD coord;
	coord.X = 52;
	coord.Y = 8;

	if (difficulty == 0)
		wsprintf(bb, L"Difficulté: 1            ");
	else
		wsprintf(bb, L"Difficulté: %u", difficulty + 1);

	DrawText(buf, coord, bb, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);

	if (next)
	{
		coord.Y = 10;
		wsprintf(bb, L"Score suivant: %u     ", next);
		DrawText(buf, coord, bb, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	}
	else
	{
		DrawText(buf, coord, L"Erreur     ", FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	}
}

void EmptyScreen(HANDLE buf)
{
	DWORD wr;

	for (WORD j = 1; j < 29; j++)
	{
		COORD c = { 26, j };
		FillConsoleOutputAttribute(buf, 0, 24, c, &wr);
		FillConsoleOutputCharacter(buf, ' ', 24, c, &wr);
	}
}

void EmptyMatrix(BYTE **matrix)
{
	memset(matrix, 0, 24 * 30);
}

void PrintGameOver(HANDLE buf, bool pr = true)
{
	COORD coord;
	coord.X = 52;
	coord.Y = 15;
	if (pr)
	{
		DrawText(buf, coord, L"Game Over! ", FOREGROUND_RED | FOREGROUND_INTENSITY);
		coord.Y++;
		DrawText(buf, coord, L"ENTER - Rejouer", FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	}
	else
	{
		DrawText(buf, coord, L"                      ", FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		coord.Y++;
		DrawText(buf, coord, L"                      ", FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	}
}
void PrintHightScore(HANDLE buf)//
{
	wchar_t bb[24];
	DWORD points = GetHeightScore();
	COORD coord;
	coord.X = 52;
	coord.Y = 12;


	wsprintf(bb, L"Meilleur Score: %u", points);

	DrawText(buf, coord, bb, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}

void start()
{
	//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_INTENSITY);


	DWORD dif_table[] = { 100, 200, 500, 1000, 1700, 2500, 3500, 5000, 8000, 12000, 18000, 25000, 34000, 42000, 55000, 75000, 100000, 150000, 300000, 0 };
	FIGURE fig[13];
	memset((char*)&fig[0], 0, sizeof(FIGURE)* 13);

	fig[0].attr = BACKGROUND_BLUE | BACKGROUND_INTENSITY;
	fig[0].c[0][0] = 1;
	fig[0].c[1][0] = 1;
	fig[0].c[0][1] = 1;
	fig[0].c[1][1] = 1;
	fig[0].size.x = 2;
	fig[0].size.y = 2;

	fig[1].attr = BACKGROUND_RED | BACKGROUND_INTENSITY;
	fig[1].c[0][0] = 1;
	fig[1].c[0][1] = 1;
	fig[1].c[0][2] = 1;
	fig[1].c[0][3] = 1;
	fig[1].size.x = 1;
	fig[1].size.y = 4;

	fig[2].attr = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
	fig[2].c[0][0] = 1;
	fig[2].c[0][1] = 1;
	fig[2].c[0][2] = 1;
	fig[2].c[1][0] = 0;
	fig[2].c[1][1] = 0;
	fig[2].c[1][2] = 1;
	fig[2].size.x = 2;
	fig[2].size.y = 3;

	fig[3].attr = BACKGROUND_GREEN | BACKGROUND_INTENSITY;
	fig[3].c[0][0] = 0;
	fig[3].c[0][1] = 0;
	fig[3].c[0][2] = 1;
	fig[3].c[1][0] = 1;
	fig[3].c[1][1] = 1;
	fig[3].c[1][2] = 1;
	fig[3].size.x = 2;
	fig[3].size.y = 3;

	fig[4].attr = BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
	fig[4].c[0][0] = 1;
	fig[4].c[0][1] = 0;
	fig[4].c[1][0] = 1;
	fig[4].c[1][1] = 1;
	fig[4].c[2][0] = 0;
	fig[4].c[2][1] = 1;
	fig[4].size.x = 3;
	fig[4].size.y = 2;

	fig[5].attr = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
	fig[5].c[0][0] = 0;
	fig[5].c[0][1] = 1;
	fig[5].c[1][0] = 1;
	fig[5].c[1][1] = 1;
	fig[5].c[2][0] = 1;
	fig[5].c[2][1] = 0;
	fig[5].size.x = 3;
	fig[5].size.y = 2;
	//T
	fig[6].attr = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
	fig[6].c[0][0] = 0;
	fig[6].c[0][1] = 1;
	fig[6].c[1][0] = 1;
	fig[6].c[1][1] = 1;
	fig[6].c[2][0] = 0;
	fig[6].c[2][1] = 1;
	fig[6].size.x = 3;
	fig[6].size.y = 2;
	//--
	fig[7].attr = BACKGROUND_RED;
	fig[7].c[0][0] = 1;
	fig[7].c[0][1] = 1;
	fig[7].size.x = 1;
	fig[7].size.y = 2;

	fig[8].attr = BACKGROUND_BLUE | BACKGROUND_RED;
	fig[8].c[0][0] = 0;
	fig[8].c[0][1] = 1;
	fig[8].c[0][2] = 0;
	fig[8].c[1][0] = 1;
	fig[8].c[1][1] = 1;
	fig[8].c[1][2] = 1;
	fig[8].c[2][0] = 0;
	fig[8].c[2][1] = 1;
	fig[8].c[2][2] = 0;
	fig[8].size.x = 3;
	fig[8].size.y = 3;

	fig[9].attr = BACKGROUND_GREEN;
	fig[9].c[0][0] = 1;
	fig[9].c[0][1] = 1;
	fig[9].c[1][0] = 1;
	fig[9].c[1][1] = 0;
	fig[9].c[2][0] = 1;
	fig[9].c[2][1] = 1;
	fig[9].size.x = 3;
	fig[9].size.y = 2;

	fig[10].attr = BACKGROUND_GREEN | BACKGROUND_BLUE;
	fig[10].c[0][0] = 1;
	fig[10].c[0][1] = 0;
	fig[10].c[0][2] = 0;
	fig[10].c[1][0] = 1;
	fig[10].c[1][1] = 1;
	fig[10].c[1][2] = 1;
	fig[10].c[2][0] = 0;
	fig[10].c[2][1] = 0;
	fig[10].c[2][2] = 1;
	fig[10].size.x = 3;
	fig[10].size.y = 3;


	fig[11].attr = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
	fig[11].c[0][0] = 1;
	fig[11].c[0][1] = 1;
	fig[11].c[0][2] = 1;
	fig[11].c[1][0] = 1;
	fig[11].c[1][1] = 1;
	fig[11].c[1][2] = 1;
	fig[11].c[2][0] = 1;
	fig[11].c[2][1] = 1;
	fig[11].c[2][2] = 1;
	fig[11].size.x = 3;
	fig[11].size.y = 3;

	fig[12].attr = BACKGROUND_BLUE | BACKGROUND_INTENSITY;
	fig[12].c[0][0] = 0;
	fig[12].c[0][1] = 0;
	fig[12].c[0][2] = 1;
	fig[12].c[1][0] = 1;
	fig[12].c[1][1] = 1;
	fig[12].c[1][2] = 1;
	fig[12].c[2][0] = 1;
	fig[12].c[2][1] = 0;
	fig[12].c[2][2] = 0;
	fig[12].size.x = 3;
	fig[12].size.y = 3;

	HANDLE buf = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	HANDLE buf2 = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_INTENSITY);
	DWORD wr = 0;

	COORD coord;

	CONSOLE_CURSOR_INFO cursor;
	cursor.bVisible = FALSE;
	cursor.dwSize = 1;

	SetConsoleCursorInfo(buf, &cursor);
	SetConsoleCursorInfo(buf2, &cursor);

	coord.X = 80;
	coord.Y = 30;
	SetConsoleScreenBufferSize(buf, coord);
	SetConsoleScreenBufferSize(buf2, coord);

	SetConsoleActiveScreenBuffer(buf);

	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r);
	MoveWindow(console, r.left, r.top, 1000, 1000, TRUE);

	INPUT_RECORD input;

	BYTE matrix[24][30];
	memset((char*)&matrix[0][0], 0, sizeof(matrix));
	coord.X = 4;
	coord.Y = 2;
	DrawText(buf, coord, L"Merci d'avoir");
	coord.Y = 6;
	coord.X = 5;
	DrawText(buf, coord, L"(c)Hellow-Dev");
	coord.Y = 4;
	coord.X = 1;
	string ver = VER;
	DrawText(buf, coord, "telecharge tetris " + ver);
	coord.X = 25;
	coord.Y = 0;
	DrawHLine(buf, coord, 55);
	DrawVLine(buf, coord, 30);

	coord.Y = 29;
	DrawHLine(buf, coord, 80);

	coord.X = 79;
	coord.Y = 0;
	DrawVLine(buf, coord, 30);

	coord.X = 50;
	DrawVLine(buf, coord, 30);

	coord.X = 25;
	DrawVLine(buf, coord, 30);

	coord.X = 50;
	coord.Y = 4;
	DrawHLine(buf, coord, 29);

	coord.X = 56;
	coord.Y = 2;
	DrawText(buf, coord, "Tetris " + ver);

	for (size_t i = 0; i < 13; i++)
	{
		fig[i].position.X = 62;
		fig[i].position.Y = 19;
	}

	coord.X = 52;
	coord.Y = 20;
	DrawText(buf, coord, L"Next:");

	coord.Y = 25;
	DrawText(buf, coord, L"F   - Repère");
	coord.Y = 26;
	DrawText(buf, coord, L"P   - Pause");
	coord.Y = 27;
	DrawText(buf, coord, L"M   - Music ON/OFF");
	coord.Y = 28;
	DrawText(buf, coord, L"ESC - Quiter");

	PrintCount(buf, 0);
	//RegisterHightScore(0);
	PrintHightScore(buf);

	DWORD defspeed = 21, difficulty = 0, SPEED = defspeed, points = 0, combo, counter = 0, inputCode, nextfig;
	PrintDifficulty(buf, difficulty, dif_table[0]);

	bool fig_exists = false, gameover = false, changed = false, play_music = true;
	PFIGURE newfig = NULL, oldfig = new FIGURE;

	SYSTEMTIME time;
	GetSystemTime(&time);
	srand(time.wSecond + time.wMinute * 60 + time.wHour * 3600);

	nextfig = rand() % 7;

	DebugMatrix(buf, matrix);

	uFMOD_PlaySong((void*)1, NULL, XM_RESOURCE);

	while (true)
	{
		if (gameover)
		{
			PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &input, 1, &wr);

			if (wr)
			{
				if (input.EventType == KEY_EVENT)
				{
					if (input.Event.KeyEvent.bKeyDown)
					{
						if (input.Event.KeyEvent.wVirtualKeyCode == VK_RETURN)
						{
							gameover = false;

							EmptyScreen(buf);
							EmptyMatrix((BYTE**)&matrix);
							DebugMatrix(buf, matrix);
							PrintDifficulty(buf, 0, dif_table[0]);
							PrintCount(buf, 0);
							//RegisterHightScore(0);
							PrintHightScore(buf);

							PrintGameOver(buf, false);
						}
						else if (input.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
						{
							break;
						}
					}
				}

				FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			}
		}
		else
		{
			if (!fig_exists)
			{
				newfig = new FIGURE(fig[nextfig]);
				DrawFigure(buf, &fig[nextfig], true);
				nextfig = rand() % (difficulty <= 7 ? 7 : 13);

				DrawFigure(buf, &fig[nextfig]);

				newfig->position.Y = 1;
				newfig->position.X = 37;

				for (size_t i = 0, rn = rand() % 4; i < rn; i++)
					RotateFigure(newfig);

				fig_exists = true;
				memcpy((char*)oldfig, (char*)newfig, sizeof(FIGURE));

				changed = true;
			}

			PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &input, 1, &wr);

			inputCode = 0;

			if (wr)
			{
				if (input.EventType == KEY_EVENT)
				{
					if (input.Event.KeyEvent.bKeyDown)
					{
						if (input.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
							break;
						else
							inputCode = input.Event.KeyEvent.wVirtualKeyCode;
					}
				}

				FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			}

			if (inputCode == VK_UP)
			{
				if (!GetMatrixRotHeight(matrix, newfig))
				{
					WORD rot = GetRightRotationCollision(matrix, newfig);
					if (newfig->position.X - rot >= 26)
					{
						newfig->position.X -= rot;

						if (GetRightRotationCollision(matrix, newfig) == 0)
						{
							WORD diff = newfig->position.X + newfig->size.y - 1;

							if (newfig->size.y > newfig->size.x && diff > 49)
								newfig->position.X -= diff - 49;

							if (GetRightRotationCollision(matrix, newfig) == 0)
							{
								RotateFigure(newfig);
								changed = true;
							}
							else
							{
								newfig->position.X += diff - 49;
							}
						}
						else
						{
							newfig->position.X += rot;
						}
					}
				}
			}
			else if (inputCode == VK_LEFT && newfig->position.X > 26 && CanMoveLeft(matrix, newfig))
			{
				changed = true;
				newfig->position.X--;
			}
			else if (inputCode == VK_RIGHT && newfig->position.X + newfig->size.x <= 49 && CanMoveRight(matrix, newfig))
			{
				changed = true;
				newfig->position.X++;
			}
			else if (inputCode == VK_DOWN)
			{
				changed = true;
				SPEED = 1;
				counter = 0;
			}
			else if (inputCode == 'm' || inputCode == 'M')
			{
				if (play_music)
					uFMOD_Pause();
				else
					uFMOD_Resume();

				play_music = !play_music;
			}
			else if (inputCode == 'f' || inputCode == 'F')
			{
				glbl_draw_points = !glbl_draw_points;
			}

			if (changed)
			{
				DrawFigure(buf, oldfig, true);
				DrawShadowFigure(buf, matrix, oldfig, true);
			}

			memcpy((char*)oldfig, (char*)newfig, sizeof(FIGURE));

			changed = false;

			if (GetMatrixHeight(matrix, newfig))
			{
				DrawFigure(buf, newfig);
				if (counter >= SPEED)
				{
					fig_exists = false;
					if (!FillMatrix(matrix, newfig))
					{
						points = 0;
						defspeed = 21;
						difficulty = 0;
						gameover = true;
						PrintGameOver(buf);
					}

					delete newfig;
					SPEED = defspeed;
					counter = 0;

					combo = 1;

					if (!gameover)
					{
						while (CheckMatrix(buf, matrix))
						{
							points += 100 * (8 - defspeed / 3)*combo;
							RegisterHightScore(points);

							PrintHightScore(buf);
							PrintCount(buf, points);


							combo += 1;

							while (difficulty < 19 && points >= dif_table[difficulty])
							{
								difficulty++;
								defspeed--;
								PrintDifficulty(buf, difficulty, dif_table[difficulty]);
							}
						}
					}

					DebugMatrix(buf, matrix);
				}
			}
			else if (fig_exists && counter == SPEED)
			{
				DrawShadowFigure(buf, matrix, newfig);
				DrawFigure(buf, newfig);

				newfig->position.Y++;
				changed = true;
				counter = 0;
			}
			else
			{
				DrawShadowFigure(buf, matrix, newfig);
				DrawFigure(buf, newfig);
			}


			if (inputCode == 'p' || inputCode == 'P')
			{
				if (play_music)
					uFMOD_Pause();

				SetConsoleActiveScreenBuffer(buf2);

				DWORD bear = 0;

				coord.X = 24;
				coord.Y = 1;
				DrawText(buf2, coord, L":::::::                                                 \
                     ,:::,...,:::                                               \
                    ,::..++++?..::.                     ,..::,.                 \
                    ::..+++++++..::....::::::,........,::::,,:::                \
                    ::.+++++...:::::::::::::~~~~~~~:..,:...?++~.:               \
                    ::.+++..::::::::::::::~~~~~~~~~~~~,..+++++++:               \
                    :::...:::::::::::::~~~~~~~~~~~~~~~~~~..++++.:               \
                     :::::::::::::::::~~~~~~~~~~~~~~~~~~~~,.++.:                \
                       ::::::::::::::~~~~~~~~~~~~~~~~~~~~~~~..:                 \
                      ::::::::::::::~~~~~~~~~~~~~~~~~~~~~~~~~,                  \
                     :::::::::::::::~~~~~~~~~~~~~~~~~~~~~~~~~                   \
                    ::::::::::::.....:~~~~~~~~~~~~~~~~~~~~~~~~                  \
                   ::::::::::::.......:~~~~~~~~~~~~~~~,...:~~~                  \
                   ,:::::::::::.......~~~~~~~~~~~~~~~......~~~~                 \
                   :::::::::::::.....,~~~~~~~~~~~~~~........~~~                 \
                  ::::::::::::::::~~~~~~~~~~~~~~~~~~~.......~~~                 \
                  :::::::::::::::::~~~~~~~~~~~~~~~~~~:....~~~~~                 \
                  :::::::::::::::::..?++++++,.++++=~~~~~~~~~~~~                 \
                 ::::::::::::::::..?++++++......+++++..~~~~~~~~                 \
                :::::::::::::::::.++++++++......++++++.:~~~~~~~~                \
               ::::::::::::::::::.?++++++++,...+++++++.:~~~~~~~~                \
            ,.::::::::::::::::::::..++++?.....~++++++~.~~~~~~~~~                \
           ,.:,..:::::::::::::::::::..~.IIIIII?......,~~~~~~~~~~~               \
                ::::::::::::::::::::..~.IIIIIIII~..~~~~~~~~~~~~~~~              \
               ::::::::::::::::::::::.~..IIIIII~~.~~~~~~~~..~~~                 \
              :,..::..::.:::::::::::,.~~..IIII.~..~~~~.~~,..~~.,                \
                    ..,.:.,.:::::::::,.~~~....~~.:~~~~,...:,                    \
               ,:::::..:::::::::::::~~..:~~~~~~.:~~~~~~..::::~~~                \
			                 :::::::::::::::::::~~~~~~~,.....:~~~~~~~~:::::~~~~~,", 6);


				coord.X = 30;
				coord.Y = 0;
				DrawText(buf2, coord, "Tetris " + ver + " Bear Edition");
				coord.X = 64;
				coord.Y = 29;
				DrawText(buf2, coord, L"P Pour Continuer");

				GetWindowRect(console, &r);
				MoveWindow(console, r.left, r.top, 1000, 1000, TRUE);

				while (true)
				{
					PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &input, 1, &wr);

					if (wr)
					{
						if (input.EventType == KEY_EVENT)
						if (input.Event.KeyEvent.bKeyDown)
						if (input.Event.KeyEvent.wVirtualKeyCode == 'p' || input.Event.KeyEvent.wVirtualKeyCode == 'P')
						{
							FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
							break;
						}
						else if (input.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
						{
							menu();
							break;
						}

						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
					}

					bear++;

					if (bear == 15)
					{
						coord.X = 54;
						coord.Y = 13;
						DrawText(buf2, coord, L"~~~~~", 8);
					}
					else if (bear == 16)
					{
						coord.X = 53;
						coord.Y = 14;
						DrawText(buf2, coord, L"~~~~~~", 8);
					}
					else if (bear == 17)
					{
						coord.X = 52;
						coord.Y = 15;
						DrawText(buf2, coord, L"~~~~~~~~", 8);
					}
					else if (bear == 18)
					{
						coord.X = 53;
						coord.Y = 16;
						DrawText(buf2, coord, L"~~~~~~~", 8);
					}
					else if (bear == 19)
					{
						coord.X = 53;
						coord.Y = 17;
						DrawText(buf2, coord, L"~~~~~", 8);
					}

					else if (bear == 21)
					{
						coord.X = 53;
						coord.Y = 17;
						DrawText(buf2, coord, L":....", 6);
					}
					else if (bear == 22)
					{
						coord.X = 53;
						coord.Y = 16;
						DrawText(buf2, coord, L".......", 6);
					}
					else if (bear == 23)
					{
						coord.X = 52;
						coord.Y = 15;
						DrawText(buf2, coord, L"........", 6);
					}
					else if (bear == 24)
					{
						coord.X = 53;
						coord.Y = 14;
						DrawText(buf2, coord, L"......", 6);
					}
					else if (bear == 25)
					{
						coord.X = 54;
						coord.Y = 13;
						DrawText(buf2, coord, L",...:", 6);
					}
					else if (bear == 65)
					{
						bear = 0;
					}

					Sleep(50);
				}

				SetConsoleActiveScreenBuffer(buf);

				if (play_music)
					uFMOD_Resume();
			}

			Sleep(20);
			counter++;
		}
	}


	delete oldfig;
	SetConsoleTextAttribute(hStdout, GRIS);
	SetConsoleActiveScreenBuffer(hStdout);
	CloseHandle(buf2);
	CloseHandle(buf);
	cursor.bVisible = TRUE;
	uFMOD_Pause();

	RegisterScore(points);
	main();

}