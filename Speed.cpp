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

#include "Speed.h"

namespace Speed
{
  //=============================================================================
  //=============================================================================
  // game running without dbr records added (or a non-mod game) the address offsets are: 
  // 0x11C - attack speed
  // 0x140 - run speed
  // 0x148 - cast speed
  // and mod with dbr record added the address offsets are:
  // 0x0F0 - attack speed
  // 0x114 - run speed
  // 0x12C - cast speed

  // the attack speed is much higher than other speeds to compensate for when a weapon is equipped
  // -see the difference between the attack speeds with a weapon equipped and without in game
  const int numSpeedVars = 6;
  SpeedData speedData[numSpeedVars] =
  {
    { 0x0F0, 190.0f },
    { 0x114, 120.0f },
    { 0x12C, 120.0f },
    { 0x11C, 190.0f },
    { 0x140, 120.0f },
    { 0x148, 120.0f }
  };

  const float fiftyFloat = 50.0f;
  const unsigned int fiftyHex = *(unsigned int*)&fiftyFloat;

  //=============================================================================
  //=============================================================================
  BOOL Setup(HANDLE hProcess, unsigned int databaseAddr)
  {
    BOOL success = TRUE;

    if (databaseAddr != 0)
    {
      // capture database memory for debugging
      // todo: remove
      BYTE database[0x200];
      ReadProcessMemory(hProcess, (BYTE*)databaseAddr, &database, sizeof(database), nullptr);

      BYTE *playerInfoAddr = (BYTE*)databaseAddr + 0x28; // offset 0x28 acquired from cheatengine
      unsigned int playerVectorAddr = 0;
      BYTE playerInfoData[0x200];

      // capture playerinfo memory for debugging
      // todo: remove
      ReadProcessMemory(hProcess, (BYTE*)playerInfoAddr, &playerInfoData, sizeof(playerInfoData), nullptr);

      // get the player vector address
      ReadProcessMemory(hProcess, (BYTE*)playerInfoAddr, &playerVectorAddr, sizeof(playerVectorAddr), nullptr);

      // capture player vector memory for debugging
      // todo: remove
      ReadProcessMemory(hProcess, (BYTE*)playerVectorAddr, &playerInfoData, sizeof(playerInfoData), nullptr);
      unsigned int writeOffset = 0;
      unsigned int writeVal = 0;
      unsigned int readVal = 0;

      for (unsigned int i = 0; i < numSpeedVars; ++i)
      {
        writeOffset = playerVectorAddr + speedData[i].offset;
        writeVal = *(unsigned int*)&speedData[i].value;

        // read existing value
        success = ReadProcessMemory(hProcess, (BYTE*)writeOffset, &readVal, sizeof(readVal), nullptr);
        while (success == FALSE)
        {
          Sleep(2);
          success = ReadProcessMemory(hProcess, (BYTE*)writeOffset, &readVal, sizeof(readVal), nullptr);
        }

        // overwrite with new value if matching
        if (readVal == fiftyHex)
        {
          success = WriteProcessMemory(hProcess, (BYTE*)writeOffset, &writeVal, sizeof(writeVal), nullptr);
          while (success == FALSE)
          {
            Sleep(2);
            success = WriteProcessMemory(hProcess, (BYTE*)writeOffset, &writeVal, sizeof(writeVal), nullptr);
          }
        }
        Sleep(2);
      }
    }

    if (success == TRUE)
    {
      printf("speed mod added\n");
    }

    return success;
  }
}