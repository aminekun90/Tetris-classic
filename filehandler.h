#include <fstream>
#include <iostream>
#include<string>
#ifdef _WIN32
#include <windows.h>
#else
#include "platform/wincon_compat.h"
#endif
using namespace std;
DWORD GetHeightScore();
void RegisterHightScore(DWORD points);
void GetScore(string*);
void RegisterScore(DWORD points);