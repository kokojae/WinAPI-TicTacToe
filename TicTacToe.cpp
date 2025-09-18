#include <windows.h>
#include <string>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("TicTacToe");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;

	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return (int)Message.wParam;
}

// 3*3 보드 그리기
void DrawBoard(HDC hdc, int x, int y, int CellSize)
{
	Rectangle(hdc, x, y, CellSize * 3 + x, CellSize * 3 + y);				// 외곽 라인
	Rectangle(hdc, x + CellSize, y, CellSize * 2 + x, CellSize * 3 + y);	// 세로 라인
	MoveToEx(hdc, x, y + CellSize, NULL);									// 상단 가로 시작
	LineTo(hdc, CellSize * 3 + x, y + CellSize);							// 상단 가로 그리기
	MoveToEx(hdc, x, y + CellSize * 2, NULL);								// 하단 가로 시작
	LineTo(hdc, CellSize * 3 + x, y + CellSize * 2);						// 하단 가로 그리기

	RECT rt = { x,y - 30,500,y };	// 텍스트 출력 범위 지정
	DrawText(hdc, TEXT("이동: 방향키, 입력: 엔터, 종료: ESC"), -1, &rt, DT_LEFT);	// 안내 텍스트 출력

}

// 커서 정보 저장 및 관리를 위한 클래스
class Cursor
{
private:
	// 절대 상수 필드
	static const int MINPOS = 0;		// 커서 위치 최솟값
	static const int MAXPOS = 2;		// 커서 위치 최댓값

	// 상대 상수 필드
	const POINT boardPosition;	// 보드 시작 위치
	const int cursorSize;		// 커서 크기

	// 변수 필드
	POINT position;		// 커서 위치 정보
	RECT cursorRect;	// 커서 좌표 정보

	// 함수 필드
	// 값이 [MINPOS, MAXPOS]의 범위를 벗어났다면 true
	static bool OutRange(int v) { return MINPOS > v || v > MAXPOS; }

	// cursorRect 값 재조정
	void CalRect()
	{
		cursorRect = {
			position.x * cursorSize + boardPosition.x,			// Left
			position.y * cursorSize + boardPosition.y,			// Top
			(position.x + 1) * cursorSize + boardPosition.x,	// Right
			(position.y + 1) * cursorSize + boardPosition.y		// Bottom
		};
	}

	// 지정된 좌표로 커서 이동
	void SetCursorPos(int x, int y)
	{
		position = { x,y };	// position을 x,y값으로 변경

		CalRect();	// cursorRect 값 설정
	}

	/// <summary>
	/// 입력한 좌표만큼 커서 이동
	/// </summary>
	/// <param name="dx">이동할 x값</param>
	/// <param name="dy">이동할 y값</param>
	void MoveCursor(int dx, int dy)
	{
		int nx = position.x + dx;	// 이동 시 x 좌표
		int ny = position.y + dy;	// 이동 시 y 좌표

		if (OutRange(nx) || OutRange(ny)) return;	// 이동한 좌표가 범위를 벗어날 경우 함수 종료

		SetCursorPos(nx, ny);	// 커서를 (nx, ny)로 이동
	}

public:
	// 생성자
	Cursor(POINT _boardPosition, int _cursorSize) :boardPosition(_boardPosition), cursorSize(_cursorSize)
	{
		position = { 0,0 };
		CalRect();
	}

	void MoveLeft() { MoveCursor(-1, 0); }	// 커서의 x를 -1 만큼 이동
	void MoveUp() { MoveCursor(0, -1); }	// 커서의 y를 -1 만큼 이동
	void MoveRight() { MoveCursor(1, 0); }	// 커서의 x를 +1 만큼 이동
	void MoveDown() { MoveCursor(0, 1); }	// 커서의 y를 +1 만큼 이동

	void ResetPos() { SetCursorPos(0, 0); }	// 커서의 위치를 (0, 0)으로 초기화

	// 커서 그리기
	void Draw(HDC hdc, HPEN pen = NULL)
	{
		HPEN oldPen = NULL;		// 이전 펜 정보 저장용

		if (pen != NULL)							// 지정한 펜이 있다면
			oldPen = (HPEN)SelectObject(hdc, pen);	// 지정된 펜으로 변경

		MoveToEx(hdc, cursorRect.left, cursorRect.top, NULL);	// 커서 시작 위치로 이동
		LineTo(hdc, cursorRect.right, cursorRect.top);			// 상단 라인 그리기
		LineTo(hdc, cursorRect.right, cursorRect.bottom);		// 우측 라인 그리기
		LineTo(hdc, cursorRect.left, cursorRect.bottom);		// 하단 라인 그리기
		LineTo(hdc, cursorRect.left, cursorRect.top);			// 좌측 라인 그리기

		if (pen != NULL)				// 지정한 펜이 있다면
			SelectObject(hdc, oldPen);	// 기존 펜으로 변경
	}
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static const int CELL_SIZE = 100;
	static const POINT START_POSITION = { 50,50 };

	static HDC hdc;										// Handle of Device Context
	static PAINTSTRUCT ps;								// Paintstruct
	static HPEN nowPen, redPen, greenPen;				// 커서를 그릴 때 사용할 펜(현재 사용자의 펜, 빨강, 초록)
	static HBRUSH myBrush, oldBrush;					// 사용할 브러쉬
	static Cursor cursor(START_POSITION, CELL_SIZE);	// 커서 정보

	switch (iMessage)
	{
	case WM_CREATE:
		greenPen = CreatePen(PS_SOLID, 5, RGB(0, 255, 0));	// 초록펜 생성
		redPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));	// 빨간펜 생성
		nowPen = greenPen;		// 초록 플레이어가 선이기 때문에 현재 펜을 초록 펜으로 지정
		return 0;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT: cursor.MoveLeft(); break;		// 커서 왼쪽으로 이동
		case VK_UP: cursor.MoveUp(); break;			// 커서 위쪽으로 이동
		case VK_RIGHT: cursor.MoveRight(); break;	// 커서 오른쪽으로 이동
		case VK_DOWN: cursor.MoveDown(); break;		// 커서 아래쪽으로 이동
		case VK_ESCAPE:DestroyWindow(hWnd); break;	// 게임(프로그램) 종료
		}

		InvalidateRect(hWnd, NULL, TRUE);
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		DrawBoard(hdc, START_POSITION.x, START_POSITION.y, CELL_SIZE);	// 보드 그리기

		cursor.Draw(hdc, nowPen);

		EndPaint(hWnd, &ps);
		return 0;

	case WM_DESTROY:
		DeleteObject(redPen);
		DeleteObject(greenPen);

		PostQuitMessage(0);
		return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

