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
#include "DBase.h"
#include "Speed.h"
#include "Loot.h"
//=============================================================================
//=============================================================================
const int MajVersion = 1;
const int MinVersion = 0;

#define ERROR_PROCID_NOTFOUND       1
#define ERROR_FAILED_PROCESS        2
#define ERROR_FAILED_DATABASE_ADDR  3

//=============================================================================
//=============================================================================
int main()
{
  // vars
  int retCode = 0;
  DWORD procId = GetProcId(L"TQ.exe");
  uintptr_t moduleBase = 0;
  uintptr_t gameDllBase = 0;
  unsigned int databaseAddr = 0;

  // print info in console
  printf("TQModRuntime (c) copyright 2022\nVersion %02d.%02d\n", MajVersion, MinVersion);

  // get module, submodule address
  if (procId != 0)
  {
    printf("TQ process found, procid = %d\n", procId);
    moduleBase = GetModuleBaseAddress(procId, L"TQ.exe");
    gameDllBase = GetModuleBaseAddress(procId, L"Game.dll");
    printf("gameDllBase = %X\n", gameDllBase);
  }
  else
  {
    printf("TQ procId not found, exit\n");
    return ERROR_PROCID_NOTFOUND;
  }

  // open process to be able to create, read, and write target memory
  HANDLE hProcess = 0;
  if (procId != 0)
  {
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
  }
  if (hProcess == NULL)
  {
    printf("process failed to open, exit\n");
    return ERROR_FAILED_PROCESS;
  }

  //set mods
  databaseAddr = DBase::Setup(hProcess, procId, gameDllBase);

  if (databaseAddr != NULL)
  {
    Speed::Setup(hProcess, databaseAddr);
    Loot::Setup(hProcess, gameDllBase, databaseAddr);
  }
  else
  {
    printf("failed to get database address\n");
    retCode = ERROR_FAILED_DATABASE_ADDR;
  }

  CloseHandle(hProcess);

  return retCode;
}

