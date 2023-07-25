#pragma once
#include "Libraries.h"
#include "resource.h"

constexpr int WND_WIDTH = 500;
constexpr int WND_HEIGHT = 500;
constexpr int STEPSIZE = 20;
constexpr int OBJ_SIZE = 20;
constexpr int MAXSNAKESIZE = 19*19;
constexpr auto UPDATE_TIME = std::chrono::milliseconds(100);

#define SubMenuCommand1 1
#define SubMenuCommand2 2
#define SubMenuCommand3 3
#define RootMenuClicked 4
#define ScoreCountWindow 5

constexpr int DIR_LEFT = 1;
constexpr int DIR_UP = 2;
constexpr int DIR_RIGHT = 3;
constexpr int DIR_DOWN = 4;

std::mutex mtx;

template<typename T> bool hasDuplicate(const std::vector<T>& v)
{
	for (size_t i = 0; i < v.size(); ++i)
		for (size_t j = i + 1; j < v.size(); ++j)
			if (v[i].top == v[j].top && v[i].left == v[j].left && v[i].right == v[j].right && v[i].bottom == v[j].bottom) {
				return true;
			}
	return false;
}

int RoundUpToX(int ValueToRound, int ValueRoundTo) {
	int RoundedValue = ValueToRound;

	if (ValueToRound % ValueRoundTo != 0) {
		RoundedValue = ValueToRound - (ValueToRound % ValueRoundTo) + ValueRoundTo;
	}

	return RoundedValue;
}

int GetRandomValue(const int Start, const int End) {

	using namespace std;

	random_device rd;   // non-deterministic generator
	mt19937 gen(rd());  // to seed mersenne twister.
	uniform_int_distribution<> dist(Start, End); // distribute results between 1 and 6 inclusive.

	int GeneratedValue = RoundUpToX(dist(gen), STEPSIZE);
	return GeneratedValue;

}

template <class T>
void SafeRelease(T** ptr) {
	if (*ptr) {
		(*ptr)->Release();
		*ptr = NULL;
	}
}

template <class Window>
class App {
public:
	App() : m_hWnd(NULL) {
		msg = {};
	};


	int Run() {
		try {
			MakeWindow();
			ShowWindow(this->m_hWnd, SW_NORMAL);
			UpdateWindow(this->m_hWnd);
		}
		catch (std::exception& ex) {
			MessageBox(m_hWnd, reinterpret_cast<const wchar_t*>(ex.what()), L"Error", MB_OK);
		}
		while (GetMessage(&msg, m_hWnd, NULL, NULL) > 0) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		return 0;
	}

	HWND GetWindow() {
		return m_hWnd;
	}


private:
	static LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		Window* pThis = nullptr;
		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* CS = reinterpret_cast<CREATESTRUCT*>(lParam);
			pThis = reinterpret_cast<Window*>(CS->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
			pThis->m_hWnd = hWnd;
		}
		else {
			pThis = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		}
		if (pThis) {
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}
		else {
			return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		}
	}

	BOOL MakeWindow() {

		WNDCLASS wc = {};
		wc.hInstance = GetModuleHandleW(NULL);
		wc.lpfnWndProc = App::WndProc;
		wc.lpszClassName = ClassName();
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_MYICON));

		if (!RegisterClass(&wc)) {
			throw std::runtime_error("Error, can't register main window class!");
		}


		RECT rc = { 0, 0, WND_WIDTH, WND_HEIGHT };
		if (!AdjustWindowRect(&rc, WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, true)) {
			throw std::runtime_error("Error, couldn't AdjustWindowRect");
		}

		m_hWnd = CreateWindowW(ClassName(), WndName(), WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, 0, 0, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, GetModuleHandle(NULL), this);

		return(m_hWnd ? true : false);
	}

	virtual LPCWSTR ClassName() = 0;
	virtual LPCWSTR WndName() = 0;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

protected:
	MSG msg;
	HWND m_hWnd;
};


class BaseWindow : public App<BaseWindow> {
public:
	BaseWindow() : pFactory(NULL), pBrush(NULL), pBrush2(NULL), pRenderTarget(NULL), isThreading(true) {
		Snake.push_back({ 240 , 240 , 240 + OBJ_SIZE  , 240 + OBJ_SIZE });
		SnakeArea = { NULL, NULL, NULL, NULL };
		SnakeAreaBorder = { NULL, NULL, NULL, NULL };
		Score = 0;
		x = 0;
		y = 0;
		SetApplePosition();
		ScoreText = std::to_wstring(Score);
		Direction = NULL;
	}

	LPCWSTR ClassName() override { return L"SnakeWindowClass"; }
	LPCWSTR WndName() override { return L"Игра Змейка"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	void DiscardGraphicsResources();
	int GetScore() { return Score; }

protected:

	//variables 
	D2D1_COLOR_F color;
	D2D1_COLOR_F color2;
	D2D1_COLOR_F color3;
	D2D1_COLOR_F SnakeColor;
	D2D1_SIZE_U size;
	D2D1_SIZE_F sizef;
	PAINTSTRUCT ps;
	float x;
	float y;


	// Graphics
	ID2D1Factory* pFactory;
	ID2D1HwndRenderTarget* pRenderTarget;
	ID2D1SolidColorBrush* pBrush;
	ID2D1SolidColorBrush* pBrush2;
	CComPtr<ID2D1SolidColorBrush> pBrushSnake;
	CComPtr<ID2D1SolidColorBrush> pBrushAppleColor;
	
	D2D1_RECT_F rectF;
	void CalculateLayout();
	HRESULT CreateGraphicsResources();
	void OnPaint();
	void Resize();
	RECT CurrentClientRect;

	//ScoreText
	HRESULT CreateTextResources();
	CComPtr<IDWriteTextFormat> pTextFormat;
	CComPtr<IDWriteFactory> pDWriteFactory;
	D2D1_RECT_F LayoutRect = { 100, 10, 400, 30 };
	std::wstring ScoreText;
	

	//Snake
	D2D1_RECT_F NewSnakeElement;
	D2D1_RECT_F SnakeArea;
	D2D1_RECT_F SnakeAreaBorder;
	D2D1_RECT_F PreviousHeadState;
	D2D1_RECT_F Buff;
	std::vector<D2D1_RECT_F> Snake;
	void MoveSnake(int& Direction);
	void MoveSnakeAround(const int& Direction);
	
	//Apple
	D2D1_ROUNDED_RECT Apple;
	void SetApplePosition();

	//Menu
	BOOL MainWindowSetUpMenu();
	int Score;
	
	//Thread (time counter)
	int Direction;
	bool isThreading;
	bool hasLost = false;
	time_t start;
	std::thread thread;
};

void BaseWindow::DiscardGraphicsResources()
{
	SafeRelease(&pRenderTarget);
	SafeRelease(&pBrush);
	SafeRelease(&pBrush2);
	Snake.~vector();
	ScoreText.~basic_string();
}

void BaseWindow::CalculateLayout() {
	if (pRenderTarget != NULL)
	{
		sizef = pRenderTarget->GetSize();
		x = (float)size.width;
		y = (float)size.height;
		//SCALING_FACTOR = (x / Screenwidth + y / Screenheight)/2;
		rectF = { 0, 0, x, y };
		SnakeArea = { (float)CurrentClientRect.left + 60 , (float)CurrentClientRect.top + 60 , (float)CurrentClientRect.right - 60, (float)CurrentClientRect.bottom - 60 };
		SnakeAreaBorder = { (float)CurrentClientRect.left + 50 , (float)CurrentClientRect.top + 50 , (float)CurrentClientRect.right - 50, (float)CurrentClientRect.bottom - 50 };
		//SnakeElement.left *= SCALING_FACTOR; SnakeElement.top *= SCALING_FACTOR; 
	}
}

HRESULT BaseWindow::CreateGraphicsResources()
{
	HRESULT hr = S_OK;
	if (pRenderTarget == NULL)
	{
		GetClientRect(m_hWnd, &CurrentClientRect);

		size = D2D1::SizeU(CurrentClientRect.right, CurrentClientRect.bottom);

		hr = pFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hWnd, size),
			&pRenderTarget);

		if (SUCCEEDED(hr))
		{
			color = D2D1::ColorF(1.0f, 0.83f, .616f);
			hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);
			if (SUCCEEDED(hr)) {
				color2 = D2D1::ColorF(0.0f, 0.0f, 0.2f);
				hr = pRenderTarget->CreateSolidColorBrush(color2, &pBrush2);
				if (SUCCEEDED(hr)) {
					color3 = D2D1::ColorF(0.807f, 0.f, 0.f);
					hr = pRenderTarget->CreateSolidColorBrush(color3, &pBrushAppleColor);
					if (SUCCEEDED(hr)) {
						SnakeColor = D2D1::ColorF(0.518f, 0.62f, 0.f);
						hr = pRenderTarget->CreateSolidColorBrush(SnakeColor, &pBrushSnake);
					}
					CalculateLayout();
				}
			}
		}
	}
	return hr;
}

void BaseWindow::OnPaint()
{
	HRESULT hr = CreateGraphicsResources();
	if (SUCCEEDED(hr))
	{
		BeginPaint(m_hWnd, &ps);
		
		pRenderTarget->BeginDraw();
		// PAINT START 

		pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));
		pRenderTarget->FillRectangle(rectF, pBrush);
		pRenderTarget->FillRectangle(SnakeAreaBorder, pBrush2);
		pRenderTarget->FillRectangle(SnakeArea, pBrush);

		for (std::vector<D2D1_RECT_F>::const_iterator it = Snake.cbegin(); it < Snake.cend(); it++) {
			pRenderTarget->FillRectangle(*it, pBrushSnake);
		}
		//pRenderTarget->SetTransform(D2D1::Matrix3x2F::Scale(D2D1::Size(0.7f, 0.7f), D2D1::Point2F((Apple.rect.right + Apple.rect.left) / 2, (Apple.rect.bottom + Apple.rect.top) / 2)));
		pRenderTarget->FillRoundedRectangle(Apple, pBrushAppleColor);
		pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		pRenderTarget->DrawTextW(ScoreText.c_str(), wcslen(ScoreText.c_str()), pTextFormat, LayoutRect, pBrushAppleColor);
		
		// PAINT END 
		hr = pRenderTarget->EndDraw();
		if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
		{
			DiscardGraphicsResources();
		}
		EndPaint(m_hWnd, &ps);
	}
}

void BaseWindow::Resize()
{
	if (pRenderTarget != NULL)
	{

		GetClientRect(m_hWnd, &CurrentClientRect);

		size = D2D1::SizeU(CurrentClientRect.right, CurrentClientRect.bottom);

		pRenderTarget->Resize(size);

		CalculateLayout();

		InvalidateRect(m_hWnd, &CurrentClientRect, FALSE);
	}
}

BOOL BaseWindow::MainWindowSetUpMenu()
{
	HMENU RootMenu = CreateMenu();
	HMENU SubMenu = CreateMenu();

	AppendMenuW(SubMenu, MF_STRING, SubMenuCommand1, L"Переместить яблоко");
	AppendMenuW(SubMenu, MF_STRING, SubMenuCommand2, L"Перезагрузка");
	AppendMenuW(SubMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenuW(SubMenu, MF_STRING, SubMenuCommand3, L"Выйти");

	AppendMenuW(RootMenu, MF_POPUP, (UINT_PTR)SubMenu, L"Игра");

	return(SetMenu(m_hWnd, RootMenu));
}

void BaseWindow::SetApplePosition() {
	if (Snake.size() < MAXSNAKESIZE) {
		float* RandomNum1 = new float(GetRandomValue(60, 420));
		float* RandomNum2 = new float(GetRandomValue(60, 420));

		bool IsInsideSnake = false;
		do {
			IsInsideSnake = false;
			for (auto SnakePart : Snake)
			{
				if (*RandomNum1 == SnakePart.left && *RandomNum2 == SnakePart.top)
				{

					*RandomNum1 = float(GetRandomValue(60, 420));
					*RandomNum2 = float(GetRandomValue(60, 420));
					IsInsideSnake = true;
				}
			}
		} while (IsInsideSnake);


		Apple = { {*RandomNum1,*RandomNum2,*RandomNum1 + OBJ_SIZE,*RandomNum2 + OBJ_SIZE} , 10, 8.5 };

		delete RandomNum1;
		delete RandomNum2;
	}
}

HRESULT BaseWindow::CreateTextResources() {

	HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(pDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));

	if (SUCCEEDED(hr))
	{
		hr = pDWriteFactory->CreateTextFormat(
			L"Myriad Pro",
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			30.0f,
			L"en-us",
			&pTextFormat
		);
	}
	if (SUCCEEDED(hr))
	{
		hr = pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	}
	if (SUCCEEDED(hr)) {
		hr = pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}

	return hr;
}

void BaseWindow::MoveSnake(int& Direction)
{
	while (isThreading)
	{
		switch (Direction)
		{
			case DIR_LEFT:
			{
				if (Snake.at(0).left == SnakeArea.left) { // Выход за поле слева
					isThreading = 0;
					MessageBox(m_hWnd, L"Игра окончена!", L"Конец", MB_OK);
				}

				if (Snake.at(0).left - STEPSIZE >= SnakeArea.left) { 
					PreviousHeadState = Snake.at(0);
					Snake.at(0).left -= STEPSIZE;
					Snake.at(0).right -= STEPSIZE;
					for (auto it = Snake.begin() + 1; it != Snake.end(); it++) {
						Buff = *it;
						*it = PreviousHeadState;
						PreviousHeadState = Buff;
					}
					SendMessageW(m_hWnd, WM_PAINT, NULL, NULL);
				}

				if (hasDuplicate(Snake)) { // Врезался в себя
					isThreading = 0;
					MessageBox(m_hWnd, L"Игра окончена!", L"Конец", MB_OK);
				}
				break;
			}
			case DIR_UP:
			{
				if (Snake.at(0).top == SnakeArea.top) { //Выход за поле сверху
					isThreading = 0;
					MessageBox(m_hWnd, L"Игра окончена!", L"Конец", MB_OK);
				}

				if (Snake.at(0).top - STEPSIZE >= SnakeArea.top) {
					PreviousHeadState = Snake.at(0);
					Snake.at(0).top -= STEPSIZE;
					Snake.at(0).bottom -= STEPSIZE;
					for (auto it = Snake.begin() + 1; it != Snake.end(); it++) {
						Buff = *it;
						*it = PreviousHeadState;
						PreviousHeadState = Buff;
					}
					SendMessageW(m_hWnd, WM_PAINT, NULL, NULL);
				}

				if (hasDuplicate(Snake)) { // Врезался в себя
					isThreading = 0;
					MessageBox(m_hWnd, L"Игра окончена!", L"Конец", MB_OK);
				}

				break;
			}
			case DIR_RIGHT:
			{
				if (Snake.at(0).right == SnakeArea.right) { //Выход за поле справа
					isThreading = 0;
					MessageBox(m_hWnd, L"Игра окончена!", L"Конец", MB_OK);
				}
				
				if (Snake.at(0).right + STEPSIZE <= SnakeArea.right) {
					PreviousHeadState = Snake.at(0);
					Snake.at(0).left += STEPSIZE;
					Snake.at(0).right += STEPSIZE;
					for (auto it = Snake.begin() + 1; it != Snake.end(); it++) {
						Buff = *it;
						*it = PreviousHeadState;
						PreviousHeadState = Buff;
					}
					SendMessageW(m_hWnd, WM_PAINT, NULL, NULL);
				}

				if (hasDuplicate(Snake)) { // Врезался в себя
					isThreading = 0;
					MessageBox(m_hWnd, L"Игра окончена!", L"Конец", MB_OK);
				}

				break;
			}
			case DIR_DOWN:
			{
				if (Snake.at(0).bottom == SnakeArea.bottom) { //Выход за поле снизу
					isThreading = 0;
					MessageBox(m_hWnd, L"Игра окончена!", L"Конец", MB_OK);
				}

				if (Snake.at(0).bottom + STEPSIZE <= SnakeArea.bottom) {
					PreviousHeadState = Snake.at(0);
					Snake.at(0).top += STEPSIZE;
					Snake.at(0).bottom += STEPSIZE;
					for (auto it = Snake.begin() + 1; it != Snake.end(); it++) {
						Buff = *it;
						*it = PreviousHeadState;
						PreviousHeadState = Buff;
					}
					SendMessageW(m_hWnd, WM_PAINT, NULL, NULL);
				}

				if (hasDuplicate(Snake)) { // Врезался в себя
					isThreading = 0;
					MessageBox(m_hWnd, L"Игра окончена!", L"Конец", MB_OK);
				}

				break;
			}
			break;
		}
		
		if (Snake.at(0).top == Apple.rect.top && Snake.at(0).left == Apple.rect.left)
		{
			if (Snake.size() > 1) {

				if (Snake.at(Snake.size() - 2).top == Snake.back().top && Snake.at(Snake.size() - 2).left < Snake.back().left) // Правее
				{
					NewSnakeElement = { Snake.back().left + OBJ_SIZE, Snake.back().top, Snake.back().right + OBJ_SIZE, Snake.back().bottom };
				}
				if (Snake.at(Snake.size() - 2).top == Snake.back().top && Snake.at(Snake.size() - 2).left > Snake.back().left) // Левее
				{
					NewSnakeElement = { Snake.back().left - OBJ_SIZE, Snake.back().top , Snake.back().right - OBJ_SIZE, Snake.back().bottom };
				}
				if (Snake.at(Snake.size() - 2).top > Snake.back().top && Snake.at(Snake.size() - 2).left == Snake.back().left) // Выше
				{
					NewSnakeElement = { Snake.back().left , Snake.back().top + OBJ_SIZE , Snake.back().right, Snake.back().bottom + OBJ_SIZE };
				}
				if (Snake.at(Snake.size() - 2).top < Snake.back().top && Snake.at(Snake.size() - 2).left == Snake.back().left) // Ниже
				{
					NewSnakeElement = { Snake.back().left , Snake.back().top - OBJ_SIZE , Snake.back().right, Snake.back().bottom - OBJ_SIZE };
				}
			}
			else {
				if (PreviousHeadState.top == Snake.back().top && PreviousHeadState.left < Snake.back().left) // Правее
				{
					NewSnakeElement = { Snake.back().left + OBJ_SIZE, Snake.back().top, Snake.back().right + OBJ_SIZE, Snake.back().bottom };
				}
				if (PreviousHeadState.top == Snake.back().top && PreviousHeadState.left > Snake.back().left) // Левее
				{
					NewSnakeElement = { Snake.back().left - OBJ_SIZE, Snake.back().top , Snake.back().right - OBJ_SIZE, Snake.back().bottom };
				}
				if (PreviousHeadState.top > Snake.back().top && PreviousHeadState.left == Snake.back().left) // Выше
				{
					NewSnakeElement = { Snake.back().left , Snake.back().top + OBJ_SIZE , Snake.back().right, Snake.back().bottom + OBJ_SIZE };
				}
				if (PreviousHeadState.top < Snake.back().top && PreviousHeadState.left == Snake.back().left) // Ниже
				{
					NewSnakeElement = { Snake.back().left , Snake.back().top - OBJ_SIZE , Snake.back().right, Snake.back().bottom - OBJ_SIZE };
				}
			}
			Snake.push_back(NewSnakeElement);
			SetApplePosition();
			Score++;
			ScoreText = std::to_wstring(Score);
			SendMessageW(m_hWnd, WM_PAINT, NULL, NULL);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_TIME));
	}
}

LRESULT BaseWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
		case WM_CREATE: 
		{
			if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory))) { return -1; }
			if (FAILED(CreateTextResources())) { return -1; }
			if (!MainWindowSetUpMenu()) { return -1;  }
			/*thread = std::thread([&]() {
				MoveSnake(std::ref(Direction));
			});
			thread.detach();*/
			SetTimer(m_hWnd, NULL, 95, (TIMERPROC)NULL);
			return 0;
		}
		case WM_CLOSE: 
		{
			int msgid = MessageBox(m_hWnd, L"Вы правда хотите выйти?", L"Выход", MB_OKCANCEL);
			if (msgid == IDOK) { DestroyWindow(m_hWnd); }
			return 0;
		}
		case WM_DESTROY: 
		{
			thread.~thread();
			isThreading = false;
			DiscardGraphicsResources();
			SafeRelease(&pFactory);
			PostQuitMessage(EXIT_SUCCESS);
			return EXIT_SUCCESS;
		}
		case WM_PAINT: 
		{
			OnPaint();
			return 0;
		}
		case WM_TIMER: 
		{
			if (!hasLost) {
				MoveSnakeAround(Direction);
			}
		}
		case WM_SIZE: 
		{
			Resize();
			return 0;
		}
		case WM_KEYDOWN:
		{
			switch ((DWORD)wParam)
			{
				case VK_LEFT:
				{
					if (Direction != DIR_RIGHT) { Direction = DIR_LEFT; }
					break;
				}
				case VK_UP:
				{
					if (Direction != DIR_DOWN) { Direction = DIR_UP; }
					break;
				}
				case VK_RIGHT:
				{
					if (Direction != DIR_LEFT) { Direction = DIR_RIGHT; }
					break;
				}
				case VK_DOWN:
				{
					if (Direction != DIR_UP) { Direction = DIR_DOWN; }
					break;
				}
				break;
			}
		}
		case WM_COMMAND: 
		{
			switch ((DWORD)wParam)
			{
				case SubMenuCommand1:
				{
					SetApplePosition();
					SendMessageW(m_hWnd, WM_PAINT, NULL, NULL);
					break;
				}
				case SubMenuCommand3:
				{
					PostQuitMessage(EXIT_SUCCESS);
					break;
				}
				case SubMenuCommand2: { // Перезагрузка
					Snake.clear();
					Snake.push_back({ 240 , 240 , 240 + OBJ_SIZE  , 240 + OBJ_SIZE });
					SetApplePosition();
					Score = 0;
					ScoreText = std::to_wstring(Score);
					isThreading = true;
					Direction = NULL;
					thread = std::thread([&]() {
						MoveSnake(std::ref(Direction));
					});
					thread.detach();
					SendMessageW(m_hWnd, WM_PAINT, NULL, NULL);
					break;
				}
			}
			break;
		}
	}
	return DefWindowProcW(m_hWnd, uMsg, wParam, lParam);
}

void BaseWindow::MoveSnakeAround(const int& Direction) {
	
	switch (Direction)
	{
	case DIR_LEFT:
	{
		if (Snake.at(0).left == SnakeArea.left) { // Выход за поле слева
			hasLost = true;
			MessageBox(m_hWnd, L"Игра окончена!", L"Конец", MB_OK);
		}

		if (Snake.at(0).left - STEPSIZE >= SnakeArea.left) {
			PreviousHeadState = Snake.at(0);
			Snake.at(0).left -= STEPSIZE;
			Snake.at(0).right -= STEPSIZE;
			for (auto it = Snake.begin() + 1; it != Snake.end(); it++) {
				Buff = *it;
				*it = PreviousHeadState;
				PreviousHeadState = Buff;
			}
			SendMessageW(m_hWnd, WM_PAINT, NULL, NULL);
		}

		if (hasDuplicate(Snake)) { // Врезался в себя
			hasLost = true;
			MessageBox(m_hWnd, L"Игра окончена!", L"Конец", MB_OK);
		}
		break;
	}
	case DIR_UP:
	{
		if (Snake.at(0).top == SnakeArea.top) { //Выход за поле сверху
			hasLost = true;
			MessageBox(m_hWnd, L"Игра окончена!", L"Конец", MB_OK);
		}

		if (Snake.at(0).top - STEPSIZE >= SnakeArea.top) {
			PreviousHeadState = Snake.at(0);
			Snake.at(0).top -= STEPSIZE;
			Snake.at(0).bottom -= STEPSIZE;
			for (auto it = Snake.begin() + 1; it != Snake.end(); it++) {
				Buff = *it;
				*it = PreviousHeadState;
				PreviousHeadState = Buff;
			}
			SendMessageW(m_hWnd, WM_PAINT, NULL, NULL);
		}

		if (hasDuplicate(Snake)) { // Врезался в себя
			hasLost = true;
			MessageBox(m_hWnd, L"Игра окончена!", L"Конец", MB_OK);
		}

		break;
	}
	case DIR_RIGHT:
	{
		if (Snake.at(0).right == SnakeArea.right) { //Выход за поле справа
			hasLost = true;
			MessageBox(m_hWnd, L"Игра окончена!", L"Конец", MB_OK);
		}

		if (Snake.at(0).right + STEPSIZE <= SnakeArea.right) {
			PreviousHeadState = Snake.at(0);
			Snake.at(0).left += STEPSIZE;
			Snake.at(0).right += STEPSIZE;
			for (auto it = Snake.begin() + 1; it != Snake.end(); it++) {
				Buff = *it;
				*it = PreviousHeadState;
				PreviousHeadState = Buff;
			}
			SendMessageW(m_hWnd, WM_PAINT, NULL, NULL);
		}

		if (hasDuplicate(Snake)) { // Врезался в себя
			hasLost = true;
			MessageBox(m_hWnd, L"Игра окончена!", L"Конец", MB_OK);
		}

		break;
	}
	case DIR_DOWN:
	{
		if (Snake.at(0).bottom == SnakeArea.bottom) { //Выход за поле снизу
			hasLost = true;
			MessageBox(m_hWnd, L"Игра окончена!", L"Конец", MB_OK);
		}

		if (Snake.at(0).bottom + STEPSIZE <= SnakeArea.bottom) {
			PreviousHeadState = Snake.at(0);
			Snake.at(0).top += STEPSIZE;
			Snake.at(0).bottom += STEPSIZE;
			for (auto it = Snake.begin() + 1; it != Snake.end(); it++) {
				Buff = *it;
				*it = PreviousHeadState;
				PreviousHeadState = Buff;
			}
			SendMessageW(m_hWnd, WM_PAINT, NULL, NULL);
		}

		if (hasDuplicate(Snake)) { // Врезался в себя
			hasLost = true;
			MessageBox(m_hWnd, L"Игра окончена!", L"Конец", MB_OK);
		}

		break;
	}
	break;
	}

	if (Snake.at(0).top == Apple.rect.top && Snake.at(0).left == Apple.rect.left)
	{
		if (Snake.size() > 1) {

			if (Snake.at(Snake.size() - 2).top == Snake.back().top && Snake.at(Snake.size() - 2).left < Snake.back().left) // Правее
			{
				NewSnakeElement = { Snake.back().left + OBJ_SIZE, Snake.back().top, Snake.back().right + OBJ_SIZE, Snake.back().bottom };
			}
			if (Snake.at(Snake.size() - 2).top == Snake.back().top && Snake.at(Snake.size() - 2).left > Snake.back().left) // Левее
			{
				NewSnakeElement = { Snake.back().left - OBJ_SIZE, Snake.back().top , Snake.back().right - OBJ_SIZE, Snake.back().bottom };
			}
			if (Snake.at(Snake.size() - 2).top > Snake.back().top && Snake.at(Snake.size() - 2).left == Snake.back().left) // Выше
			{
				NewSnakeElement = { Snake.back().left , Snake.back().top + OBJ_SIZE , Snake.back().right, Snake.back().bottom + OBJ_SIZE };
			}
			if (Snake.at(Snake.size() - 2).top < Snake.back().top && Snake.at(Snake.size() - 2).left == Snake.back().left) // Ниже
			{
				NewSnakeElement = { Snake.back().left , Snake.back().top - OBJ_SIZE , Snake.back().right, Snake.back().bottom - OBJ_SIZE };
			}
		}
		else {
			if (PreviousHeadState.top == Snake.back().top && PreviousHeadState.left < Snake.back().left) // Правее
			{
				NewSnakeElement = { Snake.back().left + OBJ_SIZE, Snake.back().top, Snake.back().right + OBJ_SIZE, Snake.back().bottom };
			}
			if (PreviousHeadState.top == Snake.back().top && PreviousHeadState.left > Snake.back().left) // Левее
			{
				NewSnakeElement = { Snake.back().left - OBJ_SIZE, Snake.back().top , Snake.back().right - OBJ_SIZE, Snake.back().bottom };
			}
			if (PreviousHeadState.top > Snake.back().top && PreviousHeadState.left == Snake.back().left) // Выше
			{
				NewSnakeElement = { Snake.back().left , Snake.back().top + OBJ_SIZE , Snake.back().right, Snake.back().bottom + OBJ_SIZE };
			}
			if (PreviousHeadState.top < Snake.back().top && PreviousHeadState.left == Snake.back().left) // Ниже
			{
				NewSnakeElement = { Snake.back().left , Snake.back().top - OBJ_SIZE , Snake.back().right, Snake.back().bottom - OBJ_SIZE };
			}
		}
		Snake.push_back(NewSnakeElement);
		SetApplePosition();
		Score++;
		ScoreText = std::to_wstring(Score);
		SendMessageW(m_hWnd, WM_PAINT, NULL, NULL);
	}
}

class SnakeSample {
public:
	SnakeSample(float x, float y, float SnakeElementSize = 20.0f)
	{
		this->x = x;
		this->y = y;
		this->SnakeElementSize = SnakeElementSize;
		Size = 0;
		Snake.push_back({ x, y, x + SnakeElementSize, y + SnakeElementSize });
	}

	SnakeSample* EnlargeSnake() {
		Snake.push_back({ x, y, x + SnakeElementSize, y + SnakeElementSize });
		return this;
	}

	~SnakeSample() {

	}

	float x;
	float y;
	float SnakeElementSize;
	int Size;
	std::vector<D2D1_RECT_F> Snake;

};