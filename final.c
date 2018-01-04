#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/SimpleTextInEx.h>
#include <Protocol/SimpleFileSystem.h>
#include <guid/FileInfo.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void constructGameFile(); //Ϊ���ǵ���Ϸ��Դ�ļ���ӵ�ģ�����������ļ�ϵͳ��
void showWelcome();  //��ʾ��ӭ����
void showMenu();     //��ʾ�˵�����
void showGame();     //����Ϸ������ʾ����, ����һ����Ϸ����ʱ,ֻ�����һ��
void showTips();    //����Ϸ�Ҳ���ʾһЩ˵��
void showSteps();   //��ʾ����
void showMap();  
void showWin();     //��ʾͨ�صĽ���
void showSelectLevel();  //��ʾѡ�صĽ���

void loadMap();    //��ȡ��Ϸ�ؿ���ͼ�ļ���Ϣ����ά������
void exitGame();   
void updateKey();  //��Ӧ���̰����¼�, ������Ϸ״̬λ

void render();     //������Ϸ�İ�������Ӧ��״̬ ������һ������
void init();       //��ʼ����Ϸ״̬λ
void clearScreen();//����
void selectLevel();//ѡ��
void playGame();   //��������
void initMap();    //�õ�����ĳ�ʼλ�� �� ���� Ŀ�ĵص� λ����Ϣ
void basicShow(char *str) { gST->ConOut->OutputString(gST->ConOut, str); }
void getFileName(int level, char* buf); 

void drawMan();    //��������
void drawDestination(); //��������Ŀ�ĵ�
void drawBox();    //��������
void drawFloor();  
void drawLine(int start_x, int start_y, int end_x, int end_y, int width);

void drawRectangle(int start_x, int start_y, int width, int length   //���Ƶ�ɫ����
	,EFI_GRAPHICS_OUTPUT_BLT_PIXEL* BltBuffer) {

	EFI_GRAPHICS_OUTPUT_PROTOCOL *gGraphicsOutput;
	EFI_STATUS _status;
	_status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,
		NULL, (VOID**)&gGraphicsOutput);
	if (EFI_ERROR(_status)) {
		Print(L"draw Fail!!!");
	}

	_status = gGraphicsOutput->Blt(gGraphicsOutput,
		BltBuffer,
		EfiBltVideoFill,
		0, 0,
	    start_x, start_y,
		width, length,
		0);

}

void copyRectangle(int s_x, int s_y,  //������Ļ����(����)��ָ����λ��
	int e_x, int e_y,int width, int length) {
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gGraphicsOutput;
	EFI_STATUS _status;
	_status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,
		NULL, (VOID**)&gGraphicsOutput);
	_status = gGraphicsOutput->Blt(gGraphicsOutput,
		0,
		EfiBltVideoToVideo,
		s_x, s_y,
		e_x, e_y,
		width, length,
		0);
}



typedef enum _game_status{ 
	WELCOME, MENU, SELECT_LEVEL, GAMING, PAUSE, EXIT, BACK, WIN
}Game_Status;                    //��Ϸ״̬

typedef enum _directory {
	UP, DOWN, LEFT, RIGHT
}Directions;                      //������ƶ�����


typedef struct {
	int x;
	int y;
	char text[3];
}Node;



typedef struct {
	int x;
	int y;
}Direction;                      //�����ƶ��������ϸ��Ϣ


static int isSelectLevel = 0;   //�Ƿ���ѡ��
static int currentLevel = 1;    //��ǰ�ǵڼ���
static int isRefresh = 0;       //�ж��Ƿ����render()����
static Game_Status game_status;//��Ϸ��״̬��Ϣ
static Directions Move_Directions;//��ǰ���ƶ�����
static int Steps = 0;           //������Ϸ�Ĳ���
static int cols = 0;            //��Ϸ��ͼ������
static int rows = 0;            //��Ϸ��ͼ������
static int boxNum = 0;          //���ص���������
static Node MAN;                //�����ҵ�λ����Ϣ
static int X_Pos[10] = { 0 };   //�洢����Ŀ�ĵص�λ����Ϣ(x������)
static int Y_Pos[10] = { 0 };   //ͬ��, ��y������
static const char BAR = '#';    //ǽ
static const char DESTINATION = 'X'; //����Ŀ�ĵ�
static const char BOX = 'O';    //����
static const char MERGE = 'Q';  //�����Ƶ�Ŀ�ĵ�
static const char EMPTY = ' ';   //��λ��
static const char MAN_char = '@'; //���


// ���涨����һЩ��ɫ��������Ϣ
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL BLACK[1] = { 0,0,0,0 };
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL SKIN[1] = { 216,229,247,0 };
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL WHITE[1] = { 255,255,255,0 };
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL LIGHT_BROWN[1] = { 47,81,175,0 };
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL DARK_BROWN[1] = { 35,40,147,0 };
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL LIGHT_BLUE[1] = { 229,183,159,0 };
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL DARK_BLUE[1] = { 161,128,44,0 };
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL LIGHT_RED[1] = { 108,127,247,0 };
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL PINK[1] = { 201,181,255,0 };
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL RED[1] = { 0,0,255,0 };
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL YELLOW[1] = {0,255,255,0 };



static char MAP[50][50]; //�洢��ǰ�ؿ��ĵ�ͼ��Ϣ


static UINT8 HU[23][21] = {
	{0,0,0,0,0,0,0,0,1,1,1,1,1},
	{0,0,0,0,0,0,1,1,1,0,0,0,1,1,1},
	{0,0,0,0,0,1,1,0,0,0,1,0,0,0,1,1},
	{0,0,0,0,1,1,0,0,1,1,1,1,1,0,0,1,1},
	{0,0,1,1,1,0,0,1,0,0,0,0,0,1,0,0,1,1,1},
	{1,1,1,0,0,0,1,0,0,1,1,1,0,0,1,0,0,0,1,1,1},
	{1,1,1,1,1,1,0,0,1,1,0,0,1,0,0,1,1,1,1,1,1},
	{0,0,0,0,0,0,0,1,1,0,1,0,1,1},
	{1,1,1,1,1,1,0,0,1,0,0,1,1,0,0,1,1,1,1,1,1},
	{1,1,1,0,0,0,1,0,0,1,1,1,0,0,1,0,0,0,1,1,1},
	{0,0,1,1,1,0,0,1,0,0,0,0,0,1,0,0,1,1,1},
	{0,0,0,0,1,1,0,0,1,1,1,1,1,0,0,1,1},
	{0,0,0,0,0,1,1,0,0,0,1,0,0,0,1,1},
	{0,0,0,0,0,0,1,1,1,0,0,0,1,1,1},
	{0,0,0,0,0,0,0,0,1,1,1,1,1},
	{0,0,0,0,0,0,0,0,0,1,0,1},
	{0,0,0,0,0,0,0,0,0,1,0,1},
	{0,0,0,0,0,0,0,0,0,1,0,1,0,0,1,1},
	{0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,1},
	{0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,1},
	{0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1},
	{0,0,0,0,1,1,0,0,1,0,0,0,1,1,1,0,0,1,1},
	{0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1}
};


static UINT8 MAN_PIC[20][13] = {   //�����ͼ����Ϣ
	{0,0,0,1,1,1,1,1,1,1},
	{0,0,1,2,2,2,2,2,2,2,1},
	{0,1,2,2,3,3,3,3,3,2,2,1},
	{0,1,3,4,4,4,4,4,4,4,3,1},
	{0,1,3,4,1,4,4,4,1,4,3,1},
	{0,1,3,4,1,4,4,4,1,4,3,1},
	{0,1,3,5,4,4,4,4,4,5,3,1},
    {0,1,3,4,1,4,4,4,1,4,3,1},
	{0,1,3,4,4,1,1,1,4,4,3,1},
	{0,0,1,4,4,4,4,4,4,4,1},
	{0,1,6,6,7,7,7,7,7,6,6,1},
	{1,6,6,6,6,7,7,7,6,6,6,6,1},
	{1,6,1,6,6,7,7,7,6,6,1,6,1},
	{1,7,1,6,6,7,7,7,6,6,1,7,1},
	{1,4,1,6,6,7,7,7,6,6,1,4,1},
	{0,1,1,7,7,7,7,7,7,7,1,1},
	{0,0,1,8,9,9,9,9,9,8,1},
	{0,0,1,9,9,1,1,1,9,9,1},
	{0,0,1,6,6,1,6,1,6,6,1},
	{0,0,0,1,1,0,0,0,1,1}
};

static UINT8 BOX_PIC[15][15] = {  //���ӵ�ͼ����Ϣ
	{ 0,0,0,1,1,1,1,1,1,1,1,1 },
	{ 0,0,1,2,2,2,2,2,2,2,2,2,1 },
	{ 0,1,2,2,2,2,2,2,2,2,2,2,2,1 },
	{ 1,2,2,2,2,2,2,2,2,2,2,2,2,2,1 },
	{ 1,2,2,2,2,2,2,2,2,2,2,2,2,2,1 },
	{ 1,2,2,2,2,2,2,2,2,2,2,2,2,2,1 },
	{ 1,2,2,2,2,2,1,1,1,2,2,2,2,2,1 },
	{ 1,1,1,1,1,1,1,3,1,1,1,1,1,1,1 },
	{ 1,3,3,3,3,3,1,1,1,3,3,3,3,3,1 },
	{ 1,3,3,3,3,3,3,3,3,3,3,3,3,3,1 },
	{ 1,3,3,3,3,3,3,3,3,3,3,3,3,3,1 },
	{ 1,3,3,3,3,3,3,3,3,3,3,3,3,3,1 },
	{ 0,1,3,3,3,3,3,3,3,3,3,3,3,1 },
	{ 0,0,1,3,3,3,3,3,3,3,3,3,1 },
	{ 0,0,0,1,1,1,1,1,1,1,1,1 },
};

static UINT8 DESTINATION_PIC[19][15] = {   //����Ŀ�ĵص�ͼ����Ϣ
	{1,1,0,0,0,0,0,0,0,0,0,1,1},
	{1,1,1,0,0,0,0,0,0,0,1,1,1},
	{1,2,2,1,1,0,0,0,1,1,2,2,1},
	{1,2,2,2,2,1,1,1,2,2,2,2,1},
	{0,1,2,2,2,2,2,2,2,2,2,1},
	{0,0,1,2,2,2,2,2,2,2,1},
	{0,1,2,2,1,2,2,2,1,2,2,1},
	{0,1,2,2,1,2,1,2,1,2,2,1,1,1,1},
	{0,1,2,3,2,1,2,1,2,3,2,1,2,2,1},
	{0,0,1,1,2,2,2,2,2,1,1,2,2,2,1},
	{0,1,2,2,1,2,2,2,1,2,2,1,2,1,1},
	{0,1,2,2,1,2,2,2,1,2,2,1,2,1},
	{0,1,2,2,2,2,2,2,2,2,2,1,2,1 },
	{0,1,4,2,2,2,2,2,2,2,4,1,2,1 },
	{0,1,2,2,2,2,2,2,2,2,2,1,1},
	{0,1,4,2,2,2,2,2,2,2,4,1},
	{0,1,2,2,2,2,2,2,2,2,2,1},
	{0,0,1,2,2,1,1,1,2,2,1},
	{0,0,0,1,1,0,0,0,1,1}
};

int main
    (IN int Argc
    , IN char **Argv) 
{
	EFI_STATUS Status;
    EFI_SIMPLE_TEXT_OUTPUT_MODE SavedConsoleMode;
     // Save the current console cursor position and attributes
    CopyMem(&SavedConsoleMode, gST->ConOut->Mode, sizeof(EFI_SIMPLE_TEXT_OUTPUT_MODE));
    Status = gST->ConOut->EnableCursor(gST->ConOut, FALSE); //���ù���Ƿ����
    ASSERT_EFI_ERROR(Status);
    Status = gST->ConOut->ClearScreen(gST->ConOut);    //����
    ASSERT_EFI_ERROR(Status);
    Status = gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));//������Ļ�������ʽ
    ASSERT_EFI_ERROR(Status);
    Status = gST->ConOut->SetCursorPosition(gST->ConOut, 0, 0);   //���ù��λ��
    ASSERT_EFI_ERROR(Status);
	
	
	constructGameFile();
    init();      
	while(1){        //ʹ��һ��ѭ����������Ϸ�Ľ���
		updateKey(); //updateKey() �� render()���� �������¼��Ķ�ȡ����Ϸ״̬������ �� ��Ӧ���߼����� �����˷���, ʹ������Ŀ�߼�������
		if (isRefresh == 1)
			render();
		if (game_status == EXIT)
			break;
	};

	gST->ConOut->EnableCursor(gST->ConOut, SavedConsoleMode.CursorVisible);
    gST->ConOut->SetCursorPosition(gST->ConOut, SavedConsoleMode.CursorColumn, SavedConsoleMode.CursorRow);
    gST->ConOut->SetAttribute(gST->ConOut, SavedConsoleMode.Attribute);
    gST->ConOut->ClearScreen(gST->ConOut);
    return 0;
}

void init() {
	game_status = WELCOME;
	isRefresh = 1;
	currentLevel = 1;
}

void updateKey() {
	EFI_STATUS _status;
	EFI_INPUT_KEY key;
	if (!isSelectLevel) { //�������ѡ��״̬, �������������������¼�, ����ѡ�غ���ר�Ŵ���
		_status = gST->ConIn->ReadKeyStroke(gST->ConIn, &key);
		ASSERT_EFI_ERROR(_status);
		if (EFI_ERROR(_status))
			return;
	}

	switch (game_status) {
	case WELCOME:	
		if (key.ScanCode == SCAN_ESC) //�˳���Ϸ
			exitGame();
		else if (key.ScanCode == SCAN_F1) { //����˵�����
			showMenu();
			game_status = MENU;
		}
   		break;
	case GAMING:
		if (key.ScanCode == SCAN_UP) { //�����ƶ�
			Move_Directions = UP;
			isRefresh = 1;
		}
		else if (key.ScanCode == SCAN_DOWN) {//�����ƶ�
			Move_Directions = DOWN;
			isRefresh = 1;
		}
		else if (key.ScanCode == SCAN_LEFT) {//�����ƶ�
			Move_Directions = LEFT;
			isRefresh = 1;
		}
		else if (key.ScanCode == SCAN_RIGHT) {//�����ƶ�
			Move_Directions = RIGHT;
			isRefresh = 1;
		}
		else if (key.ScanCode == SCAN_F1) {//���汾��
			showGame();
			isRefresh = 0;
		}
		else if (key.ScanCode == SCAN_ESC) {//�˻ص��˵�����
			game_status = MENU;
			showMenu();
			isRefresh = 1;
		}
		break;
	case MENU:
		if (key.ScanCode == SCAN_F1) { //ֱ�ӿ�ʼ��Ϸ
			showGame();
			game_status = GAMING;
		}
		else if (key.ScanCode == SCAN_F2) { //ѡ��
			game_status = SELECT_LEVEL;
			isSelectLevel = 1;
		}
		else if (key.ScanCode == SCAN_ESC)//�˳���Ϸ
			game_status = EXIT;
		break;

	case SELECT_LEVEL:
		selectLevel();       //��������¼�
		showGame();          //��ʾ��Ϸ����
		game_status = GAMING;
		isSelectLevel = 0;   //��updatekey()�����ӹܼ����¼�����Ӧ����
		break;
	case PAUSE:
		break;
	case WIN:
		if (key.ScanCode == SCAN_ESC)        // exit the game
			game_status = EXIT;
		else if (key.ScanCode == SCAN_F1) {    // select game level
			game_status = SELECT_LEVEL;
			isSelectLevel = 1;
		}
		else if (key.ScanCode == SCAN_F2) {  // ���汾��
			showGame();
			game_status = GAMING;
		}
		else if (key.ScanCode == SCAN_F3) {  // ��һ��
			currentLevel++;
			if (currentLevel > 35)
				currentLevel = 1;
			showGame();
			game_status = GAMING;
		}
		break;
	case EXIT:
		exitGame();
	}
}

void render() {
	isRefresh = 0;
	switch (game_status)
	{
	case WELCOME:
		showWelcome();
		break;
	case GAMING:
		playGame();
		break;
	case MENU:
		break;
	default:
		break;
	}
}

void exitGame() {
	game_status = EXIT;
}

void showMenu() {
	clearScreen();
	Print(L"\n\n\n\n\r");
	Print(L"\t\t\t\t\t\t\t\t\t\t\t*************************** Menu ***************************\n\r");
	Print(L"\t\t\t\t\t\t\t\t\t\t\t*                        START GAME  (F1)                  *\n\r");
	Print(L"\t\t\t\t\t\t\t\t\t\t\t*                       SELECT LEVEL (F2)                  *\n\r");
	Print(L"\t\t\t\t\t\t\t\t\t\t\t*                           EXIT     (F3)                  *\n\r");
	Print(L"\t\t\t\t\t\t\t\t\t\t\t*************************** Menu ***************************\n\r");
}

void getLevel() {
	scanf("%d", &currentLevel);
}
void selectLevel() {
	clearScreen();
	Print(L"\n\n\r");
	Print(L"\t\t\t\t\t\t\t\t\t\t\tPlease Enter A Level(1~35):");
	do {
		getLevel();
	} while (currentLevel > 35 || currentLevel < 1);
}

void showWelcome(VOID) {
	gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
	Print(L"\n\n\r");
	Print(L"\t\t\t\t\t\t\t\t\t\t\t************************** SOKOBAN *************************\n\r");
	Print(L"\t\t\t\t\t\t\t\t\t\t\t*                  Welcome to UEFI Gameing                 *\n\r");
	Print(L"\t\t\t\t\t\t\t\t\t\t\t*                   By huzehao & yinyuhan                  *\n\r");
	Print(L"\t\t\t\t\t\t\t\t\t\t\t*                         MENU   (F1)                      *\n\r");
	Print(L"\t\t\t\t\t\t\t\t\t\t\t*                         EXIT   (ESC)                     *\n\r");
	Print(L"\t\t\t\t\t\t\t\t\t\t\t************************** SOKOBAN *************************\n\r");
}

void clearScreen() {
	gST->ConOut->ClearScreen(gST->ConOut);
}


void getFileName(int num, CHAR16* buf) { //ֻ���������λ����, ��buf �д����Ҫ�򿪵��ļ���, ���� num = 1 , ��buf = "1.txt"
	int bigThenTen = num / 10;
	CHAR16 a = (CHAR16)(bigThenTen + 48); //ʮλ
	CHAR16 b = (CHAR16)(num - bigThenTen * 10 + 48); //��λ
	if (bigThenTen == 0) {
		buf[0] = b;
		buf[1] = '.';
		buf[2] = 't';
		buf[3] = 'x';
		buf[4] = 't';
		buf[5] = '\0';
	}
	else {
		buf[0] = a;
		buf[1] = b;
		buf[2] = '.';
		buf[3] = 't';
		buf[4] = 'x';
		buf[5] = 't';
		buf[6] = '\0';
	}
}

void constructGameFile() {
	EFI_STATUS status;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem; //�ļ�ϵͳ���
	EFI_FILE_PROTOCOL *Root; //�ļ�ϵͳ��Ŀ¼���
	EFI_FILE_PROTOCOL *Dir;  //�ļ�ϵͳ, ��Ϸ��Դ�ļ��о��
	EFI_FILE_PROTOCOL *File; //��Ϸ�ļ����
	CHAR16 * TextBuf = (CHAR16*)L"CREATE SUCCESSFULLY!!!";
	CHAR16 *BUFFER;         
	UINTN Buffer_Size = sizeof(CHAR16) * 65;
	status = gBS->AllocatePool(EfiBootServicesCode, Buffer_Size, (VOID**)&BUFFER);
	if (!BUFFER || EFI_ERROR(status)) {
		Print(L"ALLOCATE FAILURE");
	}

	UINTN W_BufSize = 0;
	UINTN R_BufSize = 64;
	status = gBS->LocateProtocol(&gEfiSimpleFileSystemProtocolGuid,
		NULL, (VOID**)&SimpleFileSystem);
	if (EFI_ERROR(status))
	{
		Print(L"1");
	}
	status = SimpleFileSystem->OpenVolume(SimpleFileSystem, &Root);
	status = Root->Open(Root,
		&Dir,
		(CHAR16*)L"Sokoban",
		EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
		EFI_FILE_DIRECTORY);
	status = Dir->Open(Dir,
		&File,
		(CHAR16*)L"config.txt",
		EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
		0);

	if (File && !EFI_ERROR(status))
	{
		status = File->Read(File, &R_BufSize, BUFFER); //��config.txt�ļ��ж���Ϣ, �����ȡ�����ֽ���Ϊ0(��һ����Ϸ), �򴴽���Ϸ��Դ�ļ� 
		//Print(L"R_BufSize: %d\n\r", R_BufSize);      //�ڶ�����Ϸ���Ժ���Ҫ���´�����Դ�ļ�
		if (R_BufSize == 0) {
			W_BufSize = StrLen(TextBuf) * 2;
			status = File->Write(File, &W_BufSize, TextBuf);
			if (EFI_ERROR(status))
				Print(L"Write Failure");
			status = File->Close(File);
			CHAR16 filename[10];
			int i;
			for (i = 1; i <= 35; i++) {//��Sokoban�ļ����� ����1.txt ~ 35.txt
				getFileName(i, filename);
				status = Dir->Open(Dir, &File, filename,
					EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
				status = File->Write(File, &W_BufSize, TextBuf);
				status = File->Close(File);
			}
		}
		else {
			//BUFFER[R_BufSize] = 0;
			//Print(L"config Read: %s\n\r", BUFFER);
			status = File->Close(File);
		}
	}
	else {
		File->Close(File);
		Print(L"config.txt not exist");
	}
	status = Dir->Close(Dir);
	status = Root->Close(Root);
	if (EFI_ERROR(status))
		Print(L"Close root Failure");
	status = gBS->FreePool(BUFFER);
	if (EFI_ERROR(status))
		Print(L"Free error");
}

void loadMap() {
	CHAR16 level[10];
	getFileName(currentLevel, level);
	EFI_STATUS status;
	EFI_FILE_PROTOCOL *Dir = 0;
	EFI_FILE_PROTOCOL *Root;              //NT32��Ŀ¼���
	EFI_FILE_PROTOCOL *File;              //�����ӵ�ͼ�ļ��ľ��
	CHAR8 R_BUF[500];                      //��Ŷ�ȡ���ļ��е�����
	UINTN R_SIZE = 64;                      //���һ�ζ�ȡ���ֽ���, һ������64���ֽ�
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem;//UEFI�ļ�ϵͳ���
	status = gBS->LocateProtocol(&gEfiSimpleFileSystemProtocolGuid,
		NULL, (VOID**)&SimpleFileSystem); //��ȡUEFI�ļ�ϵͳ���
	if (EFI_ERROR(status)) {
		Print(L"SimpleFileSystem");
	}
	status = SimpleFileSystem->OpenVolume(SimpleFileSystem, &Root); //��ȡNT32��Ŀ¼���
	
	status = Root->Open(                 //��ȡNT32��Ŀ¼��Sokoban�ļ��еľ��                              
		Root,
		&Dir,
		(CHAR16*)(L"Sokoban"),
		EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,         
		EFI_FILE_DIRECTORY
	);
	status = Dir->Open(Dir, &File, level,
		EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE , 0);

	status = File->Read(File, &R_SIZE, R_BUF);
	UINTN SUM = R_SIZE;
	//gST->ConOut->SetCursorPosition(gST->ConOut, 0, 40);
	//Print(L"SIZE: %d\n\r", R_SIZE);
	//R_BUF[65] = 0;
	//Print(L"r_buf: \n\r %s", R_BUF);
	while (R_SIZE == 64) { //ѭ��������Ϸ��Դ�ļ��� R_BUF , һ��������64���ֽ�
		status = File->Read(File, &R_SIZE, R_BUF + SUM);
		SUM += R_SIZE;
	}

	char c; 
	int _row = 0;
	int _col = 0;
	for (UINTN i = 0; i < SUM; i++) { //�� R_BUF�е���Ϣӳ�䵽MAP��
		c = R_BUF[i];
		if (c != NULL) {
			if (c == '\n'){
				_row++;
				_col = 0;
			}
			else MAP[_row][_col++] = c;
		}
	}
	rows = _row + 1;   //��ʼ�����صĵ�ͼ������������
	cols = _col;
	/*for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			Print(L"%c", MAP[i][j]);
		}
		Print(L"\n");
	}*/
	
	status = Root->Close(Root);
	status = Dir->Close(Dir);
	status = File->Close(File);
}

int filrate() {   // "��Ĥ��" , ������Ŀ�ĵص����ˢ��һ�� 
	int i;
	int j = 0;
	char c;
	for (i = 0;  i < boxNum; i++) {
		c = MAP[Y_Pos[i]][X_Pos[i]];
		if (c == ' ')    // ����������Ŀ�ĵص���playGame()�����Ĳ����б�Ū���˿�λ��, �����������ó�����Ŀ�ĵ�
			MAP[Y_Pos[i]][X_Pos[i]] = 'X';

		if (c == 'O'|| c== 'Q') { //����������Ŀ�ĵ�, ����ʱ�ŵ�������, �򽫸�λ�ñ�ʾΪ�����Ƶ���Ŀ�ĵ�
			MAP[Y_Pos[i]][X_Pos[i]] = 'Q';
			j++;
		}

	}
	if (j == boxNum) // ���, ���е����Ӷ�����Ŀ�ĵ�, ��pass
		return 1;
	else return 0;
}

void updateMap(int x, int y, char c) { //������Ϸ����ĵ�ͼ
	
	switch (c)
	{
	case 'X':
		copyRectangle(61, 0, 30 * x, 30 * y, 29, 30);
		break;
	case 'Q':
	case 'O':
		copyRectangle(90, 0, 30 * x, 30 * y, 30, 30);
		break;
	case '@':
		copyRectangle(31, 0, 30 * x, 30 * y, 29, 30);
		break;
	case ' ':
		drawRectangle(30 * x, 30 * y, 30, 30, BLACK);
		break;
	case '#':
		copyRectangle(0, 0, 30 * x, 30 * y, 30, 30);
		break;
	}

}

void playGame() {
	Steps += 1;  
	char thing_front_man;
	char thing_front_box;
	Node old_pos;
	old_pos.x = MAN.x;
	old_pos.y = MAN.y;
	int _move = 0;
	int _x = 0; //����ǰ��Ķ�����x������
	int _y = 0;  //����ǰ��Ķ�����y������
	int __x = 4; //����ǰ���ǰ��Ķ�����x������
	int __y = 3; //...

	Direction direction;
	switch (Move_Directions)
	{
	case UP:
		direction.x = 0;
		direction.y = -1;
		break;
	case DOWN:
		direction.x = 0;
		direction.y = 1;
		break;
	case LEFT:
		direction.x = -1;
		direction.y = 0;
		break;
	case RIGHT:
		direction.x = 1;
		direction.y = 0;
		break;
	}

    _x = MAN.x + direction.x;
	_y = MAN.y + direction.y;
	thing_front_man = MAP[_y][_x];
	if (thing_front_man != '#') {  //��������ǰ�治��ǽ
		if (thing_front_man == ' ' || thing_front_man == 'X') { //��������ǰ���ǿ�λ�û������ӵ�Ŀ�ĵ�
			MAN.y += direction.y;
			MAN.x += direction.x;
			MAP[MAN.y][MAN.x] = '@';
			MAP[old_pos.y][old_pos.x] = ' ';
			_move = 1;
		}
		else {  //��������ǰ��������
			__x = _x + direction.x;
			__y = _y + direction.y;
			thing_front_box = MAP[__y][__x];
			if (thing_front_box == 'X' || thing_front_box == ' ') {//���ӵ�ǰ���ǿյػ�Ŀ�ĵ�
				MAP[__y][__x] = 'O';
				MAP[_y][_x] = '@';
				MAP[MAN.y][MAN.x] = ' ';
				MAN.x += direction.x;
				MAN.y += direction.y;
				_move = 1;
				
			}
		}
	}
	if (_move == 1) {
		if (filrate()) {
			showWin();
			game_status = WIN;
			return;
		} //����ÿ�ƶ�һ�����ı��ͼ������λ�õ���Ϣ, ����ֻ��������λ������Ϸ�����Ͻ���ˢ�� ,������һ����ˢ����������
		updateMap(old_pos.x, old_pos.y, MAP[old_pos.y][old_pos.x]);
		updateMap(_x, _y, MAP[_y][_x]);
		if (__x != 0)   //��������ǰ��������, �������ӵ�ǰ���ǿյػ�Ŀ�ĵص�ʱ, Ҫ�������ǰ���ǰ���λ�ý���ˢ��
			updateMap(__x, __y, MAP[__y][__x]);
	}
	showSteps();   //���½���Ĳ���
}

void showGame() {
	clearScreen();
	Steps = 0;
	loadMap();
	initMap();
	showMap();
	showTips();
}

void initMap() {             //����ά����ת��Ϊһά����, ����ά����������λ����Ϣ����
	char tmp;
	boxNum = 0;
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			tmp = MAP[i][j];
			if (tmp == 'X' || tmp == 'Q') { //�õ�����Ŀ�ĵص���������ӵ�����
				X_Pos[boxNum] = j;
				Y_Pos[boxNum] = i;
				boxNum++;
			}
			if (tmp == '@') { //�õ�����ĳ�ʼλ����Ϣ
				MAN.x = j;
				MAN.y = i;
				MAN.text[0] = tmp;
				MAN.text[1] = '\0';
			}
		}
	}
}

void showWin() {
	clearScreen();
	gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
	Print(L"\n\n\r");
	Print(L"\t\t\t\t******************************  YOU WIN !!! *****************************\n\r");
	Print(L"\t\t\t\t*                             SELECT LEVEL (F1)                         *\n\r");
	Print(L"\t\t\t\t*                             PLAY AGAIN   (F2)                         *\n\r");
	Print(L"\t\t\t\t*                             NEXT LEVEL   (F3)                         *\n\r");
	Print(L"\t\t\t\t*                                 EXIT     (ESC)                        *\n\r");
	Print(L"\t\t\t\t******************************  YOU WIN !!! *****************************\n\r");
}

void showTips() {
	gST->ConOut->SetCursorPosition(gST->ConOut, 50, 2);
	gST->ConOut->OutputString(gST->ConOut, L"Controll:  ��������");
	showSteps();
	gST->ConOut->SetCursorPosition(gST->ConOut, 50, 6);
	gST->ConOut->OutputString(gST->ConOut, L"Replay(F1)");
	gST->ConOut->SetCursorPosition(gST->ConOut, 50, 8);
	gST->ConOut->OutputString(gST->ConOut, L"Exit(ESC)");
}

void showSteps() {
	gST->ConOut->SetCursorPosition(gST->ConOut, 50, 4);
	Print(L"Step: %d", Steps);
}

void showMap() { //�ڽ���ÿһ����Ϸʱֻ�����һ��, ������Ϸ��ͼ����
	drawRectangle(1, 1, 28, 13,DARK_BROWN);   //��ǽ
	drawRectangle(0, 16, 14, 13,DARK_BROWN);  //��ǽ
	drawRectangle(17, 16, 14, 13,DARK_BROWN); //��ǽ
	drawDestination();
	drawMan();
	drawBox();
	int i, j;
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			char tmp = MAP[i][j];
			if (tmp == '#') {
				copyRectangle(0, 0, 30 * j, 30 * i, 30, 30);
			}
			else if (tmp == 'X') {
				copyRectangle(61, 0, 30 * j, 30 * i, 30, 30);
			}
			else if (tmp == 'O' || tmp == 'Q' ) {
				copyRectangle(91, 0, 30 * j, 30 * i, 30, 30);
			}
			else if (tmp == '@') {
				copyRectangle(31, 0, 30 * j, 30 * i, 30, 30);
			}
		}
	}

}

//����ĺ����ǻ��� ����,����, ����Ŀ�ĵ� �ĺ���, �ڽ���ÿ��ʱֻ�����һ��, �����ı�д�е㲻���ģʽ, �Ժ��ٸ�...
//������ʾͼ�������, һ������һ�����صĻ���ͼ��
void drawBox() {
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gGraphicsOutput;
	EFI_STATUS _status;
	_status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,
		NULL, (VOID**)&gGraphicsOutput);
	if (EFI_ERROR(_status)) {
		Print(L"draw Fail!!!");
	}
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL COMMON[1] = { 0,0,0,0 };
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 15; j++) {
			switch (BOX_PIC[i][j])
			{
			case 1:
				COMMON[0] = BLACK[0];
				break;
			case 2:
				COMMON[0] = RED[0];
				break;
			case 3:
				COMMON[0] = WHITE[0];
				break;
			}

			_status = gGraphicsOutput->Blt(gGraphicsOutput,
				COMMON,
				EfiBltVideoFill,
				0, 0,
				j + 97, i + 7,
				1, 1,
				0);

		}
	}
}

void drawDestination() {
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gGraphicsOutput;
	EFI_STATUS _status;
	_status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,
		NULL, (VOID**)&gGraphicsOutput);
	if (EFI_ERROR(_status)) {
		Print(L"draw Fail!!!");
	}
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL COMMON[1] = { 0,0,0,0 };
	for (int i = 0; i < 19; i++) {
		for (int j = 0; j < 15; j++) {

			switch (DESTINATION_PIC[i][j])
			{
			case 1:
				COMMON[0] = BLACK[0];
				break;
			case 2:
				COMMON[0] = YELLOW[0];
				break;
			case 3:
				COMMON[0] = RED[0];
				break;
			case 4:
				COMMON[0] = DARK_BROWN[0];
				break;
			default:
				break;
			}
			_status = gGraphicsOutput->Blt(gGraphicsOutput,
				COMMON,
				EfiBltVideoFill,
				0, 0,
				j + 67, i + 5,
				1, 1,
				0);
		}
	}
}

void drawMan() {
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gGraphicsOutput;
	EFI_STATUS _status;
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL COMMON[1] = { 0,0,0,0 };
	_status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,
		NULL, (VOID**)&gGraphicsOutput);
	if (EFI_ERROR(_status)) {
		Print(L"draw Fail!!!");
	}

	for (int i = 0; i < 19; i++) {
		for (int j = 0; j < 15; j++) {
			switch (MAN_PIC[i][j]) {
			case 1:
				COMMON[0] = BLACK[0];
				break;
			case 2:
				COMMON[0] = LIGHT_BROWN[0];
				break;
			case 3:
				COMMON[0] = DARK_BROWN[0];
				break;
			case 4:
				COMMON[0] = SKIN[0];
				break;
			case 5:
				COMMON[0] = LIGHT_RED[0];
				break;
			case 6:
				COMMON[0] = WHITE[0];
				break;
			case 7:
				COMMON[0] = PINK[0];
				break;
			case 8:
				COMMON[0] = DARK_BLUE[0];
				break;
			case 9:
				COMMON[0] = LIGHT_BLUE[0];
				break;
			}
			
			_status = gGraphicsOutput->Blt(gGraphicsOutput,
				COMMON,
				EfiBltVideoFill,
				0, 0,
				j + 38, i + 4,
				1, 1,
				0);
		}
	}
}

void drawFloor() {

}