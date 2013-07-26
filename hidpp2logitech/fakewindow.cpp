#include <Windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include "main.h"

// maximum mumber of lines the output console should have

#define MAX_CONSOLE_LINES 500

VOID RedirectIOToConsole()
{
    int hConHandle;
    long lStdHandle;
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    FILE *fp;

    // allocate a console for this app
    AllocConsole();

    // set the screen buffer to be big enough to let us scroll text
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
    coninfo.dwSize.Y = MAX_CONSOLE_LINES;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

    // redirect unbuffered STDOUT to the console
    lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stdout = *fp;
    setvbuf( stdout, NULL, _IONBF, 0 );

    // redirect unbuffered STDIN to the console
    lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "r" );
    *stdin = *fp;

    setvbuf( stdin, NULL, _IONBF, 0 );

    // redirect unbuffered STDERR to the console

    lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stderr = *fp;
    setvbuf( stderr, NULL, _IONBF, 0 );
}

// For informational messages and window titles
LPCTSTR g_pszAppName;

VOID MessagePump(HWND hWnd)
{
    MSG msg; 
    int retVal;

    while((retVal = GetMessage(&msg, NULL, 0, 0)) != 0) 
    { 
        if(retVal == -1)
        {
            break;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } 
}

#include "blowfish.h"

unsigned int __cdecl GetBlockSizeOfBuffer(int Buffer, unsigned int Size)
{
  return Buffer + Size - 1 - (Buffer + Size - 1) % Size;
}

void IniLoadFile(LPCSTR lpFileName)
{
	if(!lpFileName)
	{
		printf("$$\n");
	}
	if(!*lpFileName)
	{
		printf("$$$$\n");
	}
	HANDLE 
		g_hHeap = GetProcessHeap();
	HANDLE
		hObject = CreateFileA(lpFileName, 0x80000000u, 0, 0, 3u, 0x80u, 0);
	BYTE* lpMem;
	if(hObject != (HANDLE)-1)
	{
		DWORD
			nNumberOfBytesToRead = GetFileSize(hObject, 0);
		DWORD
			dwNumberOfBytesRead;
		if ( nNumberOfBytesToRead != -1 )
			lpMem = (BYTE*)HeapAlloc(g_hHeap, 8u, nNumberOfBytesToRead);
		if ( lpMem )
		{
			if(ReadFile(hObject, lpMem, nNumberOfBytesToRead, &dwNumberOfBytesRead, 0) != 1)
			{
				printf("$3\n");
			}
		}
		if ( lpMem )
		{
			char key[] = "SetPoint-83918EEE-DB31-4df1-B11D-F93932DBC1B8";
			//v39 = *(_WORD *)"8";
			printf("key:%s\n",key);
			int 
				keylen = strlen(key);
			blowfish::Pad
				pad = blowfish::generatePad(&key, keylen);
			blowfish::Block chain(0xC009F158u, 0x17EC17ECu);
			if(strncmp((const char*)lpMem, ";KHAL", 5))
			{
				DWORD
					*dest = NULL;
				DWORD
					v33 = *(DWORD *)lpMem,
					byteSize = GetBlockSizeOfBuffer(v33, 8u);
				if(dwNumberOfBytesRead != byteSize + 12 || v33 > byteSize)
				{
					printf("$1\n");
				}
				dest = (DWORD*)HeapAlloc(g_hHeap, 8u, byteSize + 1);
				if(!dest)
				{
					printf("$2\n");
				}
				blowfish::decrypt_CBC(pad, (const void*)(lpMem + 4), dest, byteSize, &chain);
				chain = blowfish::core::encipherBlock(pad, chain);
				DWORD
					L = *(DWORD *)&lpMem[byteSize + 4];
				DWORD
					R = *(DWORD *)&lpMem[byteSize + 8];
				if((chain.L != L || chain.R != R))
				{
					printf("not matching L and R pairs %x %x\n", L, R);
				}
				std::ofstream of("Device_Decrypted.ini");
				of << std::string((char*)dest);
				of.close();
			}
			else
			{
				printf("other shit\n");
				/*
				lpBuffer = 0;
				HANDLE
				hFile = CreateFileA(lpFileName, 0x40000000u, 0, 0, 2u, 0x80u, 0);
				if ( hFile != (HANDLE)-1 )
				{
					nNumberOfBytesToWrite = GetBlockSizeOfBuffer(Buffer, 8u);
					lpBuffer = HeapAlloc(g_hHeap, 8u, nNumberOfBytesToWrite);
					WriteFile(hFile, &Buffer, 4u, &NumberOfBytesWritten, 0);
					if(lpBuffer)
					{
						blowfish::decrypt_CFB(pad, (const void*)lpMem, (void*)lpBuffer, nNumberOfBytesToWrite, &chain);
						WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, &NumberOfBytesWritten, 0);
						HeapFree(g_hHeap, 0, (LPVOID)lpBuffer);
					}
					chain = blowfish::core::encipherBlock(pad, chain);
					WriteFile(hFile, &chain, 4u, &NumberOfBytesWritten, 0);
					WriteFile(hFile, &chain.R, 4u, &NumberOfBytesWritten, 0);
					CloseHandle(hFile);
				}
				*/
			}
			HeapFree(g_hHeap, 0, lpMem);
			lpMem = 0;
		}
	}
}


//
// WinProcCallback
//
INT_PTR WINAPI WinProcCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet = 1;
    switch (message)
    {
        case WM_CREATE:
        {
			RedirectIOToConsole();
			char szPath[] = "C:\\GooglesData\\Google\'s Data\\_Reverse Engineering\\logitech-rce\\hidpp2logitech\\Release\\Device.ini";
			IniLoadFile(szPath);
			/*
			Hidpp *hpp = new Hidpp;
			hpp->SetWindow(hWnd);
			hpp->Initialize();*/
        }
        break;
        case WM_CLOSE:
        {
            FreeConsole();
            DestroyWindow(hWnd);
        }
        break;
        case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
        break;
		case WM_USER:
		{
			printf("Received message!\n");
		}
		break;
        default:
        {
            lRet = DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }

    return lRet;
}

#define WND_CLASS_NAME TEXT("HIDPP2LOGITECH")

BOOL InitWindowClass()
{
    WNDCLASSEX wndClass;

    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wndClass.hInstance = (HINSTANCE)GetModuleHandle(NULL);
    wndClass.lpfnWndProc = (WNDPROC)WinProcCallback;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hIcon = LoadIcon(0,IDI_APPLICATION);
    wndClass.hbrBackground = CreateSolidBrush(RGB(192,192,192));
    wndClass.hCursor = LoadCursor(0, IDC_ARROW);
    wndClass.lpszClassName = WND_CLASS_NAME;
    wndClass.lpszMenuName = NULL;
    wndClass.hIconSm = wndClass.hIcon;


    if(!RegisterClassEx(&wndClass))
    {
        return FALSE;
    }
    return TRUE;
}

INT WINAPI WinMain(HINSTANCE hInstanceExe, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    int nArgC;
    LPTSTR ppArgV;
    HWND hWnd;
    nArgC = 0;
    ppArgV = GetCommandLine();
    //g_pszAppName = ppArgV[0];
    if(!InitWindowClass())
    {
        // InitWindowClass displays any errors
        return -1;
    }

    // Main app window

    hWnd = CreateWindowEx(
                    WS_EX_CLIENTEDGE | WS_EX_APPWINDOW,
                    WND_CLASS_NAME,
                    g_pszAppName,
                    WS_OVERLAPPEDWINDOW, // style
                    CW_USEDEFAULT, 0, 
                    640, 480,
                    NULL, NULL, 
                    hInstanceExe, 
                    NULL);

    if ( hWnd == NULL )
    {
        return -1;
    }

    // Actually draw the window.

    //ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);
    // The message pump loops until the window is destroyed.
    MessagePump(hWnd);
    return 1;
} 