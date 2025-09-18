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

// 3*3 ���� �׸���
void DrawBoard(HDC hdc, int x, int y, int CellSize)
{
	Rectangle(hdc, x, y, CellSize * 3 + x, CellSize * 3 + y);				// �ܰ� ����
	Rectangle(hdc, x + CellSize, y, CellSize * 2 + x, CellSize * 3 + y);	// ���� ����
	MoveToEx(hdc, x, y + CellSize, NULL);									// ��� ���� ����
	LineTo(hdc, CellSize * 3 + x, y + CellSize);							// ��� ���� �׸���
	MoveToEx(hdc, x, y + CellSize * 2, NULL);								// �ϴ� ���� ����
	LineTo(hdc, CellSize * 3 + x, y + CellSize * 2);						// �ϴ� ���� �׸���

	RECT rt = { x,y - 30,500,y };	// �ؽ�Ʈ ��� ���� ����
	DrawText(hdc, TEXT("�̵�: ����Ű, �Է�: ����, ����: ESC"), -1, &rt, DT_LEFT);	// �ȳ� �ؽ�Ʈ ���

}

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
	POINT position;		// Ŀ�� ��ġ ����
	RECT cursorRect;	// Ŀ�� ��ǥ ����

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
	static const int CELL_SIZE = 100;
	static const POINT START_POSITION = { 50,50 };

	static HDC hdc;										// Handle of Device Context
	static PAINTSTRUCT ps;								// Paintstruct
	static HPEN nowPen, redPen, greenPen;				// Ŀ���� �׸� �� ����� ��(���� ������� ��, ����, �ʷ�)
	static HBRUSH myBrush, oldBrush;					// ����� �귯��
	static Cursor cursor(START_POSITION, CELL_SIZE);	// Ŀ�� ����

	switch (iMessage)
	{
	case WM_CREATE:
		greenPen = CreatePen(PS_SOLID, 5, RGB(0, 255, 0));	// �ʷ��� ����
		redPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));	// ������ ����
		nowPen = greenPen;		// �ʷ� �÷��̾ ���̱� ������ ���� ���� �ʷ� ������ ����
		return 0;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT: cursor.MoveLeft(); break;		// Ŀ�� �������� �̵�
		case VK_UP: cursor.MoveUp(); break;			// Ŀ�� �������� �̵�
		case VK_RIGHT: cursor.MoveRight(); break;	// Ŀ�� ���������� �̵�
		case VK_DOWN: cursor.MoveDown(); break;		// Ŀ�� �Ʒ������� �̵�
		case VK_ESCAPE:DestroyWindow(hWnd); break;	// ����(���α׷�) ����
		}

		InvalidateRect(hWnd, NULL, TRUE);
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		DrawBoard(hdc, START_POSITION.x, START_POSITION.y, CELL_SIZE);	// ���� �׸���

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

