//
// Copyright (c) 2022 the TQModRuntime project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "proc.h"
#include <psapi.h>

//=============================================================================
// reference for this routine was found in this video:
// https://www.youtube.com/watch?v=wiX5LmdD5yk
//=============================================================================
DWORD GetProcId(const wchar_t* procName)
{
  DWORD procId = 0;
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  if (hSnap != INVALID_HANDLE_VALUE)
  {
    PROCESSENTRY32 procEntry;
    procEntry.dwSize = sizeof(procEntry);

    if (Process32First(hSnap, &procEntry))
    {
      do
      {
        if (!_wcsicmp(procEntry.szExeFile, procName))
        {
          procId = procEntry.th32ProcessID;
          break;
        }
      } while (Process32Next(hSnap, &procEntry));
    }
  }

  CloseHandle(hSnap);

  return procId;
}

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
  uintptr_t modBaseAddr = 0;
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);

  if (hSnap != INVALID_HANDLE_VALUE)
  {
    MODULEENTRY32 modEntry;
    modEntry.dwSize = sizeof(modEntry);

    if (Module32First(hSnap, &modEntry))
    {
      do
      {
        if (!_wcsicmp(modEntry.szModule, modName))
        {
          modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
          //break;
        }
      } while (Module32Next(hSnap, &modEntry));
    }
  }

  CloseHandle(hSnap);

  return modBaseAddr;
}


