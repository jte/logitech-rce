/**
 *	An implementation of Logitech HID++ 2.0 with Signal11 HIDAPI C.
 *		*!* Transport Layer uses Big-Endian.
 */
#pragma once
#include <iostream>
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <set>
#include <map>

class Hidpp
{
public:
	void Initialize();
	void SetWindow(HWND hWnd) { m_hWnd = hWnd; }
private:
	HWND m_hWnd;
};