#define _CRT_SECURE_NO_WARNINGS
#include<Windows.h>
#include<conio.h>
#include"HandleCharArrayFunc.h";
#include<ctime>
#include<thread>
#include<fstream>
#include<iostream>
#pragma comment(lib, "winmm.lib")
using namespace std;

//Hằng số
#define MAX_SIZE_SNAKE 10
#define MAX_SIZE_FOOD 4
#define MAX_SPEED 3
//Biến toàn cục
POINT snake[10]; //Con rắn
POINT food[4];	//Thức ăn
int CHAR_LOCK;	//Biến xác định hướng không thể di chuyển (Ở một thời điểm có một hướng rắn không thể duy chuyển)
int MOVING;		//Biến xác định hướng di chuyển của snake (Ở một thời điểm có ba hướng rắn có thể di chuyển)
int SPEED;		//Có thể hiểu như level, level càng cao thì tốc độ càng nhanh
int HEIGH_CONSOLE, WIDTH_CONSOLE;	//Độ rộng và độ cao của màn hình console
int FOOD_INDEX;	//Chỉ số food hiện hành đang có trên màn hình
int SIZE_SNAKE;	//Kích thước của snake, lúc đầu có 6 và tối đa lên tới 10
int STATE;		//Trạng thái sống hay chết của rắn
int SCORE;		//Điểm của rắn = số thức ăn rắn ăn được
char name[] = "1560685";
int nameIndex;
//Cổng
//gate[1]gate[0]gate[2]
//gate[3]		gate[4]
int MAX_SIZE_GATE = 5;
POINT gate[5];
bool gateIsExist;
POINT gateBehind;
int g;

/*------------------------
|0				max width|
|						 |
|						 |
|max heigh				 |
|------------------------*/



void FixConsoleWindow() {
	HWND consoleWindow = GetConsoleWindow();
	LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
	style = style & ~(WS_MAXIMIZEBOX) & ~(WS_THICKFRAME);
	SetWindowLong(consoleWindow, GWL_STYLE, style);
}

void GotoXY(int x, int y) {
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

bool IsValid(int x, int y) {
	for (int i = 0; i < SIZE_SNAKE; i++)
		if (snake[i].x == x && snake[i].y == y)
			return false;
	return true;
}

void GenerateFood() {
	int x, y;
	srand(time(NULL));
	for (int i = 0; i < MAX_SIZE_FOOD; i++) {
		do {
			x = rand() % (WIDTH_CONSOLE - 1) + 1;
			y = rand() % (HEIGH_CONSOLE - 1) + 1;
		} while (IsValid(x, y) == false);
		food[i] = { x, y };
	}
}

void ResetData() {
	//Khởi tạo giá trị toàn cục
	CHAR_LOCK = 'A', MOVING = 'D', SPEED = 1; FOOD_INDEX = 0;
	WIDTH_CONSOLE = 70; HEIGH_CONSOLE = 20, SIZE_SNAKE = 6;
	//Khởi tạo giá trị mặc định cho snake
	snake[0] = { 10,5 }; snake[1] = { 11,5 };
	snake[2] = { 12,5 }; snake[3] = { 13,5 };
	snake[4] = { 14,5 }; snake[5] = { 15,5 };
	GenerateFood();	//Tạo mảng thức ăn food
}

void DrawBoard(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 1);	//Vẽ tường màu xanh dương
	GotoXY(x, y); cout << 'X';
	for (int i = 1; i < width; i++) cout << 'X';
	cout << 'X';
	GotoXY(x, height + y); cout << 'X';
	for (int i = 1; i < width; i++) cout << 'X';
	cout << 'X';
	for (int i = y + 1; i < height + y; i++) {
		GotoXY(x, i); cout << 'X';
		GotoXY(x + width, i); cout << 'X';
	}
	GotoXY(curPosX, curPosY);
}

void StartGame() {
	PlaySound(NULL, NULL, 0);
	system("cls");
	ResetData();	//Khởi tạo dữ liệu gốc
	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);	//Về màn hình game
	STATE = 1;	//Bắt đầu cho Thread chạy
	SCORE = 0;	//Ban đầu điểm = 0
}

void LevelUp() {
	PlaySound("sounds/levelup.wav", NULL, SND_ASYNC);
	system("cls");
	//Khởi tạo giá trị toàn cục
	CHAR_LOCK = 'A', MOVING = 'D'; FOOD_INDEX = 0; SIZE_SNAKE = 10;
	GenerateFood();
	if (SPEED == MAX_SPEED) {	//4.3 Xử lý thiết lập lại từ đầu khi lên MAX_SPEED
		SIZE_SNAKE = 6;
		SPEED = 1;
	}
	else
		SPEED++;					//4.4 Xử lý khi qua cổng mới lên cấp
	for (int i = 0; i < SIZE_SNAKE; i++)
		snake[i] = { i + 1, 1 };
	WIDTH_CONSOLE = 70; HEIGH_CONSOLE = 20;
	//Khởi tạo giá trị mặc định cho snake
	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);	//Về màn hình game
	//reset gate = 0
	for (int i = 0; i < 5; i++)
		gate[i].x = gate[i].y = 0;
}

//Hàm thoát game
void ExitGame(HANDLE t) {
	system("cls");
	TerminateThread(t, 0);
	exit(0);	//Đóng console
}

//Hàm dừng game
void PauseGame(HANDLE t) {
	SuspendThread(t);
	GotoXY(20, HEIGH_CONSOLE + 2);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
	cout << "Press any key to continue...";
	char c;
	c = toupper(_getch());
	if (c) {
		GotoXY(20, HEIGH_CONSOLE + 2);
		cout << "                            ";
		ResumeThread(t);
	}
}

void Write(ostream& outDev) {
	outDev << "level: " << SPEED << endl;
	outDev << "score: " << SCORE << endl;
	outDev << "snake:" << endl;
	for (int i = 0; i < SIZE_SNAKE; i++)
		outDev << snake[i].x << " " << snake[i].y << endl;
	outDev << "food: " << FOOD_INDEX << endl;
	for (int i = 0; i < MAX_SIZE_FOOD; i++)
		outDev << food[i].x << " " << food[i].y << endl;
	outDev << "gate:" << endl;
	for (int i = 0; i < 5; i++)
		outDev << gate[i].x << " " << gate[i].y << endl;
	outDev << "gate behind:" << endl;
	outDev << gateBehind.x << " " << gateBehind.y << endl;
	outDev << "moving: " << MOVING;
}

void SaveGame(HANDLE t) {
	SuspendThread(t);
	char fileName[259];
	char ext[] = ".sa";	//append
	GotoXY(15, HEIGH_CONSOLE + 2);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
	cout << "Enter your name to create save file: ";
	cin >> fileName;
	strcat(fileName, ext);
	char path[] = "save";
	char full[264];
	strcpy(full, path);
	strcat(full, "/");
	strcat(full, fileName);
	DWORD check = GetFileAttributesA(path);
	if (check == INVALID_FILE_ATTRIBUTES)
		CreateDirectory(path, NULL);
	ifstream temp;
	temp.open(full);
	if (temp) {
		do {
			temp.close();
			GotoXY(5, HEIGH_CONSOLE + 2);
			cout << "                                                                      ";
			GotoXY(5, HEIGH_CONSOLE + 2);
			cout << "File name already exists, please enter different file name: ";
			cin >> fileName;
			strcat(fileName, ext);
			strcpy(full, path);
			strcat(full, "/");
			strcat(full, fileName);
			temp.open(full);
		} while (temp.good());
	}
	//
	ofstream file(full);
	Write(file);
	//
	GotoXY(5, HEIGH_CONSOLE + 2);
	cout << "                                                                      ";
	char c;
	GotoXY(10, HEIGH_CONSOLE + 2);
	cout << "Press any key to continue or press E to exit...";
	c = toupper(_getch());
	if (c == 'E')
		ExitGame(t);
	else {
		GotoXY(10, HEIGH_CONSOLE + 2);
		cout << "                                               ";
		ResumeThread(t);
	}
}

void ResizeConsole(int width, int height);
void DrawGate();
void DrawSnakeAndFood(const char* str);
void StartGameWhenLoad(HANDLE handle_t1) {
	ResizeConsole(750, 500);
	WIDTH_CONSOLE = 70; HEIGH_CONSOLE = 20;

	int temp;
	PlaySound(NULL, NULL, 0);
	system("cls");
	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);	//Về màn hình game
	STATE = 1;	//Bắt đầu cho Thread chạy

	if (gate[0].x != 0 && gate[0].y != 0)
		DrawGate();
		
	DrawSnakeAndFood("snake");
	ResumeThread(handle_t1);
	PauseGame(handle_t1);

	while (1) {
		temp = toupper(_getch());
		if (STATE == 1) {
			if (temp == 'P')
				PauseGame(handle_t1);
			else if (temp == 'O')
				SaveGame(handle_t1);
			else if (temp == 27) {
				ExitGame(handle_t1);
				return;
			}
			else {
				ResumeThread(handle_t1);
				if ((temp != CHAR_LOCK) && (temp == 'D' || temp == 'A' || temp == 'W' || temp == 'S')) {
					if (temp == 'D') CHAR_LOCK = 'A';
					else if (temp == 'W') CHAR_LOCK = 'S';
					else if (temp == 'S') CHAR_LOCK = 'W';
					else CHAR_LOCK = 'D';
					MOVING = temp;
				}
			}
		}
		else {
			if (temp == 'Y') StartGame();
			else {
				ExitGame(handle_t1);
				return;
			}
		}
	}
}

void LoadGame(HANDLE handle_t1) {
	SuspendThread(handle_t1);
	ifstream file;
	int level = 0;
	do {
		GotoXY(1, 17);
		for (int i = 0; i < 4; i++)
			cout << "\t\t\t\t\t\t\t             " << endl;
		char fileName[259];
		char ext[] = ".sa";
		GotoXY(1, 17);
		cout << "\t\t\t\t\t\t\t Enter file name: ";
		cin >> fileName;
		strcat(fileName, ext);
		char path[] = "save";
		char full[264];
		strcpy(full, path);
		strcat(full, "/");
		strcat(full, fileName);

		file.open(full);
		if (!file) {
			GotoXY(1, 17);
			cout << "\t\t\t\t\t\t File doestn't exits, please try again...";
			//_getwch();
		}
		GotoXY(1, 17);
		cout << "\t\t\t\t\t\t                                         ";
	} while (!file);


	const int maxSize = 255;
	char buf[maxSize + 1];
	//level
	file.getline(buf, maxSize);
	int n;
	n = strlen(buf + 7);
	for (int i = 0; i < n; i++) {		//Hoặc dùng atoi()
		int temp1, temp2;
		temp1 = (int)buf[7 + i] - 48;
		temp2 = 10 * (n - (i + 1));
		if (temp2 != 0)
			level += temp1 * temp2;
		else
			level += temp1;
	}
	SPEED = level;
	//score
	file.getline(buf, maxSize);
	SCORE = atoi(buf + 7);
	//snake, ở level 4, 8, 12,... size_snake == 6
	SIZE_SNAKE = MAX_SIZE_SNAKE;
	if (SCORE == 0 || SPEED % 4*1.0 == 0)
		SIZE_SNAKE = 6;
	if (SCORE < 4)
		SIZE_SNAKE = 6 + SCORE;
	file.ignore(1000, '\n');
	for (int i = 0; i < SIZE_SNAKE; i++) {
		file.getline(buf, maxSize, ' ');
		snake[i].x = atoi(buf);
		file.getline(buf, maxSize, '\n');
		snake[i].y = atoi(buf);
	}
	//food
	file.ignore(6, ' ');
	file.getline(buf, maxSize);
	FOOD_INDEX = atoi(buf);
	for (int i = 0; i < MAX_SIZE_FOOD; i++) {
		file.getline(buf, maxSize, ' ');
		food[i].x = atoi(buf);
		file.getline(buf, maxSize, '\n');
		food[i].y = atoi(buf);
	}
	file.ignore(5, '\n');
	//gate
	for (int i = 0; i < MAX_SIZE_GATE; i++) {
		file.getline(buf, maxSize, ' ');
		gate[i].x = atoi(buf);
		file.getline(buf, maxSize, '\n');
		gate[i].y = atoi(buf);
	}
	//gate behind
	file.ignore(1000, '\n');
	file.getline(buf, maxSize, ' ');
	gateBehind.x = atoi(buf);
	file.getline(buf, maxSize, '\n');
	gateBehind.y = atoi(buf);
	//moving
	file.ignore(8, ' ');
	file.getline(buf, maxSize);
	MOVING = atoi(buf);
	//
	StartGameWhenLoad(handle_t1);
}

bool IsValidFood(int x, int y) {	//Tránh việc cổng tạo ra trùng với food
	if (food[FOOD_INDEX].x == x && food[FOOD_INDEX].y == y)
		return false;
	return true;
}

//4.4 Xử lý khi ăn xong 4 food ở một cấp, tạo cổng quay về
bool CheckGate() {
	for (int i = 1; i < MAX_SIZE_GATE; i++) {
		if (snake[SIZE_SNAKE - 1].x == gate[i].x && snake[SIZE_SNAKE - 1].y == gate[i].y) {		//Đầu rắn đụng tường
			gateIsExist = false;
			return false;
		}
	}
	if (SIZE_SNAKE >= 2) {
		if (snake[SIZE_SNAKE - 1].x == gate[0].x && snake[SIZE_SNAKE - 1].y == gate[0].y) {			//Đầu rắn vào đúng cổng
			if (snake[SIZE_SNAKE - 2].x == gateBehind.x && snake[SIZE_SNAKE - 2].y == gateBehind.y) {	//Cổ rắn ở vị trí sau cổng, để kt xem rắn có đi ngược vào cổng hay không
				gateIsExist = false;
				return false;
			}
			else	//Nếu rắn vào cổng hợp lệ, giảm kích thước rắn để không hiển thị và tránh đụng vào tường nếu cổng nằm quá gần tường.
				SIZE_SNAKE--;
		}
	}
	if (SIZE_SNAKE == 1)
		//Sau khi đuôi rắn qua cửa mới trả về kết quả đúng để lên cấp
		if (snake[0].x == gate[0].x && snake[0].y == gate[0].y)
			gateIsExist = false;
	return true;
}

void DrawGate() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
	gateIsExist = true;
	if (gate[0].x == 0 && gate[0].y == 0) {	//trường hợp chưa có cổng hoặc file load k có cổng.
		srand(time(NULL));
		do {
			//Tạo giá trị ngẫu nhiên cho điểm giữa cổng
			gate[0].x = rand() % (WIDTH_CONSOLE - 1) + 9;
			gate[0].y = rand() % (HEIGH_CONSOLE - 1) + 7;
		} while (IsValid(gate[0].x, gate[0].y) == false || IsValidFood(gate[0].x, gate[0].y) == false || gate[0].x <= 2 || gate[0].x >= WIDTH_CONSOLE - 2 || gate[0].y <= 2 || gate[0].y >= HEIGH_CONSOLE - 2);
		GotoXY(gate[0].x, gate[0].y);
		cout << "G";

		srand(time(NULL));
		g = rand() % 3 + 0;
		//Cổng kiểu 0. |     |
		//			   |_____|
		if (g == 0) {
			gate[1].x = gate[0].x - 1;
			gate[1].y = gate[0].y;
			GotoXY(gate[1].x, gate[1].y);
			cout << "G";

			gate[2].x = gate[0].x + 1;
			gate[2].y = gate[0].y;
			GotoXY(gate[2].x, gate[2].y);
			cout << "G";

			gate[3].x = gate[1].x;
			gate[3].y = gate[1].y - 1;
			GotoXY(gate[3].x, gate[3].y);
			cout << "G";

			gate[4].x = gate[2].x;
			gate[4].y = gate[2].y - 1;
			GotoXY(gate[4].x, gate[4].y);
			cout << "G";

			gateBehind.x = gate[0].x;
			gateBehind.y = gate[0].y + 1;
		}												//Cổng kiểu 1. |-----|
		else if (g == 1) {								//			   |	 |
			gate[1].x = gate[0].x - 1;
			gate[1].y = gate[0].y;
			GotoXY(gate[1].x, gate[1].y);
			cout << "G";

			gate[2].x = gate[0].x + 1;
			gate[2].y = gate[0].y;
			GotoXY(gate[2].x, gate[2].y);
			cout << "G";

			gate[3].x = gate[1].x;
			gate[3].y = gate[1].y + 1;
			GotoXY(gate[3].x, gate[3].y);
			cout << "G";

			gate[4].x = gate[2].x;
			gate[4].y = gate[2].y + 1;
			GotoXY(gate[4].x, gate[4].y);
			cout << "G";

			gateBehind.x = gate[0].x;
			gateBehind.y = gate[0].y - 1;
		}
		else if (g == 2) {								//Cổng kiểu 2.	|-----
			gate[1].x = gate[0].x;						//				|
			gate[1].y = gate[0].y - 1;					//				|-----
			GotoXY(gate[1].x, gate[1].y);
			cout << "G";

			gate[2].x = gate[0].x;
			gate[2].y = gate[0].y + 1;
			GotoXY(gate[2].x, gate[2].y);
			cout << "G";

			gate[3].x = gate[1].x + 1;
			gate[3].y = gate[1].y;
			GotoXY(gate[3].x, gate[3].y);
			cout << "G";

			gate[4].x = gate[2].x + 1;
			gate[4].y = gate[2].y;
			GotoXY(gate[4].x, gate[4].y);
			cout << "G";

			gateBehind.x = gate[0].x - 1;
			gateBehind.y = gate[0].y;
		}
		else if (g == 3) {								//Cổng kiểu 3.	-----|							
			gate[1].x = gate[0].x;						//					 |
			gate[1].y = gate[0].y - 1;					//				-----|
			GotoXY(gate[1].x, gate[1].y);
			cout << "G";

			gate[2].x = gate[0].x;
			gate[2].y = gate[0].y + 1;
			GotoXY(gate[2].x, gate[2].y);
			cout << "G";

			gate[3].x = gate[1].x - 1;
			gate[3].y = gate[1].y;
			GotoXY(gate[3].x, gate[3].y);
			cout << "G";

			gate[4].x = gate[2].x - 1;
			gate[4].y = gate[2].y;
			GotoXY(gate[4].x, gate[4].y);
			cout << "G";

			gateBehind.x = gate[0].x + 1;
			gateBehind.y = gate[0].y;
		}
	}
	else {
		for (int i = 0; i < 5; i++) {
			GotoXY(gate[i].x, gate[i].y);
			cout << "G";
		}
	}
}

//Hàm cập nhật dữ liệu toàn cục
void Eat() {
	PlaySound("sounds/eat.wav", NULL, SND_ASYNC);
	SCORE++;
	snake[SIZE_SNAKE] = food[FOOD_INDEX];
	if (FOOD_INDEX == MAX_SIZE_FOOD - 1) {
		DrawGate();											//4.4 Xử lý vẽ cổng khi ăn hết food ở 1 cấp
		FOOD_INDEX = 0;
		//SIZE_SNAKE = 6;									//4.3 Xử lý độ dài giữ nguyên khi lên cấp
		GenerateFood();
	}
	else
		FOOD_INDEX++;
	if (SIZE_SNAKE < 10)
		SIZE_SNAKE++;
}

//Hàm xử lý khi snake chết
void ProcessDead(int x) {
	PlaySound("sounds/hit.wav", NULL, SND_ASYNC);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);	//Vẽ chữ màu đỏ
	STATE = 0;
	GotoXY(20, HEIGH_CONSOLE + 2);
	if (x == 0)
		cout << "Dead because hit the wall!";
	else if (x == 1)
		cout << "Your Snake eats itself! @@";
	else
		cout << "Dead because hit the gate!";
	GotoXY(12, HEIGH_CONSOLE + 3);
	cout << "Press Y to play new game or anykey to exit.";
}

//4.1 Hàm xử lý khi snake đụng đuôi
bool StupidSnake(int x, int y) {
	for (int i = 0; i < SIZE_SNAKE - 1; i++)
		if (snake[i].x == x && snake[i].y == y)
			return true;
	return false;
}

//Hàm hiển thị điểm số
void DrawScore() {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);	//Vẽ thức ăn màu xanh lá cây
	GotoXY(WIDTH_CONSOLE + 5, HEIGH_CONSOLE - 20);
	cout << "HUNGRY SNAKE";
	GotoXY(WIDTH_CONSOLE + 5, HEIGH_CONSOLE - 19);
	cout << "LEVEL: " << SPEED;
	GotoXY(WIDTH_CONSOLE + 5, HEIGH_CONSOLE - 18);
	cout << "SCORE: " << SCORE;
}

void DrawSnakeAndFood(const char* str) {
	//Nếu đầu rắn đụng trúng đuôi
	if (StupidSnake(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y) == true)
		ProcessDead(1);									//Truyền thêm tham số để quăng thông báo, ProcessDead chỉ để set state về 0 và báo lý do
	for (int i = 0; i < SIZE_SNAKE; i++) {
		if(STATE == 0)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);	//Vẽ rắn màu đỏ
		else
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 13);	//Vẽ rắn màu tím
		GotoXY(snake[i].x, snake[i].y);
		nameIndex = i;									//Xử lý con rắn lặp MSSV
		if (nameIndex >= 7)
			nameIndex -= 7;
		if (str == "snake") {
			if (SIZE_SNAKE != 1)	//Nếu size_snake chỉ còn 1 thì không hiển thị, để tránh chồng lên cổng.
				cout << name[nameIndex];
		}
		else
			cout << " ";
		if (gateIsExist == true && FOOD_INDEX == 0 && SCORE != 0) {	//Nếu chưa qua cổng, qua cổng là đụng cổng hoặc qua thành công
			if (CheckGate() == true && gateIsExist == false)		//Nếu đã qua cổng
				LevelUp();
			else if (CheckGate() == false && gateIsExist == false) {	//Nếu như va phải cổng
				//lúc này đuôi rắn vẫn còn màu cũ do vòng lặp chỉ vẽ màu cho những phần sau
				//vậy nên phải vẽ lại đuôi rắn.
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
				char tail;
				tail = name[0];
				GotoXY(snake[0].x, snake[0].y);
				cout << tail;
				ProcessDead(2);
			}
		}
	}
	if (STATE == 1 && gateIsExist == false) {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);	//Vẽ thức ăn màu xanh lá cây
		GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
		cout << "0";
	}
	DrawScore();
}

void MoveRight() {
	if (snake[SIZE_SNAKE - 1].x + 1 == WIDTH_CONSOLE)
		ProcessDead(0);
	else {
		if (snake[SIZE_SNAKE - 1].x + 1 == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y)
			Eat();
		for (int i = 0; i < SIZE_SNAKE - 1; i++) {
			snake[i].x = snake[i + 1].x;
			snake[i].y = snake[i + 1].y;
		}
		snake[SIZE_SNAKE - 1].x++;
	}
}

void MoveLeft() {
	if (snake[SIZE_SNAKE - 1].x - 1 == 0)
		ProcessDead(0);
	else {
		if (snake[SIZE_SNAKE - 1].x - 1 == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y)
			Eat();
		for (int i = 0; i < SIZE_SNAKE - 1; i++) {
			snake[i].x = snake[i + 1].x;
			snake[i].y = snake[i + 1].y;
		}
		snake[SIZE_SNAKE - 1].x--;
	}
}

void MoveDown() {
	if (snake[SIZE_SNAKE - 1].y + 1 == HEIGH_CONSOLE)
		ProcessDead(0);
	else {
		if (snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y + 1 == food[FOOD_INDEX].y)
			Eat();
		for (int i = 0; i < SIZE_SNAKE - 1; i++) {
			snake[i].x = snake[i + 1].x;
			snake[i].y = snake[i + 1].y;
		}
		snake[SIZE_SNAKE - 1].y++;
	}
}

void MoveUp() {
	if (snake[SIZE_SNAKE - 1].y - 1 == 0)
		ProcessDead(0);
	else {
		if (snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y - 1 == food[FOOD_INDEX].y)
			Eat();
		for (int i = 0; i < SIZE_SNAKE - 1; i++) {
			snake[i].x = snake[i + 1].x;
			snake[i].y = snake[i + 1].y;
		}
		snake[SIZE_SNAKE - 1].y--;
	}
}

//Thủ tục cho thread
void ThreadFunc() {
	while (1) {
		if (STATE == 1) {	//Nếu vẫn còn snake vẫn còn sống
			DrawSnakeAndFood("space");
			switch (MOVING) {
			case 'A':
				MoveLeft();
				break;
			case 'D':
				MoveRight();
				break;
			case 'W':
				MoveUp();
				break;
			case 'S':
				MoveDown();
				break;
			}
			DrawSnakeAndFood("snake");
			Sleep(200 / SPEED);	//Hàm ngủ theo tốc độ SPEED
		}
	}
}
//
void ResizeConsole(int width, int height)
{
	HWND console = GetConsoleWindow();
	HWND screen = GetDesktopWindow();
	RECT rs;
	GetWindowRect(screen, &rs);
	MoveWindow(console, (rs.right - width)/2, (rs.bottom - height)/2 - 50, width, height, TRUE);
}

void DrawSnake() {
	cout << "\t\t    **********      ******           ***               ****               ***      ***      *************  " << endl;
	cout << "\t\t  **************    *******          ***              ******              ***     ***       *************  " << endl;
	cout << "\t\t ***          ***   ***  ***         ***             ***  ***             ***    ***        ***            " << endl;
	cout << "\t\t ***                ***   ***        ***            ***    ***            ***   ***         ***            " << endl;
	cout << "\t\t   ***              ***    ***       ***           ***      ***           ***  ***          *************  " << endl;
	cout << "\t\t      ***           ***     ***      ***          **************          ********          *************  " << endl;
	cout << "\t\t        ***         ***      ***     ***         ****************         *********         ***            " << endl;
	cout << "\t\t           ***      ***       ***    ***        ***            ***        ***    ***        ***            " << endl;
	cout << "\t\t             ***    ***        ***   ***       ***              ***       ***     ***       ***			" << endl;
	cout << "\t\t ***         ***    ***         ***  ***      ***                ***      ***      ***      ***			" << endl;
	cout << "\t\t  *************     ***          *******     ***                  ***     ***       ***     *************  " << endl;
	cout << "\t\t    *********       ***           ******    ***                    ***    ***        ***    *************  " << endl;
}

void NewGame(HANDLE handle_t1) {
	ResizeConsole(750, 500);
	int temp;
	StartGame();
	
	while (1) {
		temp = toupper(_getch());
		if (STATE == 1) {
			if (temp == 'P')
				PauseGame(handle_t1);
			else if (temp == 'O')
				SaveGame(handle_t1);
			else if (temp == 27) {
				ExitGame(handle_t1);
				return;
			}
			else {
				ResumeThread(handle_t1);
				if ((temp != CHAR_LOCK) && (temp == 'D' || temp == 'A' || temp == 'W' || temp == 'S')) {
					if (temp == 'D') CHAR_LOCK = 'A';
					else if (temp == 'W') CHAR_LOCK = 'S';
					else if (temp == 'S') CHAR_LOCK = 'W';
					else CHAR_LOCK = 'D';
					MOVING = temp;
				}
			}
		}
		else {
			if (temp == 'Y') StartGame();
			else {
				ExitGame(handle_t1);
				return;
			}
		}
	}
}

void DrawGuide(HANDLE handle_t1) {
	GotoXY(1, 17);
	for (int i = 0; i < 4; i++)
		cout << "\t\t\t\t\t\t\t             " << endl;
	GotoXY(1, 18);
	cout << "\t\t\t\t\t\t Use the W, A, S, D key to control the Snake." << endl;
	cout << "\t\t\t\t\t A to the left, D to the right, W to go up and S to go down." << endl;
	cout << "\t\t\t\t\t\t O key to save and P key to pause when playing." << endl << endl << endl << endl << endl;
	cout << "\t\t\t\t\t\t Press any key to start game...";
	if (_getch())
		NewGame(handle_t1);
}

void DrawMenu(HANDLE handle_t1) {
	{
		SetConsoleTitle("Snake Game");
		FixConsoleWindow();
		ResizeConsole(1150, 500);
		PlaySound("sounds/intro.wav", NULL, SND_ASYNC | SND_LOOP);
		for (int i = 1; i < 15; i++)
		{
			GotoXY(1, 3);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), i);
			DrawSnake();
			Sleep(150);
			if (i == 14) {
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 1);
				GotoXY(1, 3);
				DrawSnake();
				break;
			}
		}
		GotoXY(1, 17);
		char c;
		cout << "\t\t\t\t\t\t\t 1. NEW GAME" << endl;
		cout << "\t\t\t\t\t\t\t 2. LOAD GAME" << endl;
		cout << "\t\t\t\t\t\t\t 3. GUIDE" << endl;
		cout << "\t\t\t\t\t\t\t 4. EXIT" << endl;
		
		do {
			c = _getch();
			if (c == '1')
				NewGame(handle_t1);
			else if (c == '2')
				LoadGame(handle_t1);
			else if (c == '3')
				DrawGuide(handle_t1);
			else if (c == '4')
				ExitGame(handle_t1);
		} while (c != '1' && c != '2' && c != '3' && c != '4');
	}
}

void main() {
	thread t1(ThreadFunc);
	HANDLE handle_t1 = t1.native_handle();
	DrawMenu(handle_t1);
	//system("pause");
}