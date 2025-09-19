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

// ��Ŀ ������, ��Ŀ ���� NONE(0):��ĭ / CROSS(1):X / CIRCLE(2):O
enum Marker { NONE, CROSS, CIRCLE };

// 3 * 3 ���� ���� �� ���� ��Ŀ ���� ����
class Board
{
private:
	// ��� ��� �ʵ�
	const POINT startPosition;	// ���� �׸��� ��ġ
	const int cellSize;			// ĭ ũ��
	const int markerSize;		// ��Ŀ ũ��
	const int markerPadding;	// ĭ�� ��Ŀ ���� ����

	// ��� ���� �ʵ�
	RECT cellRect[3][3];	// ���� ĭ RECT ��ǥ �����
	Marker marker[3][3];	// ��Ŀ ���� ����
	RECT markerRect[3][3];	// ��Ŀ�� �׷��� RECT ��ǥ �����
	HPEN crossPen;			// X ��Ŀ �׸� ��
	HPEN circlePen;			// O ��Ŀ �׸� ��
	HPEN oldPen;			// �� ��ȯ�� ����
	HBRUSH crossBrush;		// X ��Ŀ �׸� �귯��
	HBRUSH circleBrush;		// O ��Ŀ �׸� �귯��
	HBRUSH oldBrush;		// �귯�� ��ȯ�� ����

	// ��� �Լ�
	// ��Ŀ �ʱ�ȭ
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

	// ���� ĭ RECT ���
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

	// ��Ŀ RECT ���
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

	// 3*3 ���� �׸���
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

	// �� ����
	void ChangePen(HDC hdc, HPEN hPen)
	{
		if (hPen != NULL)							// ������ ���� NULL�� �ƴ϶��
			oldPen = (HPEN)SelectObject(hdc, hPen);	// ������ ������ ����
	}

	// �� ��� ����
	void EndPen(HDC hdc)
	{
		if (oldPen != NULL)				// oldPen�� NULL�� �ƴ϶��
		{
			SelectObject(hdc, oldPen);	// ���� ������ ����
			oldPen = NULL;				// �߸� �� ���� ������ ���� �ʱ�ȭ
		}
	}

	// �귯�� ����
	void ChangeBrush(HDC hdc, HBRUSH hBrush)
	{
		if (hBrush != NULL)									// ������ �귯���� NULL�� �ƴ϶��
			oldBrush = (HBRUSH)SelectObject(hdc, hBrush);	// ������ �귯���� ����
	}

	// �귯�� ��� ����
	void EndBrush(HDC hdc)
	{
		if (oldBrush != NULL)				// oldBrush�� NULL�� �ƴ϶��
		{
			SelectObject(hdc, oldBrush);	// ���� �귯���� ����
			oldBrush = NULL;				// �߸��� ���� ������ ���� �ʱ�ȭ
		}
	}

	// X ��Ŀ �׸���
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

	// O ��Ŀ �׸���
	void DrawCircleMarker(HDC hdc, int col, int row)
	{
		ChangePen(hdc, circlePen);
		ChangeBrush(hdc, circleBrush);

		Ellipse(hdc, markerRect[col][row].left, markerRect[col][row].top, markerRect[col][row].right, markerRect[col][row].bottom);

		EndPen(hdc);
		EndBrush(hdc);
	}

	// ��Ŀ �׸���
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

	// �ȳ� �ؽ�Ʈ �׸���
	void DrawTip(HDC hdc)
	{
		RECT rt = { startPosition.x,startPosition.y - 30,500,startPosition.y };	// �ؽ�Ʈ ��� ���� ����
		DrawText(hdc, TEXT("�̵�: ����Ű, �Է�: ����, ����: ESC"), -1, &rt, DT_LEFT);	// �ȳ� �ؽ�Ʈ ���
	}

public:
	// ������
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

	void SetCrossPen(HPEN hPen) { crossPen = hPen; }	// X ��Ŀ �� ����
	void SetCirclePen(HPEN hPen) { circlePen = hPen; }	// O ��Ŀ �� ����
	void SetCrossBrush(HBRUSH hBrush) { crossBrush = hBrush; }	// X ��Ŀ �귯�� ����
	void SetCircleBrush(HBRUSH hBrush) { circleBrush = hBrush; }	// O ��Ŀ �귯�� ����

	// ������ ��ġ�� ��Ŀ �ۼ�. ���� �� true ��ȯ, ���� �� false ��ȯ
	bool SetMarker(POINT position, Marker markerType)
	{
		if (marker[position.y][position.x] != Marker::NONE) return false;	// ���� ��ĭ�� �ƴ϶�� false ��ȯ

		marker[position.y][position.x] = markerType;	// �ش� ��ġ�� ��Ŀ �ۼ�

		return true;	// ���� �� ture ��ȯ
	}

	// ���� �׸���
	void Draw(HDC hdc)
	{
		DrawTip(hdc);
		DrawBoard(hdc);
		DrawMarkers(hdc);
	}
};

// Ŀ�� ���� ���� �� ������ ���� Ŭ����
class Cursor
{
private:
	// ���� ��� �ʵ�
	static const int MINPOS = 0;		// Ŀ�� ��ġ �ּڰ�
	static const int MAXPOS = 2;		// Ŀ�� ��ġ �ִ�

	// ��� ��� �ʵ�
	const POINT boardPosition;	// ���� ���� ��ġ
	const int cursorSize;		// Ŀ�� ũ��

	// ���� �ʵ�
	POINT position;		// Ŀ�� ��ġ(0~2, 0~2) ����
	RECT cursorRect;	// Ŀ�� ��ǥ(�ȼ�) ����

	// �Լ� �ʵ�
	// ���� [MINPOS, MAXPOS]�� ������ ����ٸ� true
	static bool OutRange(int v) { return MINPOS > v || v > MAXPOS; }

	// cursorRect �� ������
	void CalRect()
	{
		cursorRect = {
			position.x * cursorSize + boardPosition.x,			// Left
			position.y * cursorSize + boardPosition.y,			// Top
			(position.x + 1) * cursorSize + boardPosition.x,	// Right
			(position.y + 1) * cursorSize + boardPosition.y		// Bottom
		};
	}

	// ������ ��ǥ�� Ŀ�� �̵�
	void SetCursorPos(int x, int y)
	{
		position = { x,y };	// position�� x,y������ ����

		CalRect();	// cursorRect �� ����
	}

	/// <summary>
	/// �Է��� ��ǥ��ŭ Ŀ�� �̵�
	/// </summary>
	/// <param name="dx">�̵��� x��</param>
	/// <param name="dy">�̵��� y��</param>
	void MoveCursor(int dx, int dy)
	{
		int nx = position.x + dx;	// �̵� �� x ��ǥ
		int ny = position.y + dy;	// �̵� �� y ��ǥ

		if (OutRange(nx) || OutRange(ny)) return;	// �̵��� ��ǥ�� ������ ��� ��� �Լ� ����

		SetCursorPos(nx, ny);	// Ŀ���� (nx, ny)�� �̵�
	}

public:
	// ������
	Cursor(POINT _boardPosition, int _cursorSize) :boardPosition(_boardPosition), cursorSize(_cursorSize)
	{
		position = { 0,0 };
		CalRect();
	}

	POINT GetPos() { return position; }		// Ŀ���� ��ǥ(0~2,0~2) ��ȯ

	void MoveLeft() { MoveCursor(-1, 0); }	// Ŀ���� x�� -1 ��ŭ �̵�
	void MoveUp() { MoveCursor(0, -1); }	// Ŀ���� y�� -1 ��ŭ �̵�
	void MoveRight() { MoveCursor(1, 0); }	// Ŀ���� x�� +1 ��ŭ �̵�
	void MoveDown() { MoveCursor(0, 1); }	// Ŀ���� y�� +1 ��ŭ �̵�

	void ResetPos() { SetCursorPos(0, 0); }	// Ŀ���� ��ġ�� (0, 0)���� �ʱ�ȭ

	// Ŀ�� �׸���
	void Draw(HDC hdc, HPEN pen = NULL)
	{
		HPEN oldPen = NULL;		// ���� �� ���� �����

		if (pen != NULL)							// ������ ���� �ִٸ�
			oldPen = (HPEN)SelectObject(hdc, pen);	// ������ ������ ����

		MoveToEx(hdc, cursorRect.left, cursorRect.top, NULL);	// Ŀ�� ���� ��ġ�� �̵�
		LineTo(hdc, cursorRect.right, cursorRect.top);			// ��� ���� �׸���
		LineTo(hdc, cursorRect.right, cursorRect.bottom);		// ���� ���� �׸���
		LineTo(hdc, cursorRect.left, cursorRect.bottom);		// �ϴ� ���� �׸���
		LineTo(hdc, cursorRect.left, cursorRect.top);			// ���� ���� �׸���

		if (pen != NULL)				// ������ ���� �ִٸ�
			SelectObject(hdc, oldPen);	// ���� ������ ����
	}
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static const int CELL_SIZE = 100;					// ĭ ũ�� ���
	static const POINT START_POSITION = { 50,50 };		// ���� ���� ��ġ ���

	static HDC hdc;										// Handle of Device Context
	static PAINTSTRUCT ps;								// Paintstruct

	static HPEN nowPen, redPen, greenPen;				// Ŀ���� �׸� �� ����� ��(���� ������� ��, ����, �ʷ�)
	static HBRUSH redBrush, greenBrush;					// ����� �귯��

	static Board board(START_POSITION, CELL_SIZE);		// ���� ����
	static Cursor cursor(START_POSITION, CELL_SIZE);	// Ŀ�� ����

	static bool isFirstPlayer = true;					// ���� �÷��̾� ����

	switch (iMessage)
	{
	case WM_CREATE:
		redPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));	// ������ ����
		greenPen = CreatePen(PS_SOLID, 5, RGB(0, 255, 0));	// �ʷ��� ����

		redBrush = CreateSolidBrush(RGB(255, 0, 0));		// ���� �귯�� ����
		greenBrush = CreateSolidBrush(RGB(0, 255, 0));		// �ʷ� �귯�� ����

		board.SetCrossPen(greenPen);		// X ��Ŀ �� ����
		board.SetCrossBrush(greenBrush);	// X ��Ŀ �귯�� ����
		board.SetCirclePen(redPen);			// O ��Ŀ �� ����
		board.SetCircleBrush(redBrush);		// O ��Ŀ �귯�� ����

		nowPen = greenPen;		// �ʷ� �÷��̾ ���̱� ������ ���� ���� �ʷ� ������ ����
		return 0;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT: cursor.MoveLeft(); break;		// Ŀ�� �������� �̵�
		case VK_UP: cursor.MoveUp(); break;			// Ŀ�� �������� �̵�
		case VK_RIGHT: cursor.MoveRight(); break;	// Ŀ�� ���������� �̵�
		case VK_DOWN: cursor.MoveDown(); break;		// Ŀ�� �Ʒ������� �̵�
		case VK_RETURN:		// Enter �Է� ��
			if (board.SetMarker(cursor.GetPos(), (isFirstPlayer ? Marker::CROSS : Marker::CIRCLE)))		// Ŀ�� ��ġ�� ��Ŀ �Է�
			{
				// ��Ŀ �Է� ���� ��
				isFirstPlayer = !isFirstPlayer;					// ���� ����
				nowPen = (isFirstPlayer ? greenPen : redPen);	// �� ����
				cursor.ResetPos();								// Ŀ�� ��ġ �ʱ�ȭ
			}
			break;
		case VK_ESCAPE: DestroyWindow(hWnd); break;	// ����(���α׷�) ����
		}

		InvalidateRect(hWnd, NULL, TRUE);
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		board.Draw(hdc);	// ���� �׸���

		cursor.Draw(hdc, nowPen);	// Ŀ�� �׸���

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

