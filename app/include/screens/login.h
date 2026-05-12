#pragma once
#include "raylib.h"

inline const Color LG_BG         = {  12,  10,   8, 255 };
inline const Color LG_SIDEBAR    = {  28,  24,  20, 255 };
inline const Color LG_PANEL      = {  22,  18,  14, 255 };
inline const Color LG_BORDER     = {  55,  46,  36, 255 };
inline const Color LG_ORANGE     = { 234, 108,  15, 255 };
inline const Color LG_ORANGE_LT  = { 251, 146,  60, 255 };
inline const Color LG_ORANGE_GL  = { 251, 146,  60,  50 };
inline const Color LG_WHITE      = { 255, 255, 255, 255 };
inline const Color LG_GREY       = { 180, 160, 140, 255 };
inline const Color LG_DARKGREY   = { 100,  85,  70, 255 };
inline const Color LG_INPUT_BG   = {  32,  27,  21, 255 };
inline const Color LG_INPUT_ACT  = {  42,  35,  26, 255 };
inline const Color LG_ERROR      = { 220,  80,  60, 255 };
inline const Color LG_SUCCESS    = {  80, 200, 120, 255 };

int  drawLoginScreen(int sw, int sh);
void resetLoginToLanding();
bool loginRequestedExit();

bool drawButton(const char* label, int x, int y, int w, int h, bool danger = false);
bool drawOutlineButton(const char* label, int x, int y, int w, int h);
