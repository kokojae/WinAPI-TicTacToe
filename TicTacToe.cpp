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

// 마커 열거형, 마커 종류 NONE(0):빈칸 / CROSS(1):X / CIRCLE(2):O
enum Marker { NONE, CROSS, CIRCLE };

// 3 * 3 보드 정보 및 현재 마커 정보 저장
class Board
{
private:
	// 상대 상수 필드
	const POINT startPosition;	// 보드 그리는 위치
	const int cellSize;			// 칸 크기
	const int markerSize;		// 마커 크기
	const int markerPadding;	// 칸과 마커 사이 공간

	// 멤버 변수 필드
	RECT cellRect[3][3];	// 보드 칸 RECT 좌표 저장용
	Marker marker[3][3];	// 마커 상태 저장
	RECT markerRect[3][3];	// 마커가 그려질 RECT 좌표 저장용
	HPEN crossPen;			// X 마커 그릴 펜
	HPEN circlePen;			// O 마커 그릴 펜
	HPEN oldPen;			// 펜 교환용 변수
	HBRUSH crossBrush;		// X 마커 그릴 브러쉬
	HBRUSH circleBrush;		// O 마커 그릴 브러쉬
	HBRUSH oldBrush;		// 브러쉬 교환용 변수

	// 멤버 함수
	// 마커 초기화
	void ResetMarker()
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				marker[i][j] = Marker::NONE;
			}
		}
	}

	// 보드 칸 RECT 계산
	void CalCellRect()
	{
		for (int col = 0; col < 3; col++)
		{
			for (int row = 0; row < 3; row++)
			{
				cellRect[col][row] = {
					startPosition.x + row * cellSize,
					startPosition.y + col * cellSize,
					startPosition.x + (row + 1) * cellSize,
					startPosition.y + (col + 1) * cellSize
				};
			}
		}
	}

	// 마커 RECT 계산
	void CalMarkerRect()
	{
		for (int col = 0; col < 3; col++)
		{
			for (int row = 0; row < 3; row++)
			{
				markerRect[col][row] = {
					startPosition.x + row * cellSize + markerPadding,
					startPosition.y + col * cellSize + markerPadding,
					startPosition.x + (row + 1) * cellSize - markerPadding,
					startPosition.y + (col + 1) * cellSize - markerPadding
				};
			}
		}
	}

	// 3*3 보드 그리기
	void DrawBoard(HDC hdc)
	{
		for (int col = 0; col < 3; col++)
		{
			for (int row = 0; row < 3; row++)
			{
				Rectangle(hdc, cellRect[col][row].left, cellRect[col][row].top, cellRect[col][row].right, cellRect[col][row].bottom);
			}
		}
	}

	// 펜 변경
	void ChangePen(HDC hdc, HPEN hPen)
	{
		if (hPen != NULL)							// 변경할 펜이 NULL이 아니라면
			oldPen = (HPEN)SelectObject(hdc, hPen);	// 지정된 펜으로 변경
	}

	// 펜 사용 종료
	void EndPen(HDC hdc)
	{
		if (oldPen != NULL)				// oldPen이 NULL이 아니라면
		{
			SelectObject(hdc, oldPen);	// 기존 펜으로 변경
			oldPen = NULL;				// 잘못 된 해제 방지를 위한 초기화
		}
	}

	// 브러쉬 변경
	void ChangeBrush(HDC hdc, HBRUSH hBrush)
	{
		if (hBrush != NULL)									// 변경할 브러쉬가 NULL이 아니라면
			oldBrush = (HBRUSH)SelectObject(hdc, hBrush);	// 지정된 브러쉬로 변경
	}

	// 브러쉬 사용 종료
	void EndBrush(HDC hdc)
	{
		if (oldBrush != NULL)				// oldBrush가 NULL이 아니라면
		{
			SelectObject(hdc, oldBrush);	// 기존 브러쉬로 변경
			oldBrush = NULL;				// 잘못된 해제 방지를 위한 초기화
		}
	}

	// X 마커 그리기
	void DrawCrossMarker(HDC hdc, int col, int row)
	{
		ChangePen(hdc, crossPen);
		ChangeBrush(hdc, crossBrush);

		MoveToEx(hdc, markerRect[col][row].left, markerRect[col][row].top, NULL);
		LineTo(hdc, markerRect[col][row].right, markerRect[col][row].bottom);
		MoveToEx(hdc, markerRect[col][row].right, markerRect[col][row].top, NULL);
		LineTo(hdc, markerRect[col][row].left, markerRect[col][row].bottom);

		EndPen(hdc);
		EndBrush(hdc);
	}

	// O 마커 그리기
	void DrawCircleMarker(HDC hdc, int col, int row)
	{
		ChangePen(hdc, circlePen);
		ChangeBrush(hdc, circleBrush);

		Ellipse(hdc, markerRect[col][row].left, markerRect[col][row].top, markerRect[col][row].right, markerRect[col][row].bottom);

		EndPen(hdc);
		EndBrush(hdc);
	}

	// 마커 그리기
	void DrawMarkers(HDC hdc)
	{
		for (int col = 0; col < 3; col++)
		{
			for (int row = 0; row < 3; row++)
			{
				switch (marker[col][row])
				{
				case Marker::CROSS: DrawCrossMarker(hdc, col, row); break;
				case Marker::CIRCLE: DrawCircleMarker(hdc, col, row); break;
				}
			}
		}
	}

	// 안내 텍스트 그리기
	void DrawTip(HDC hdc)
	{
		RECT rt = { startPosition.x,startPosition.y - 30,500,startPosition.y };	// 텍스트 출력 범위 지정
		DrawText(hdc, TEXT("이동: 방향키, 입력: 엔터, 종료: ESC"), -1, &rt, DT_LEFT);	// 안내 텍스트 출력
	}

public:
	// 생성자
	Board(POINT _startPosition, int _cellSize) :
		startPosition(_startPosition),
		cellSize(_cellSize),
		markerSize(_cellSize * 0.9),
		markerPadding(_cellSize * 0.1)
	{
		crossPen = NULL;
		circlePen = NULL;
		oldPen = NULL;
		crossBrush = NULL;
		circleBrush = NULL;
		oldBrush = NULL;

		ResetMarker();
		CalMarkerRect();
		CalCellRect();
	}

	void SetCrossPen(HPEN hPen) { crossPen = hPen; }	// X 마커 펜 지정
	void SetCirclePen(HPEN hPen) { circlePen = hPen; }	// O 마커 펜 지정
	void SetCrossBrush(HBRUSH hBrush) { crossBrush = hBrush; }	// X 마커 브러쉬 지정
	void SetCircleBrush(HBRUSH hBrush) { circleBrush = hBrush; }	// O 마커 브러쉬 지정

	// 지정된 위치에 마커 작성. 성공 시 true 반환, 실패 시 false 반환
	bool SetMarker(POINT position, Marker markerType)
	{
		if (marker[position.y][position.x] != Marker::NONE) return false;	// 만약 빈칸이 아니라면 false 반환

		marker[position.y][position.x] = markerType;	// 해당 위치에 마커 작성

		return true;	// 성공 시 ture 반환
	}

	// 보드 그리기
	void Draw(HDC hdc)
	{
		DrawTip(hdc);
		DrawBoard(hdc);
		DrawMarkers(hdc);
	}
};

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
	POINT position;		// 커서 위치(0~2, 0~2) 정보
	RECT cursorRect;	// 커서 좌표(픽셀) 정보

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

	POINT GetPos() { return position; }		// 커서의 좌표(0~2,0~2) 반환

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
	static const int CELL_SIZE = 100;					// 칸 크기 상수
	static const POINT START_POSITION = { 50,50 };		// 보드 시작 위치 상수

	static HDC hdc;										// Handle of Device Context
	static PAINTSTRUCT ps;								// Paintstruct

	static HPEN nowPen, redPen, greenPen;				// 커서를 그릴 때 사용할 펜(현재 사용자의 펜, 빨강, 초록)
	static HBRUSH redBrush, greenBrush;					// 사용할 브러쉬

	static Board board(START_POSITION, CELL_SIZE);		// 보드 정보
	static Cursor cursor(START_POSITION, CELL_SIZE);	// 커서 정보

	static bool isFirstPlayer = true;					// 현재 플레이어 정보

	switch (iMessage)
	{
	case WM_CREATE:
		redPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));	// 빨간펜 생성
		greenPen = CreatePen(PS_SOLID, 5, RGB(0, 255, 0));	// 초록펜 생성

		redBrush = CreateSolidBrush(RGB(255, 0, 0));		// 빨간 브러쉬 생성
		greenBrush = CreateSolidBrush(RGB(0, 255, 0));		// 초록 브러쉬 생성

		board.SetCrossPen(greenPen);		// X 마커 펜 지정
		board.SetCrossBrush(greenBrush);	// X 마커 브러쉬 지정
		board.SetCirclePen(redPen);			// O 마커 펜 지정
		board.SetCircleBrush(redBrush);		// O 마커 브러쉬 지정

		nowPen = greenPen;		// 초록 플레이어가 선이기 때문에 현재 펜을 초록 펜으로 지정
		return 0;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT: cursor.MoveLeft(); break;		// 커서 왼쪽으로 이동
		case VK_UP: cursor.MoveUp(); break;			// 커서 위쪽으로 이동
		case VK_RIGHT: cursor.MoveRight(); break;	// 커서 오른쪽으로 이동
		case VK_DOWN: cursor.MoveDown(); break;		// 커서 아래쪽으로 이동
		case VK_RETURN:		// Enter 입력 시
			if (board.SetMarker(cursor.GetPos(), (isFirstPlayer ? Marker::CROSS : Marker::CIRCLE)))		// 커서 위치에 마커 입력
			{
				// 마커 입력 성공 시
				isFirstPlayer = !isFirstPlayer;					// 유저 변경
				nowPen = (isFirstPlayer ? greenPen : redPen);	// 펜 변경
				cursor.ResetPos();								// 커서 위치 초기화
			}
			break;
		case VK_ESCAPE: DestroyWindow(hWnd); break;	// 게임(프로그램) 종료
		}

		InvalidateRect(hWnd, NULL, TRUE);
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		board.Draw(hdc);	// 보드 그리기

		cursor.Draw(hdc, nowPen);	// 커서 그리기

		EndPaint(hWnd, &ps);
		return 0;

	case WM_DESTROY:
		DeleteObject(redPen);
		DeleteObject(greenPen);

		DeleteObject(redBrush);
		DeleteObject(greenBrush);

		PostQuitMessage(0);
		return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

