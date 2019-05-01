#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef DEBUG
#define DEBUG
#endif

#include <Windows.h>

#include "DX12/DX12R.h"

#include <string.h>
#include <memory>
#include <assert.h>

#include "Framework.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720