// header.h: включаемый файл для стандартных системных включаемых файлов
// или включаемые файлы для конкретного проекта
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows
// Файлы заголовков Windows
#include <windows.h>
// Файлы заголовков среды выполнения C
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
// Мои файлы заголовков 

#include <shobjidl.h>
#include <d2d1.h>
#include <string>
#include <iostream>
#include <dwrite.h>
#include <stdio.h>
#include <atlbase.h>
#include <random>
#include <vector>
#include <thread>
#include <chrono>

#include <mutex>


#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2d1")