#pragma once
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <string>
#include <tchar.h>
#include <map>


using namespace std;

enum ERROR_FILE_OPNE {
    ERROR_GET_FILE_NAME,
    ERROR_START_PROCESS
};

constexpr int MOUSE = 0;
constexpr int KEYBOARD = 1;

map<UINT8, BOOL>m_mapKeyStore;

//훅 전역 변수
HHOOK keyboardHook;
HHOOK mouseHook;

//필요 시 전처리기에 _GETKEYVALUES 설정하여 사용하고 싶은 키 Value 확인후 정의
//페이지 다운 업 키 설정
constexpr UINT8 VIEW_LOG = VK_F11;
constexpr UINT8 PG_DOWN = VK_F10;
constexpr UINT8 PG_UP = VK_F9;
//얻을 핸들 숫자
constexpr UINT8 m_nProgramCnt = 2;
//PDF 핸들값을 저장할 변수
HWND WindowHandle[m_nProgramCnt]{};
//Log On/Off
bool bViewLog = FALSE;

HWND m_MainHwnd = GetConsoleWindow();
//핸들 Alive 체크용 boolean 변수

BOOL StartProcess_SpyPlusplus();
BOOL SetProcessHandle();
BOOL CheckingProcessAlive(HWND hwnd);
BOOL Select_SpyPlusplus();
void PrintProcessInfo();
void InitProcess();
void SetKeyStore();
void InitProcessHandle();

// 초기화 작업
void InitProcess()
{
    SetKeyStore();
    InitProcessHandle();
}


// 사용 가능한 키 지정 여기서 지정된 키만 사용 가능
void SetKeyStore() {
    //기본 지정 키 초기화
    for (int i = VK_F7; i <= VK_F12; i++) {
        m_mapKeyStore[i] = true;
    }
    // #TODO : 추가 키 사용 시 추가
}

string GetLogString(const UINT8 nKey) {
    string strLog = "";
    switch (nKey) {
    case VIEW_LOG: strLog = bViewLog ? "LOG 출력" : "LOG 출력 중지"; break;
    case PG_DOWN:  strLog = "PAGE DOWN"; break;
    case PG_UP:  strLog = "PAGE UP"; break;
    case VK_F7: strLog = "핸들 재설정"; break;
    case VK_F8: strLog = "핸들 상태 확인"; break;
    case VK_F12: strLog = "작업표시줄 상태 :" + IsWindowVisible(m_MainHwnd) ? "표시" : "미표시"; break;
    default: strLog = "알려지지 않은 Key값"; break;
    }

    return strLog + "\n";
}

// 핸들 값 초기화 
void InitProcessHandle()
{
    for (int i = 0; i < m_nProgramCnt; i++) {
        WindowHandle[i] = nullptr;
    }
}

// 키보드에서 눌린 키 값에 맞춰 동작 실행
void KeyboardController(DWORD vkCode) {
    switch (vkCode) {
    case PG_UP:
        for (const HWND hwndValue : WindowHandle) {
            SendMessage(hwndValue, WM_KEYDOWN, WPARAM(VK_PRIOR), LPARAM(0));
            SendMessage(hwndValue, WM_KEYUP, WPARAM(VK_PRIOR), LPARAM(0));
        }
        break;

    case PG_DOWN:
        for (const HWND hwndValue : WindowHandle) {
            SendMessage(hwndValue, WM_KEYDOWN, WPARAM(VK_NEXT), LPARAM(0));
            SendMessage(hwndValue, WM_KEYUP, WPARAM(VK_NEXT), LPARAM(0));
        }
        break;

    case VK_F7:
        // 재설정 시 작업표시줄 숨김 상태라면 표시 상태로 변경
        if (IsWindowVisible(m_MainHwnd) == SW_HIDE) ShowWindow(m_MainHwnd, SW_SHOW);
        // Spy ++ 실행 후 핸들 설정
        if (StartProcess_SpyPlusplus() == TRUE)  SetProcessHandle();
        // 단축키 설명 재출력
        PrintProcessInfo();
        break;

    case VK_F8:
        // 핸들값이 유효한 상태인지 체크 
        for (const HWND hwndValue : WindowHandle) CheckingProcessAlive(hwndValue);
        PrintProcessInfo();
        break;

    case VIEW_LOG:
        bViewLog = !bViewLog;
        cout << GetLogString((UINT8)vkCode);
        break;

    case VK_F12:
        ShowWindow(m_MainHwnd, IsWindowVisible(m_MainHwnd) ? SW_HIDE : SW_SHOW);
        break;

    default: break;
    }
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        MSLLHOOKSTRUCT* pMouseStruct = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
#ifdef _GETKEYVALUES
        if (wParam == WM_MOUSEMOVE) {
            std::cout << "Mouse Move: X: " << pMouseStruct->pt.x << ", Y: " << pMouseStruct->pt.y << std::endl;
        }
        return CallNextHookEx(mouseHook, nCode, wParam, lParam);
#endif // DEBUG
        // 마우스 휠 감지 
        if (wParam == WM_MOUSEWHEEL) {
            WPARAM localwParam = MAKEWPARAM(0, GET_WHEEL_DELTA_WPARAM(pMouseStruct->mouseData));
            for (const HWND hwndValue : WindowHandle) {
                if (hwndValue) {
                    SendMessage(hwndValue, WM_MOUSEWHEEL, localwParam, 0);
                    SendMessage(hwndValue, WM_MOUSEWHEEL, localwParam, 0);
                }
            }
        }
        //마우스 움직임 감지 
        else if (wParam == WM_MOUSEMOVE) {
            //std::cout << "Mouse Move: X: " << pMouseStruct->pt.x << ", Y: " << pMouseStruct->pt.y << std::endl;
        }
    }

    return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* kbStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

        // 유효한 키가 눌린 경우 동작 실행
        if (wParam == WM_KEYDOWN && m_mapKeyStore[(UINT8)kbStruct->vkCode] == TRUE) {
#ifdef _GETKEYVALUES
            printf("%d\n", kbStruct->vkCode);
            return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
#endif // DEBUG
            // 로그 출력 설정값에 따라 로그출력 실행
            if (bViewLog) cout << GetLogString((UINT8)kbStruct->vkCode);
            // 키 값에 따른 동작 실행
            KeyboardController(kbStruct->vkCode);
        }
    }

    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

//실행파일을 포함한 경로를 받아 실행시켜주는 함수
BOOL StartProcess_SpyPlusplus(LPCWSTR path)
{
    //const TCHAR* path = _T("C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\Tools\\spyxx.exe");
    HINSTANCE result = ShellExecute(NULL, _T("runas"), path, NULL, NULL, SW_SHOWNORMAL);

    if (reinterpret_cast<long long>(result) > 32)  cout << "Spy++ 실행 성공 (검색 -> 창 찾기 -> 마우스로 찾기도구 아이콘을 끌어 핸들 탐색) " << endl;
    else {
        DWORD error = GetLastError();
        cerr << "프로세스 실행에 실패했습니다. 오류 코드: " << error << endl;
        return FALSE;
    }

    return TRUE;
}

BOOL StartProcess_SpyPlusplus()
{
    BOOL bOK = TRUE;
    TCHAR filePath[MAX_PATH];
    try {
        if (GetModuleFileName(nullptr, filePath, MAX_PATH) == FALSE) throw ERROR_GET_FILE_NAME;

        wstring wstringResult(filePath);
        TCHAR SpyPath[MAX_PATH];
        // 실행파일 이름을 삭제하고 spy++ 바로가기 이름으로 대체하고 종료문자인 \0도 함께 추가
        wstringResult = wstringResult.substr(0, wstringResult.rfind(L"\\") + 1).append(L"spyxx.lnk\0");
        copy(wstringResult.begin(), wstringResult.end(), SpyPath);
        //설정한 경로에 있는 프로세스 실행
        if (StartProcess_SpyPlusplus(SpyPath) == FALSE) throw ERROR_START_PROCESS;
    }
    catch (ERROR_FILE_OPNE ErrorType) {
        switch (ErrorType) {
        case ERROR_GET_FILE_NAME:  cout << "실행파일 경로를 가져올 수 없습니다. - spy++ 실행 파일을 직접 선택 해주세요. " << endl;
        case ERROR_START_PROCESS:  cout << "기본경로값 OPEN 실패 - spy++ 실행 파일을 직접 선택 해주세요. " << endl;
        }
        // 직접선택 후 오픈 성공했다면 TRUE 반환
        bOK = Select_SpyPlusplus();
    }

    return bOK;
}

BOOL SetProcessHandle()
{
    //핸들값을 저장할 전역변수 초기화
    InitProcessHandle();
    for (int i = 0; i < m_nProgramCnt; i++) {
        long long hexValue;
        string sTemp;
        stringstream ss;
        cout << i + 1 << "번째 pdf 핸들값을 입력 : ";
        cin >> sTemp;
        //입력받은 string 값을 16진수로 변환
        ss << hex << sTemp;
        ss >> hexValue;
        WindowHandle[i] = static_cast<HWND>(reinterpret_cast<HANDLE>(hexValue));

        //핸들이 올바른지 체크
        if (CheckingProcessAlive(WindowHandle[i]) == TRUE) {
            //nOtherIdx가 같이 실행되는 핸들을 바라볼 수 있도록 설정
            int nOtherIdx = i == 0 ? 1 : 0;
            //같은 핸들값 중복 입력 체크
            if (WindowHandle[i] == WindowHandle[nOtherIdx]) {
                cerr << "같은 핸들 중복 입력" << endl;
                return FALSE;
            }
        }
        else
            return FALSE;
    }
    return TRUE;
}

BOOL CheckingProcessAlive(HWND hwnd)
{
    BOOL bOK = FALSE;
    TCHAR szWindowTitle[256];
    if (hwnd != NULL) {
        int len = GetWindowText(hwnd, szWindowTitle, sizeof(szWindowTitle) / sizeof(szWindowTitle[0]));
        bOK = len > 0 ? TRUE : FALSE;
    }

    // hwnd값이 NULL이 아니고 캡션 이름이 있다면 통과
    if (bOK == TRUE)  wcout << "윈도우 캡션(타이틀) 이름: " << szWindowTitle << endl;
    else cerr << "프로세스를 찾지 못했습니다." << endl;

    return bOK;
}

BOOL Select_SpyPlusplus()
{
    BOOL bResult = FALSE;
    HWND hWnd = GetConsoleWindow();
    OPENFILENAME OFN;
    TCHAR filePathName[100] = L"";
    TCHAR lpstrFile[100] = L"";

    memset(&OFN, 0, sizeof(OPENFILENAME));
    OFN.lStructSize = sizeof(OPENFILENAME);
    OFN.hwndOwner = hWnd;
    OFN.lpstrFilter = TEXT("Executable Files\0*.exe\0Folders\0*.\0");
    OFN.lpstrFile = lpstrFile;
    OFN.nMaxFile = 100;
    OFN.lpstrInitialDir = L".";

    if (GetOpenFileName(&OFN) != 0) {
        wcout << OFN.lpstrFile << endl;
        wsprintf(filePathName, L"%s 파일을 열겠습니까?", OFN.lpstrFile);
        if (MessageBox(hWnd, filePathName, L"열기 선택", MB_OKCANCEL) == TRUE) {
            bResult = StartProcess_SpyPlusplus(OFN.lpstrFile);
        }
    }
    return bResult;
}

//단축키 설명 출력
void PrintProcessInfo() {
    string strStar = "", strInfo = "";
    strInfo = "F7 : 핸들 재설정 / F8 : HWND Value Check  /  F9 : PAGE UP  /  F10 : PAGE DOWN  / F11 : 로그표시 / F12 : 작업표시줄(HIDE/SHOW)\n";
    for (int i = 0; i < strInfo.length(); i++) strStar.append("*");
    strStar.append("\n");

    cout << strStar << strInfo << strStar;
}

BOOL StartHook() 
{
    const string strHookError[m_nProgramCnt] = { "Mause", "Keyboard" };

    try {
        keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
        //mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(NULL), 0);

        if (keyboardHook == NULL) throw strHookError[KEYBOARD];
        if (mouseHook == NULL) throw strHookError[MOUSE];
    }
    catch (const string errorString) {
        cerr << "Hook Error : " << errorString << endl;
        if (errorString == strHookError[KEYBOARD]) {
            return FALSE;  // 키보드 후킹 실패시 종료
        }

        cout << "후킹 시작 \n" << endl;
        PrintProcessInfo(); // 상태 출력

        // 메시지 루프
        MSG msg;
        while (GetMessage(&msg, NULL, NULL, NULL) != NULL) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 후킹 종료
        if (keyboardHook != NULL) UnhookWindowsHookEx(keyboardHook);
        if (mouseHook != NULL) UnhookWindowsHookEx(mouseHook);
        return TRUE;

    }
}


    BOOL main() {
        BOOL bOK = FALSE;
#ifdef _GETKEYVALUES
        StartHook();
#else
        // ERROR처리 : HIDE 상태로 프로세스 강제 종료 시 다음 실행될 때 HIDE 상태로 실행될 수 있음
        ShowWindow(m_MainHwnd, SW_SHOW);
        InitProcess();

        while (StartProcess_SpyPlusplus() == TRUE) {
            if (SetProcessHandle() == TRUE) {
                cout << "핸들 입력 완료 " << endl;
                //키보드 후킹 시작
                bOK = StartHook();
                break;
            }
            else  cerr << "다시 입력해주세요" << endl;
        }

#endif
        return bOK;
    }