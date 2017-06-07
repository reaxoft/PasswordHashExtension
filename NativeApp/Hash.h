#pragma once


int CalcPasswordHash(wchar_t *dst, int dstLen, Platform::String ^_salt, WCHAR* psswd);