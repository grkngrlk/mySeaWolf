#include <windows.h>  
#include "chmat.h"
#include <thread>
#include <random>
#include <string>
#include <atlstr.h>

/*

Nişangahı sağa götürmek için sağ oka , sola götürmek için sol oka basılır.
Eğer nişangahı sabitlemek isterseniz alt tuşa basılır.
Boşluk tuşuyla torpido ateşlenir.

Oyun ekranında aynı anda en fazla 2 adet torpido olabilir.
Torpido hedefi bulmadan veya oyun alanından çıkmadan yenisi atılamaz

Oyunda aynı anda 6 adet gemi olabilir. Eğer gemi oyun alanından çıkarsa veya patlatılırsa yenisi üretilir(sayısı #define NUMBEROFSHIPS ile değiştirilebilir)
Tanker vurmak 100, Patrol gemisi vurmak 250, Battleship vurmak 500 puan kazandırır(ilgili defineşerle değiştirilebilir), hedef vurulunca torpido yok olur.

Oyunda aynı anda 6 mayın olabilir. Eğer patlatılırsa veya rastgele oranlarla silinip yinileri üretilir.(sayısı #define NUMBEROFMINES ile değiştirilebilir).
Mayın vurulursa puan kazanılmaz ve torpido yok olur.
Eğer vurulan mayının enkazı yok olmadan aynı yer vurulursa yine torpido boşa gider ve yok olur.

Torpido sayısı 30 la sınırlıdır hepsi tükenince oyun biter (istenirse  TOTALTORPEDOS define'si ile değiştirilebilir).
Oyun bittikten sonra restart tuşuyla yeniden oynanabilir.
Exit Tuşuyla ne zaman istenirse çıkılabilir.

Gürkan Gürlek
 */

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::default_random_engine generator(seed);
std::uniform_int_distribution<int> spawnFirst(15, 60); // Set the numbers for int.
std::uniform_int_distribution<int> spawnY(185, 280); // Set the numbers for int.
std::uniform_int_distribution<int> spawnType(0, 500);

using namespace std;

#define IDI_ICON1 101
#define IDC_OPEN	3000
#define IDC_BUTTON1 3001
#define IDC_BUTTON2 3002
#define IDC_BUTTON3 3003
#define randnum(min, max) ((rand() % (int) (((max) + 1) - (min))) + (min))
#define NUMBEROFMINES 6
#define NUMBEROFSHIPS 6

#define TOTALTORPEDOS 30
#define TANKERPOINT 100
#define PATROLPOINT 250
#define BSPOINT 500
#define LIMITS 10

#define destroyColor 0xe25822
#define mineColor 0xffff00
#define aimColor 0x14e91e
#define torpedoColor 0xffbd2e
#define patrolColor 0x72705d
#define tankerColor 0xff0000
#define waveColor 0x2c7b8a
#define battleShipColor 0xeae4ce


HINSTANCE hInst;   // current instance
HWND     hWnd, hEdit, HButton1, HButton2, HButton3, Hmainbmp, hTextScore, hEditScore, hTextTorpedo, hEditTorpedo, hEditGameOver;//parent window
LPCTSTR lpszAppName = "SEA WOLF";
LPCTSTR lpszTitle = "SEA WOLF 1976";
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


struct mines
{
	int id, x, y;
	bool onBoard;
};

//Type 1 = Tanker Type 2 = Patrol Type 3 = Battleship
struct ships
{
	int type, x, y, direction;

	bool onBoard;
};

struct mines mine[NUMBEROFMINES];
struct ships ship[NUMBEROFSHIPS];


VOID CALLBACK SlidingBox(PVOID lpParam, BOOLEAN TimerOrWaitFired);
void Shot();
void drawMine(int mineX, int mineY, int color);
void drawTorpedo(int torX, int torY, int color);
void drawAim(int aimX, int aimY, int color);
void drawDestroy(int desX, int desY, int color);
void drawTanker(int tank1X, int tank1Y, int color, int direction);
void drawPatrol(int patX, int patY, int color, int direction);
void drawWave(int waveX, int waveY, int color);
void drawBattleship(int bx, int by, int color, int direction);
void tailEraser(int desX, int desY);
int shipSpawn(int i);


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
	MSG      msg;
	WNDCLASSEX wc;
	HMENU MainMenu, FileMenu;
	MainMenu = CreateMenu();
	FileMenu = CreatePopupMenu();
	AppendMenu(FileMenu, MF_STRING, IDC_OPEN, "Open");
	AppendMenu(MainMenu, MF_POPUP, (UINT_PTR)FileMenu, "FILE");

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wc.lpszMenuName = lpszAppName;
	wc.lpszClassName = lpszAppName;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hIconSm = (HICON)LoadImage(hInstance, lpszAppName,
		IMAGE_ICON, 16, 16,
		LR_DEFAULTCOLOR);

	if (!RegisterClassEx(&wc))
		return(FALSE);

	hInst = hInstance;
	hWnd = CreateWindowEx(0, lpszAppName,
		lpszTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		450, 450,
		NULL,
		MainMenu,
		hInstance,
		NULL
	);

	if (!hWnd)
		return(FALSE);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

}

// !!!!!!!!!!!!!!!!!!!Global definition makes it easier!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
CHMAT m(418, 300, CHMAT_INT);

HANDLE hTimerQueue = NULL;
HANDLE hTimer = NULL;

int boxlocation = 210;
int lastKey = 0;
bool isActive = false;
bool isSpawnAvailable = true;
bool restart = false;

int nrOfActMines = 0;
int nrOfChanges = 0;
int nrOfShots = 0;
int nrOfActiveTorpedos = 0;

int totalTorpedosFired = 0;
int score = 0;
CString strScore;
int countertorpedo = TOTALTORPEDOS;
CString strtorpedo;
CString gameOver;

int keypressed;
bool isInExlodeTiming = false;
bool isInExlodeTimingShip = false;

bool shipBeenShot = false;
bool firstTime = true;
bool stopper = false;


//Thread called by the timer
VOID CALLBACK SlidingBox(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	strScore.Format(_T("%d"), score);
	SetWindowText(hEditScore, strScore);
	countertorpedo = TOTALTORPEDOS - totalTorpedosFired;
	strtorpedo.Format(_T("%d"), countertorpedo);
	SetWindowText(hEditTorpedo, strtorpedo);

	if (totalTorpedosFired == TOTALTORPEDOS && nrOfActiveTorpedos == 0)
	{

		//hEditGameOver = CreateWindow(TEXT("Static"), TEXT(" "), WS_VISIBLE | WS_CHILD, 150, 350, 30, 20, hWnd, NULL, NULL, NULL);
		gameOver.Format(_T("        Game Over"));
		SetWindowText(hEditGameOver, gameOver);

		isActive = false;
		stopper = false;
		m = 0;

		boxlocation = 210;
		lastKey = 0;
		isSpawnAvailable = true;

		nrOfActMines = 0;
		nrOfChanges = 0;
		nrOfShots = 0;
		nrOfActiveTorpedos = 0;

		totalTorpedosFired = 0;
		score = 0;
		countertorpedo = TOTALTORPEDOS;

		isInExlodeTiming = false;
		isInExlodeTimingShip = false;

		shipBeenShot = false;
		firstTime = true;
		restart = true;
		DeleteTimerQueue(hTimerQueue);

	}


	//10 Genişlikte 6 yükseklikte kutu çiziyor
	//m matrisinin boxlocation + i , y = 100 noktasına 0 = siyah koyuyor(kuyruk silici döngü)
	//boxlocation kutunun x'i ni tutuyor
	Sleep(10);
	srand((unsigned int)time(NULL));
	//randnum(555, 780);
	tailEraser(boxlocation, 290);

	for (int i = 4; i <= m.X; i = i + 7) {
		drawWave(i, 178, waveColor);
	}

	if (nrOfActMines < NUMBEROFMINES && !restart) {

		for (int i = 0; i < NUMBEROFMINES; i++) {

			if (!mine[i].onBoard && !isInExlodeTiming) {

				mine[i].x = randnum(30, 380);
				mine[i].y = randnum(10, 170);
				drawMine(mine[i].x, mine[i].y, mineColor);
				mine[i].onBoard = true;
				nrOfActMines++;

			}

		}

	}

	if (!firstTime) {

		for (size_t i = 0; i < NUMBEROFSHIPS; i++)
		{
			if (ship[i].onBoard && ship[i].type == 1)
			{
				tailEraser(ship[i].x, ship[i].y);
				Sleep(1);
			}
			if (ship[i].onBoard && ship[i].type == 2)
			{
				tailEraser(ship[i].x, ship[i].y);
				Sleep(1);
			}

			if (ship[i].onBoard && ship[i].type == 3)
			{
				tailEraser(ship[i].x, ship[i].y);
				Sleep(1);
			}
		}
	}

	SendMessage(Hmainbmp, STM_SETIMAGE, 0, (LPARAM)m.HBitmap);



	if (keypressed == 37 && boxlocation > 10) {
		lastKey = 37;
		boxlocation -= 2;
	}
	// left
	if (keypressed == 39 && boxlocation < m.X - 11) {
		lastKey = 39;
		boxlocation += 2;
	}
	// right
	if (keypressed == 40) {
		lastKey = 40;
		boxlocation += 0;
	}
	// down


	if (nrOfShots == 3 && nrOfActiveTorpedos < 2 && !isInExlodeTiming || restart) {

		for (int i = 0; i < NUMBEROFMINES; i++) {

			drawMine(mine[i].x, mine[i].y, 0);
			mine[i].x = randnum(30, 380);
			mine[i].y = randnum(10, 170);
			drawMine(mine[i].x, mine[i].y, mineColor);
			mine[i].onBoard = true;
			nrOfActMines++;

		}
		nrOfShots = 0;
		restart = false;
	}

	if (!isInExlodeTiming && randnum(0, 100) % 6 == 0) {

		for (int i = 0; i < NUMBEROFMINES; i++) {

			drawMine(mine[i].x, mine[i].y, 0);
			mine[i].x = randnum(30, 380);
			mine[i].y = randnum(10, 170);
			drawMine(mine[i].x, mine[i].y, mineColor);
			mine[i].onBoard = true;
			nrOfActMines++;

		}
		nrOfShots = 0;
	}

	if (firstTime) {

		for (int i = 0; i < NUMBEROFSHIPS; i++) {

			int random = spawnType(generator) % 3;

			if (random == 0) {

				ship[i].y = randnum(205, 280);
				ship[i].onBoard = true;
				ship[i].type = 1;

				if (randnum(0, 10) % 2 == 0) {
					ship[i].direction = 1;
					ship[i].x = spawnFirst(generator);
				}
				else {
					ship[i].direction = -1;
					ship[i].x = m.X - spawnFirst(generator);
				}

				if (i == 0 || i == 2) {
					ship[i].x += randnum(0, 5);
				}
				drawTanker(ship[i].x, ship[i].y, tankerColor, ship[i].direction * -1);

			}

			else if (random == 1)
			{
				//ship[i].x = randnum(30, 380);
				ship[i].y = randnum(205, 280);
				ship[i].onBoard = true;
				ship[i].type = 2;

				if (randnum(0, 10) % 2 == 0) {
					ship[i].direction = 1;
					ship[i].x = spawnFirst(generator);
				}

				else {
					ship[i].direction = -1;
					ship[i].x = m.X - spawnFirst(generator);
				}

				drawPatrol(ship[i].x, ship[i].y, patrolColor, ship[i].direction * -1);

			}

			else
			{
				//ship[i].x = randnum(30, 380);
				ship[i].y = randnum(205, 280);
				ship[i].onBoard = true;
				ship[i].type = 3;

				if (randnum(0, 10) % 2 == 0) {
					ship[i].direction = 1;
					ship[i].x = spawnFirst(generator);
				}

				else {
					ship[i].direction = -1;
					ship[i].x = m.X - spawnFirst(generator);
				}

				drawBattleship(ship[i].x, ship[i].y, battleShipColor, ship[i].direction * -1);
			}
		}
		firstTime = false;
	}

	else {

		for (int i = 0; i < NUMBEROFSHIPS; i++)
		{

			if (ship[i].type == 1 && ship[i].onBoard)
			{

				if (ship[i].x + 2 > m.X - 5 || ship[i].x - 2 < 5)
				{
					ship[i].onBoard = false;
					drawTanker(ship[i].x, ship[i].y, 0, ship[i].direction * -1);
				}

				if (ship[i].direction == 1 && ship[i].x < m.X - 5 && ship[i].onBoard)
				{
					ship[i].x += 1;
					/*drawTanker(ship[i].x + 3, ship[i].y, 0, ship[i].direction * -1);*/
					tailEraser(ship[i].x, ship[i].y);
					drawTanker(ship[i].x, ship[i].y, tankerColor, ship[i].direction * -1);
				}
				if (ship[i].direction == -1 && ship[i].x > 5 && ship[i].onBoard)
				{
					ship[i].x -= 1;
					/*drawTanker(ship[i].x - 3, ship[i].y, 0, ship[i].direction * -1);*/
					tailEraser(ship[i].x, ship[i].y);
					drawTanker(ship[i].x, ship[i].y, tankerColor, ship[i].direction * -1);
				}

			}

			else if (ship[i].type == 2 && ship[i].onBoard)
			{

				if (ship[i].x + 2 > m.X - 5 || ship[i].x - 2 < 5)
				{
					ship[i].onBoard = false;
					drawPatrol(ship[i].x, ship[i].y, 0, ship[i].direction * -1);
				}

				if (ship[i].direction == 1 && ship[i].x < m.X - 5 && ship[i].onBoard)
				{
					ship[i].x += 2;
					/*drawPatrol(ship[i].x - 2, ship[i].y, 0, ship[i].direction * -1);
					drawPatrol(ship[i].x - 3, ship[i].y, 0, ship[i].direction * -1);*/
					tailEraser(ship[i].x, ship[i].y);
					drawPatrol(ship[i].x, ship[i].y, patrolColor, ship[i].direction * -1);
				}

				if (ship[i].direction == -1 && ship[i].x > 5 && ship[i].onBoard)
				{
					ship[i].x -= 2;
					/*drawPatrol(ship[i].x + 2, ship[i].y, 0, ship[i].direction * -1);
					drawPatrol(ship[i].x + 3, ship[i].y, 0, ship[i].direction * -1);*/
					tailEraser(ship[i].x, ship[i].y);
					drawPatrol(ship[i].x, ship[i].y, patrolColor, ship[i].direction * -1);
				}

			}

			else if (ship[i].type == 3 && ship[i].onBoard)
			{

				if (ship[i].x + 2 > m.X - 5 || ship[i].x - 2 < 5)
				{
					//drawBattleship(ship[i].x, ship[i].y, battleShipColor, ship[i].direction * -1);
					ship[i].onBoard = false;
					drawBattleship(ship[i].x, ship[i].y, 0, ship[i].direction * -1);
				}

				if (ship[i].direction == 1 && ship[i].x < m.X - 5 && ship[i].onBoard)
				{
					ship[i].x += 2;
					/*drawBattleship(ship[i].x + 2, ship[i].y, 0, ship[i].direction * -1);
					drawBattleship(ship[i].x + 3, ship[i].y, 0, ship[i].direction * -1);
					drawBattleship(ship[i].x, ship[i].y+1, 0, ship[i].direction * -1);*/
					tailEraser(ship[i].x, ship[i].y);
					drawBattleship(ship[i].x, ship[i].y, battleShipColor, ship[i].direction * -1);
				}

				if (ship[i].direction == -1 && ship[i].x > 5 && ship[i].onBoard)
				{
					ship[i].x -= 2;
					/*drawBattleship(ship[i].x - 2, ship[i].y, 0, ship[i].direction * -1);
					drawBattleship(ship[i].x - 3, ship[i].y, 0, ship[i].direction * -1);
					drawBattleship(ship[i].x, ship[i].y + 1, 0, ship[i].direction * -1);
					drawBattleship(ship[i].x, ship[i].y + 2, 0, ship[i].direction * -1);*/
					tailEraser(ship[i].x, ship[i].y);
					drawBattleship(ship[i].x, ship[i].y, battleShipColor, ship[i].direction * -1);
				}

			}

			if (!ship[i].onBoard)
			{

				if (isSpawnAvailable && !isInExlodeTimingShip)
				{
					isSpawnAvailable = false;
					int a = shipSpawn(i);
				}

			}

		}

	}

	//It waits for space key and opens a thread
	//Sometimes it not erasing tail of the sliding box i don't know why
	if (keypressed == 32 && nrOfActiveTorpedos < 2 && totalTorpedosFired < TOTALTORPEDOS) {
		keypressed = lastKey;
		//opening thread with default location values
		//isInExlodeTiming = false;
		totalTorpedosFired++;
		nrOfShots++;
		thread laserShot(Shot);
		laserShot.join();

	}

	keypressed = lastKey;

	//10 x 6 kutu oluşturuyor
	//m(x, y) = kırmızı basıyor
	drawAim(boxlocation, 290, aimColor);

	SendMessage(Hmainbmp, STM_SETIMAGE, 0, (LPARAM)m.HBitmap);
}

//spawns Ship random location 
int shipSpawn(int i) {

	int random = spawnType(generator) % 3;

	if (random == 0) {

		ship[i].y = spawnY(generator);
		ship[i].onBoard = true;
		ship[i].type = 1;

		if (randnum(0, 10) % 2 == 0) {
			ship[i].direction = 1;
			ship[i].x = spawnFirst(generator);
		}
		else {
			ship[i].direction = -1;
			ship[i].x = m.X - spawnFirst(generator);
		}
		drawTanker(ship[i].x, ship[i].y, tankerColor, ship[i].direction * -1);

	}

	else if (random == 1)
	{
		//ship[i].x = randnum(30, 380);
		ship[i].y = spawnY(generator);
		ship[i].onBoard = true;
		ship[i].type = 2;

		if (randnum(0, 10) % 2 == 0) {
			ship[i].direction = 1;
			ship[i].x = spawnFirst(generator);
		}

		else {
			ship[i].direction = -1;
			ship[i].x = m.X - spawnFirst(generator);
		}

		drawPatrol(ship[i].x, ship[i].y, patrolColor, ship[i].direction * -1);

	}

	else
	{
		//ship[i].x = randnum(30, 380);
		ship[i].y = spawnY(generator);
		ship[i].onBoard = true;
		ship[i].type = 3;

		if (randnum(0, 10) % 2 == 0) {
			ship[i].direction = 1;
			ship[i].x = spawnFirst(generator);
		}

		else {
			ship[i].direction = -1;
			ship[i].x = m.X - spawnFirst(generator);
		}

		drawBattleship(ship[i].x, ship[i].y, battleShipColor, ship[i].direction * -1);
	}

	isSpawnAvailable = true;
	/*for (size_t i = 0; i < 3; i++)
	{
		if (ship[i].direction == -1 && !ship[i].onBoard) ship[i].x += randnum(0, 100);
		if (ship[i].direction == -1 && !ship[i].onBoard) ship[i].x -= randnum(0, 100);
	}*/
	firstTime = false;
	return 1;
}

//for shoting torpedo and tracking
void Shot()
{
	nrOfActiveTorpedos++;
	int laserLocationY = 15;
	int laserLocationX = boxlocation + 4;
	int laserLocationYEraser = laserLocationY;
	bool stopLaser = false;
	bool isInexplodeTimerInside = false;

	while (laserLocationY - 12 < m.Y) {
		//I add sleep for visibility
		Sleep(10);

		/*its is my tail eraser, it prints black to my laser's last 10 element for hiding it,
		if it get close more than 10 pixels to actual laser it stops thus it's not deleting laser*/
		while (laserLocationYEraser < laserLocationY + 7)
		{
			//if it comes to border of m matrix it stops there for preventing overflow errors otherwise it continues
			if (laserLocationYEraser + 1 < m.Y) laserLocationYEraser++;
			else break;
			drawTorpedo(laserLocationX + 1, laserLocationYEraser - 5, 0);
			SendMessage(Hmainbmp, STM_SETIMAGE, 0, (LPARAM)m.HBitmap);
		}

		if (stopLaser) {

			break;
		}

		laserLocationY += 3;
		//my printer loop
		//it prints a laser with 2 width and 5 length with yellow color
		for (int i = 0; i < 5; i++)
		{
			//Stops if cross to border of the m matris
			if (laserLocationY + 10 + i < m.Y) {
				drawTorpedo(laserLocationX + 1, laserLocationY + i, torpedoColor);
			}
			else break;

		}

		for (int i = 0; i < NUMBEROFMINES; i++)
		{
			//Checking laser for impact mines
			if (mine[i].y < laserLocationY + LIMITS && mine[i].y > laserLocationY - LIMITS && mine[i].x > laserLocationX - LIMITS && mine[i].x < laserLocationX + LIMITS && mine[i].onBoard) {

				drawMine(mine[i].x, mine[i].y, 0);
				drawDestroy(mine[i].x, mine[i].y, destroyColor);
				drawTorpedo(laserLocationX + 1, laserLocationY, 0);
				drawTorpedo(laserLocationX + 1, laserLocationY + 4, 0);
				drawTorpedo(laserLocationX + 1, laserLocationY - 5, 0);

				//This is for semaphore, if this is true no new mine will be spawn
				isInExlodeTiming = true;

				Sleep(1000);

				int a = mine[i].x;
				int b = mine[i].y;
				drawDestroy(a, b, 0);

				mine[i].onBoard = false;
				nrOfActMines--;
				m(laserLocationX, laserLocationY + 1) = 0;
				m(laserLocationX + 1, laserLocationY + 1) = 0;
				m(laserLocationX - 1, laserLocationY + 1) = 0;
				//SendMessage(Hmainbmp, STM_SETIMAGE, 0, (LPARAM)m.HBitmap);
				isInExlodeTiming = false;
				stopLaser = true;
			}

		}

		for (int i = 0; i < NUMBEROFSHIPS; i++)
		{
			//Checking laser for impact ships
			if (ship[i].y < laserLocationY + LIMITS && ship[i].y > laserLocationY - LIMITS && ship[i].x > laserLocationX - LIMITS && ship[i].x < laserLocationX + LIMITS && ship[i].onBoard) {

				//This is for semaphore, if this is true no new ship will be spawn
				isInExlodeTimingShip = true;
				int a, b;
				if (ship[i].type == 1) {
					score += TANKERPOINT;
					ship[i].onBoard = false;
					drawTanker(ship[i].x, ship[i].y, 0, ship[i].direction * -1);
					a = ship[i].x;
					b = ship[i].y;
				}
				else if (ship[i].type == 2)
				{
					ship[i].onBoard = false;
					score += PATROLPOINT;
					drawPatrol(ship[i].x, ship[i].y, 0, ship[i].direction * -1);
					a = ship[i].x;
					b = ship[i].y;
				}

				else {
					ship[i].onBoard = false;
					score += BSPOINT;
					drawBattleship(ship[i].x, ship[i].y, 0, ship[i].direction * -1);
					a = ship[i].x;
					b = ship[i].y;
				}
				drawDestroy(ship[i].x, ship[i].y, destroyColor);
				drawTorpedo(laserLocationX + 1, laserLocationY, 0);
				drawTorpedo(laserLocationX + 1, laserLocationY + 4, 0);
				drawTorpedo(laserLocationX + 1, laserLocationY - 5, 0);

				//This is for semaphore, if this is true no ship will be spawn

				Sleep(1000);

				drawDestroy(a, b, 0);

				nrOfActMines--;
				m(laserLocationX, laserLocationY + 1) = 0;
				m(laserLocationX + 1, laserLocationY + 1) = 0;
				m(laserLocationX - 1, laserLocationY + 1) = 0;
				//SendMessage(Hmainbmp, STM_SETIMAGE, 0, (LPARAM)m.HBitmap);
				isInExlodeTimingShip = false;
				stopLaser = true;
			}

		}

		SendMessage(Hmainbmp, STM_SETIMAGE, 0, (LPARAM)m.HBitmap);
	}
	nrOfActiveTorpedos--;
}

void drawWave(int waveX, int waveY, int color) {
	m(waveX - 3, waveY) = color;
	m(waveX - 2, waveY) = color;
	m(waveX - 1, waveY + 1) = color;
	m(waveX, waveY) = color;
	m(waveX + 1, waveY + 1) = color;
	m(waveX + 2, waveY + 1) = color;
	m(waveX + 3, waveY) = color;
}

//belirlenen x y noktasına aim'i çizer. bgColor'u animate ederken arkasında kalan kısım için düşünüldü. hareket özelliği yok henüz.
void drawAim(int aimX, int aimY, int color) {
	//input coordinates are center of aim. area of aim block is 7*10 pixels.
	for (int i = -5; i <= 5; i++) {
		m(aimX - 7, aimY + i) = color;
	}

	m(aimX - 7, aimY - 5) = color;
	m(aimX - 6, aimY - 5) = color;
	m(aimX - 5, aimY - 5) = color;
	m(aimX - 4, aimY - 5) = color;
	m(aimX - 4, aimY + 5) = color;
	m(aimX - 5, aimY + 5) = color;
	m(aimX - 6, aimY + 5) = color;
	m(aimX - 7, aimY + 5) = color;

	for (int i = -5; i <= 5; i++) {
		m(aimX + 7, aimY + i) = color;
	}

	m(aimX + 7, aimY - 5) = color;
	m(aimX + 6, aimY - 5) = color;
	m(aimX + 5, aimY - 5) = color;
	m(aimX + 4, aimY - 5) = color;
	m(aimX + 4, aimY + 5) = color;
	m(aimX + 5, aimY + 5) = color;
	m(aimX + 6, aimY + 5) = color;
	m(aimX + 7, aimY + 5) = color;
	m(aimX, aimY) = color;
}

void drawMine(int mineX, int mineY, int color) {
	int i;

	for (i = -3; i <= 3; i++) {
		m(mineX + i, mineY) = color;
	}
	for (i = -2; i <= 2; i++) {
		m(mineX + i, mineY + 1) = color;
		m(mineX + i, mineY - 1) = color;
	}
	for (i = -1; i <= 1; i++) {
		m(mineX + i, mineY + 2) = color;
		m(mineX + i, mineY - 2) = color;
	}
	m(mineX, mineY + 3) = color;
	m(mineX, mineY - 3) = color;

	for (i = 0; i <= 3; i++) {
		m(mineX + 3 + i, mineY + i) = color;
		m(mineX + 3 + i, mineY - i) = color;
		m(mineX - 3 - i, mineY + i) = color;
		m(mineX - 3 - i, mineY - i) = color;
		m(mineX - i, mineY - 5 - i) = color;
	}

	m(mineX, mineY - 4) = color;
	m(mineX, mineY) = color;
	m(mineX, mineY) = color;
	m(mineX, mineY) = color;
	m(mineX, mineY) = color;
	m(mineX, mineY) = color;
}

void drawTorpedo(int torX, int torY, int color) {
	m(torX, torY + 3) = color;
	for (int i = -2; i <= 2; i++) {
		for (int j = -1; j <= 1; j++) {
			m(torX + j, torY + i) = color;
		}
	}
	m(torX, torY - 3) = color;
	m(torX, torY - 4) = color;
	m(torX - 1, torY - 4) = color;
	m(torX + 1, torY - 4) = color;
}

void drawPatrol(int patX, int patY, int color, int direction) {
	int i;
	for (i = -2; i <= 1; i++) {
		m(patX + i * direction, patY + 2) = color;
	}
	for (i = -3; i <= 2; i++) {
		m(patX + i * direction, patY + 1) = color;
	}
	for (i = -7; i <= 4; i++) {
		m(patX + i * direction, patY) = color;
	}
	for (i = -6; i <= 8; i++) {
		m(patX + i * direction, patY - 1) = color;
	}
	for (i = -5; i <= 8; i++) {
		m(patX + i * direction, patY - 2) = color;
	}
}

void drawTanker(int tank1X, int tank1Y, int color, int direction) {
	int i;
	for (i = -5; i <= 8; i++) {
		m(tank1X + i * direction, tank1Y - 1) = color;
	}
	for (i = -4; i <= 8; i++) {
		m(tank1X + i * direction, tank1Y - 2) = color;
	}
	for (i = -3; i <= 8; i++) {
		m(tank1X + i * direction, tank1Y - 3) = color;
	}
	for (i = -7; i <= -4; i++) {
		m(tank1X + i * direction, tank1Y) = color;
	}
	for (i = 0; i <= 4; i++) {
		m(tank1X - 4 * direction, tank1Y + i) = color;
	}
	for (i = 0; i <= 4; i++) {
		m(tank1X + 4 * direction, tank1Y + i) = color;
	}

	for (i = 6; i <= 7; i++) {
		m(tank1X + i * direction, tank1Y + 2) = color;
	}
	for (i = 6; i <= 7; i++) {
		m(tank1X + i * direction, tank1Y + 1) = color;
	}
	for (i = 6; i <= 9; i++) {
		m(tank1X + i * direction, tank1Y) = color;
	}
	//m(tank1X, tank1Y) = color;
}

void drawBattleship(int bx, int by, int color, int direction) {
	int i;
	for (i = -7; i <= 7; i++) {
		m(bx + i * direction, by) = color;
	}
	for (i = -6; i <= 7; i++) {
		m(bx + i * direction, by - 1) = color;
	}
	for (i = -5; i <= 6; i++) {
		m(bx + i * direction, by - 2) = color;
	}
	for (i = -4; i <= 5; i++) {
		m(bx + i * direction, by - 3) = color;
	}
	m(bx - 5 * direction, by + 1) = color;
	m(bx - 4 * direction, by + 1) = color;
	m(bx - 2 * direction, by + 1) = color;
	m(bx - 1 * direction, by + 1) = color;
	m(bx, by + 1) = color;
	m(bx + 2 * direction, by + 1) = color;
	m(bx + 4 * direction, by + 1) = color;
	m(bx + 6 * direction, by + 1) = color;
	m(bx - 6 * direction, by + 2) = color;
	m(bx - 5 * direction, by + 2) = color;
	m(bx - 2 * direction, by + 2) = color;
	m(bx - 1 * direction, by + 2) = color;
	m(bx - 2 * direction, by + 3) = color;
}

void drawDestroy(int desX, int desY, int color) {
	int i;
	for (i = -4; i <= 4; i++) {
		m(desX + i, desY - 3) = color;
	}

	for (i = -5; i <= 5; i++) {
		m(desX + i, desY - 2) = color;
	}
	for (i = -6; i <= 6; i++) {
		m(desX + i, desY - 1) = color;
	}

	m(desX - 5, desY) = color;
	m(desX - 3, desY) = color;
	m(desX - 2, desY) = color;
	m(desX + 1, desY) = color;
	m(desX + 3, desY) = color;
	m(desX + 5, desY) = color;

	m(desX - 7, desY + 1) = color;
	m(desX - 6, desY + 1) = color;
	m(desX - 3, desY + 1) = color;
	m(desX + 1, desY + 1) = color;
	m(desX + 3, desY + 1) = color;
	m(desX + 6, desY + 1) = color;
	m(desX + 7, desY + 1) = color;

	m(desX - 4, desY + 2) = color;
	m(desX + 1, desY + 2) = color;
	m(desX + 4, desY + 2) = color;
	m(desX, desY + 3) = color;
}

void tailEraser(int desX, int desY) {
	int i;
	for (i = -11; i <= 11; i++) {
		m(desX + i, desY + 5) = 0;
		m(desX + i, desY + 4) = 0;
		m(desX + i, desY + 3) = 0;
		m(desX + i, desY + 2) = 0;
		m(desX + i, desY + 1) = 0;
		m(desX + i, desY) = 0;
		m(desX + i, desY - 1) = 0;
		m(desX + i, desY - 2) = 0;
		m(desX + i, desY - 3) = 0;
		m(desX + i, desY - 4) = 0;
		m(desX + i, desY - 5) = 0;
	}

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{

	case WM_CREATE:
	{
		HButton1 = CreateWindowEx(NULL, "BUTTON", "START/RESTART", WS_CHILD | WS_VISIBLE | SS_CENTER,
			2, 2, 150, 20, hWnd, (HMENU)IDC_BUTTON1, hInst, NULL);
		HButton2 = CreateWindowEx(NULL, "BUTTON", "EXIT", WS_CHILD | WS_VISIBLE | SS_CENTER,
			152, 2, 80, 20, hWnd, (HMENU)IDC_BUTTON2, hInst, NULL);
		Hmainbmp = CreateWindowEx(NULL, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_BITMAP | WS_THICKFRAME, 1, 23, 600, 500, hWnd, NULL, hInst, NULL);
		hTextScore = CreateWindow(TEXT("Static"), TEXT("Score:"), WS_VISIBLE | WS_CHILD, 15, 350, 50, 20, hWnd, NULL, NULL, NULL);
		hTextTorpedo = CreateWindow(TEXT("Static"), TEXT("Torpedo Left:"), WS_VISIBLE | WS_CHILD, 300, 350, 100, 20, hWnd, NULL, NULL, NULL);
		hEditScore = CreateWindow(TEXT("Static"), TEXT(strScore), WS_VISIBLE | WS_CHILD, 85, 350, 40, 20, hWnd, NULL, NULL, NULL);
		hEditTorpedo = CreateWindow(TEXT("Static"), TEXT(strScore), WS_VISIBLE | WS_CHILD, 390, 350, 30, 20, hWnd, NULL, NULL, NULL);

		hEditGameOver = CreateWindow(TEXT("Static"), TEXT(" "), WS_VISIBLE | WS_CHILD, 130, 350, 150, 20, hWnd, NULL, NULL, NULL);


		strtorpedo.Format(_T("%d"), countertorpedo);
		SetWindowText(hEditTorpedo, strtorpedo);
		strScore.Format(_T("%d"), score);
		SetWindowText(hEditScore, strScore);
		//int countertorpedo = totaltorpedos;
		//cstring strtorpedo;
	}
	break;
	case WM_KEYDOWN:
	{
		keypressed = (int)wParam;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case IDC_OPEN:
		{
			OPENFILENAME ofn;

			char szFile[260];
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFile = szFile;
			//
			// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
			// use the contents of szFile to initialize itself.
			//
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			GetOpenFileName(&ofn);
		}
		break;
		case IDC_BUTTON1:
		{

			//if (!isActive) {
			//	hTimerQueue = CreateTimerQueue();
			//}

			//if (NULL == hTimerQueue)
			//{
			//	break;
			//}
			//starts 1000 ms later calls thread every 30 ms

			if (!isActive) {
				hTimerQueue = CreateTimerQueue();
				CreateTimerQueueTimer(&hTimer, hTimerQueue, (WAITORTIMERCALLBACK)SlidingBox, NULL, 1000, 15, 0);
				isActive = true;
				gameOver.Format(_T("      Game Started"));
				SetWindowText(hEditGameOver, gameOver);
				SetFocus(hWnd);
			}

		}
		break;

		case IDC_BUTTON2:
		{
			if (isActive) {
				SetFocus(hWnd);
				DeleteTimerQueue(hTimerQueue);
				exit(0);
			}
			else exit(0);
		}
		break;

		default:
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	}

	return(0L);
}
