#include <windows.h>

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
	// 절대 상수 필드
	static const LPCTSTR basicToolTip;		// 기본 텍스트
	static const LPCTSTR crossWinText;		// X 승리 텍스트
	static const LPCTSTR circleWinText;		// O 승리 텍스트
	static const LPCTSTR drawText;			// 무승부 텍스트
	// Board 하단에 값 지정

	// 상대 상수 필드
	const POINT startPosition;	// 보드 그리는 위치
	const int cellSize;			// 칸 크기
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

	RECT textRect;			// 텍스트 출력 RECT
	LPCTSTR nowText;		// 현재 텍스트

	// 멤버 함수
	// 보드 칸 RECT 계산
	void CalCellRect()
	{
		for (int row = 0; row < 3; row++)
		{
			for (int col = 0; col < 3; col++)
			{
				cellRect[row][col] = {
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
		for (int row = 0; row < 3; row++)
		{
			for (int col = 0; col < 3; col++)
			{
				markerRect[row][col] = {
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
		for (int row = 0; row < 3; row++)
		{
			for (int col = 0; col < 3; col++)
			{
				Rectangle(
					hdc,
					cellRect[row][col].left,
					cellRect[row][col].top,
					cellRect[row][col].right,
					cellRect[row][col].bottom
				);
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
	void DrawCrossMarker(HDC hdc, int row, int col)
	{
		ChangePen(hdc, crossPen);
		ChangeBrush(hdc, crossBrush);

		MoveToEx(hdc,	markerRect[row][col].left,	markerRect[row][col].top, NULL);	// 우하향 대각선
		LineTo(hdc,		markerRect[row][col].right,	markerRect[row][col].bottom);

		MoveToEx(hdc,	markerRect[row][col].right,	markerRect[row][col].top, NULL);	// 좌하향 대각선
		LineTo(hdc,		markerRect[row][col].left,	markerRect[row][col].bottom);

		EndPen(hdc);
		EndBrush(hdc);
	}

	// O 마커 그리기
	void DrawCircleMarker(HDC hdc, int row, int col)
	{
		ChangePen(hdc, circlePen);
		ChangeBrush(hdc, circleBrush);

		Ellipse(
			hdc,
			markerRect[row][col].left,
			markerRect[row][col].top,
			markerRect[row][col].right,
			markerRect[row][col].bottom
		);

		EndPen(hdc);
		EndBrush(hdc);
	}

	// 마커 그리기
	void DrawMarkers(HDC hdc)
	{
		for (int row = 0; row < 3; row++)
		{
			for (int col = 0; col < 3; col++)
			{
				switch (marker[row][col])	// 마커의 종류에 따라서 별개의 그리기 처리
				{
				case Marker::CROSS: DrawCrossMarker(hdc, row, col); break;
				case Marker::CIRCLE: DrawCircleMarker(hdc, row, col); break;
				}
			}
		}
	}

	// 안내 텍스트 그리기
	void DrawTip(HDC hdc)
	{
		DrawText(hdc, nowText, -1, &textRect, DT_LEFT);	// 현재 텍스트 출력
	}

public:
	// 생성자
	Board(POINT _startPosition, int _cellSize) :
		startPosition(_startPosition),		// 시작위치 상수 초기화
		cellSize(_cellSize),				// 칸 크기 상수 초기화
		markerPadding(_cellSize * 0.1)		// 칸과 마커 사이의 여유공간 상수 초기화
	{										// 칸 크기의 10%를 상하좌우에 여유공간으로 사용 => 실제 마커 사이즈 == 칸 크기의 80%
		crossPen = NULL;
		circlePen = NULL;
		oldPen = NULL;

		crossBrush = NULL;
		circleBrush = NULL;
		oldBrush = NULL;

		textRect = {				// 텍스트 출력 범위 적당히 지정
			startPosition.x,
			startPosition.y - 30,
			500,
			startPosition.y 
		};	
		nowText = basicToolTip;		// 기본 텍스트 설정												

		ResetMarker();
		CalMarkerRect();
		CalCellRect();
	}

	void SetCrossPen(HPEN hPen) { crossPen = hPen; }	// X 마커 펜 지정
	void SetCirclePen(HPEN hPen) { circlePen = hPen; }	// O 마커 펜 지정

	void SetCrossBrush(HBRUSH hBrush) { crossBrush = hBrush; }		// X 마커 브러쉬 지정
	void SetCircleBrush(HBRUSH hBrush) { circleBrush = hBrush; }	// O 마커 브러쉬 지정

	// 지정된 위치에 마커 작성. 성공 시 true 반환, 실패 시 false 반환
	bool SetMarker(POINT position, Marker markerType)
	{
		if (marker[position.x][position.y] != Marker::NONE) return false;	// 만약 빈칸이 아니라면 false 반환

		marker[position.x][position.y] = markerType;	// 해당 위치에 마커 작성

		return true;	// 성공 시 ture 반환
	}

	void SetTextToBasic() { nowText = basicToolTip; }			// 텍스트를 기본 텍스트로 변경
	void SetTextToCrossWin() { nowText = crossWinText; }		// 텍스트를 X 승리 텍스트로 변경
	void SetTextToCircleWin() { nowText = circleWinText; }		// 텍스트를 O 승리 텍스트로 변경
	void SetTextToDraw() { nowText = drawText; }				// 텍스트를 무승부 텍스트로 변경

	// 승리 체크
	Marker CheckWinner()
	{
		// 가로 체크
		for (int row = 0; row < 3; row++)
			if (Marker::NONE != marker[row][0] &&
				marker[row][0] == marker[row][1] &&
				marker[row][1] == marker[row][2])
				return marker[row][2];

		// 세로 체크
		for (int col = 0; col < 3; col++)
			if (Marker::NONE != marker[0][col] &&
				marker[0][col] == marker[1][col] &&
				marker[1][col] == marker[2][col])
				return marker[2][col];

		// 우하향 대각선 체크
		if (Marker::NONE != marker[0][0] &&
			marker[0][0] == marker[1][1] &&
			marker[1][1] == marker[2][2])
			return marker[2][2];

		// 좌하향 대각선 체크
		if (Marker::NONE != marker[0][2] &&
			marker[0][2] == marker[1][1] &&
			marker[1][1] == marker[2][0])
			return marker[2][0];

		// 승리자 없음
		return Marker::NONE;
	}

	// 무승부 체크, 무승부 시 true 반환
	bool CheckDraw()
	{
		for (int row = 0; row < 3; row++)
		{
			for (int col = 0; col < 3; col++)
			{
				if (marker[row][col] == Marker::NONE)
					return false;
			}
		}

		return true;
	}

	// 마커 초기화
	void ResetMarker()
	{
		for (int row = 0; row < 3; row++)
		{
			for (int col = 0; col < 3; col++)
			{
				marker[row][col] = Marker::NONE;
			}
		}
	}

	// 보드 그리기
	void Draw(HDC hdc)
	{
		DrawTip(hdc);
		DrawBoard(hdc);
		DrawMarkers(hdc);
	}
};

const LPCTSTR Board::basicToolTip = TEXT("이동: 방향키, 입력: 엔터, 종료: ESC");		// 기본 텍스트
const LPCTSTR Board::crossWinText = TEXT("X가 승리했습니다. 재시작:R, 종료: ESC");		// X 승리 텍스트
const LPCTSTR Board::circleWinText = TEXT("O가 승리했습니다. 재시작:R, 종료: ESC");		// O 승리 텍스트
const LPCTSTR Board::drawText = TEXT("무승부입니다. 재시작:R, 종료: ESC");				// 무승부 텍스트

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
	static bool isGameOver = false;						// 게임 종료 여부

	switch (iMessage)
	{
	case WM_CREATE:
		redPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));	// 빨간펜 생성
		greenPen = CreatePen(PS_SOLID, 5, RGB(0, 255, 0));	// 초록펜 생성

		redBrush = CreateSolidBrush(RGB(255, 0, 0));		// 빨간 브러쉬 생성
		greenBrush = CreateSolidBrush(RGB(0, 255, 0));		// 초록 브러쉬 생성

		board.SetCrossPen(redPen);		// X 마커 펜 지정
		board.SetCrossBrush(redBrush);	// X 마커 브러쉬 지정

		board.SetCirclePen(greenPen);			// O 마커 펜 지정
		board.SetCircleBrush(greenBrush);		// O 마커 브러쉬 지정

		nowPen = redPen;		// 선 플레이어의 펜을 빨간 펜으로 지정
		return 0;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT: cursor.MoveLeft(); break;		// 커서 왼쪽으로 이동
		case VK_UP: cursor.MoveUp(); break;			// 커서 위쪽으로 이동
		case VK_RIGHT: cursor.MoveRight(); break;	// 커서 오른쪽으로 이동
		case VK_DOWN: cursor.MoveDown(); break;		// 커서 아래쪽으로 이동
		case VK_RETURN:		// Enter 입력 시
			if (isGameOver) break;	// 게임 종료 시 입력 무시

			if (board.SetMarker(cursor.GetPos(), (isFirstPlayer ? Marker::CROSS : Marker::CIRCLE)))		// 커서 위치에 마커 입력
			{
				// 마커 입력 성공 시
				isFirstPlayer = !isFirstPlayer;					// 유저 변경
				nowPen = (isFirstPlayer ? redPen : greenPen);	// 펜 변경
				cursor.ResetPos();								// 커서 위치 초기화

				// 승자 확인
				switch (board.CheckWinner())
				{
				case Marker::CROSS:				// X 승리 시
					isGameOver = true;			// 게임 종료
					board.SetTextToCrossWin();	// X 승리 텍스트 출력
					break;
				case Marker::CIRCLE:			// O 승리 시
					isGameOver = true;			// 게임 종료
					board.SetTextToCircleWin();	// O 승리 텍스트 출력
					break;
				}

				// 무승부 확인
				if (board.CheckDraw())
				{							// 무승부라면
					isGameOver = true;		// 게임 종료
					board.SetTextToDraw();	// 무승부 텍스트 출력
				}
			}
			break;
		case VK_ESCAPE: DestroyWindow(hWnd); break;	// 게임(프로그램) 종료
		}

		InvalidateRect(hWnd, NULL, TRUE);
		return 0;

	case WM_CHAR:
		switch ((TCHAR)wParam)
		{
		case 'R':
		case 'r':
			if (isGameOver)	// 게임 오버된 상태에서 R 입력 시
			{
				board.ResetMarker();	// 마커 초기화
				board.SetTextToBasic();	// 텍스트 초기화

				cursor.ResetPos();	// 커서 초기화

				isFirstPlayer = true;	// 시작 플레이어의 턴으로 변경
				nowPen = redPen;		// 시작 플레이어의 펜으로 변경

				isGameOver = false;	// 게임 오버 상태 해제
			}
			break;
		}

		InvalidateRect(hWnd, NULL, TRUE);
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		board.Draw(hdc);	// 보드 그리기

		if (!isGameOver)				// 게임이 끝나지 않았다면
			cursor.Draw(hdc, nowPen);	// 커서 그리기

		EndPaint(hWnd, &ps);
		return 0;

	case WM_DESTROY:
		DeleteObject(redPen);		// 빨간펜 삭제
		DeleteObject(greenPen);		// 초록펜 삭제

		DeleteObject(redBrush);		// 빨간 브러쉬 삭제
		DeleteObject(greenBrush);	// 초록 브러쉬 삭제

		PostQuitMessage(0);
		return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

