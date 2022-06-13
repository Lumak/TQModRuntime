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

#include "DBase.h"

//=============================================================================
//=============================================================================
namespace DBase
{
  unsigned int Setup(HANDLE hProcess, DWORD procId, uintptr_t gameDllBase)
  {
    unsigned int databaseAddr = 0;

    //001A0CA0 ?GetDatabase@GameEngine@GAME@@QBEABVLoadTable@2@XZ
    BYTE getdatabaseopcode[0x200];
    unsigned int sizeofopcodebuffer = 0x13f; //size from cheatEngine
    uintptr_t getDatabaseFnAddr = gameDllBase + 0x001A0CA0;
    ReadProcessMemory(hProcess, (BYTE*)getDatabaseFnAddr, &getdatabaseopcode, sizeofopcodebuffer, nullptr);

    // create small memory in target's memory space to store the database object pointer
    // note: memory created by VirtualAllocEx() is already intialized to zero
    DWORD  dwBufferSize = 0x20;
    void *pMemory = VirtualAllocEx(hProcess, 0, dwBufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    BYTE *pWriteDatabaseObjAddr = (BYTE*)pMemory;
    unsigned int dbaseDestinationAddr = (unsigned int)pWriteDatabaseObjAddr;

    if (pMemory != NULL)
    {
      const unsigned int origReturnOpcodeIdx = 0x136; // acquired from cheatengine
      const BYTE movOpcode = 0xA3; // move eax to ds address
      const BYTE retOpcode = 0xC3; // return
      BYTE vercode[6];

      ReadProcessMemory(hProcess, (BYTE*)getDatabaseFnAddr + origReturnOpcodeIdx, &vercode, sizeof(vercode), nullptr);

      // look for ret opcode or verify previous overwrite
      if (getdatabaseopcode[origReturnOpcodeIdx] == retOpcode)
      {
        // has not been updated yet, write mov eax to the address that's created
        //A3 98 43 37 10     mov         dword ptr ds : [10374398h], eax //10374398h is random address just shown as example
        //C3                 ret   
        BYTE codeChange[6];
        codeChange[0] = movOpcode;
        codeChange[1] = ((BYTE*)&dbaseDestinationAddr)[0];
        codeChange[2] = ((BYTE*)&dbaseDestinationAddr)[1];
        codeChange[3] = ((BYTE*)&dbaseDestinationAddr)[2];
        codeChange[4] = ((BYTE*)&dbaseDestinationAddr)[3];
        codeChange[5] = retOpcode;

        // rewrite mem code
        BOOL success = WriteProcessMemory(hProcess, (BYTE*)getDatabaseFnAddr + origReturnOpcodeIdx, codeChange, sizeof(codeChange), nullptr);
        while (success == FALSE)
        {
          Sleep(2);
          success = WriteProcessMemory(hProcess, (BYTE*)getDatabaseFnAddr + origReturnOpcodeIdx, codeChange, sizeof(codeChange), nullptr);
        }

        printf("acquire database fn added\n");
      }
      // fn has already been modified, verify the two opcodes
      else if (vercode[0] == movOpcode && vercode[5] == retOpcode)
      {
        // read the destination address of where the database pointer is stored
        unsigned int writeAddr = 0;
        ReadProcessMemory(hProcess, (BYTE*)getDatabaseFnAddr + origReturnOpcodeIdx + 1, &writeAddr, sizeof(writeAddr), nullptr);
        pWriteDatabaseObjAddr = (BYTE*)writeAddr;

        printf("acquire database fn already changed\n");
      }
      else
      {
        // possiblely the wrong game.dll version is found
        printf("invalid game.dll data\n");
        return 0;
      }

      // wait until database obj address becomes available
      printf("waiting for database scene update\n");
      while (databaseAddr == 0)
      {
        ReadProcessMemory(hProcess, (BYTE*)pWriteDatabaseObjAddr, &databaseAddr, sizeof(databaseAddr), nullptr);
        Sleep(100);

        // exit if TQ terminated during this process
        if (GetModuleBaseAddress(procId, L"TQ.exe") == NULL)
        {
          printf("TQ terminated, exit\n");
          return 0;
        }
      }
    }

    if (databaseAddr != 0)
    {
      printf("database mod added\n");
    }

    return databaseAddr;
  }
}