// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define  _WINSOCK_DEPRECATED_NO_WARNINGS
#include "targetver.h"

#include <map>
#include <list>
#include <stdio.h>
#include <tchar.h>
#include <WinSock2.h>
#include <mswsock.h>
#include <Windows.h>

#ifdef _DEBUG
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define DEBUG_NEW new
#endif

// TODO: 在此处引用程序需要的其他头文件
