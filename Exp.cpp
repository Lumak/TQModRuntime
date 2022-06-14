#include "Exp.h"

namespace Exp
{
  BOOL Setup(HANDLE hProcess, uintptr_t gameDllBase)
  {
    BOOL success = TRUE;

    //mod experience gain
    //001A0EC0 ?CalculateExperienceReward@GameEngine@GAME@@QAEIIII@Z
    //only mod to multiply exp
    //CalculateExperienceReward + BC - 8B 04 24 - mov eax, [esp]
    //*insert                        - C1 E0 05 - shl eax,1
    //CalculateExperienceReward + BF - D9 6C 24 14 - fldcw[esp + 14]
    //CalculateExperienceReward + C3 - 83 C4 08 - add esp, 08 { 8 }
    //CalculateExperienceReward + C6 - C2 0C00 - ret 000C{ 12 }
    //CalculateExperienceReward + C9 - 8B C7 - mov eax, edi
    //CalculateExperienceReward + CB - 5F - pop edi
    //CalculateExperienceReward + CC - 5E - pop esi
    //CalculateExperienceReward + CD - 83 C4 08 - add esp, 08 { 8 }
    //CalculateExperienceReward + D0 - C2 0C00 - ret 000C{ 12 }
    //CalculateExperienceReward + D3 - CC - int 3
    uintptr_t calcExpAddr = gameDllBase + 0x001A0EC0 + 0xBC;
    const unsigned int opcodeSize = 0xD3 - 0xBC;
    BYTE calcExpOpcode[opcodeSize + 3];

    success = ReadProcessMemory(hProcess, (BYTE*)calcExpAddr, &calcExpOpcode, sizeof(calcExpOpcode), nullptr);
    while (success == FALSE)
    {
      Sleep(2);
      success = ReadProcessMemory(hProcess, (BYTE*)calcExpAddr, &calcExpOpcode, sizeof(calcExpOpcode), nullptr);
    }
    if (calcExpOpcode[0x3] == 0xD9)
    {
      // shift the buffer by 3 bytes to the right for opcode insertion
      const int shiftSize = 3;
      for (int i = opcodeSize; i >= 0x3; --i)
      {
        calcExpOpcode[i + shiftSize] = calcExpOpcode[i];
      }

      // insert shl by 1 (or multiply by 2); C1 E0 05 - shl eax,1
      calcExpOpcode[0x03] = 0xC1;
      calcExpOpcode[0x04] = 0xE0;
      calcExpOpcode[0x05] = 0x01;

      success = WriteProcessMemory(hProcess, (BYTE*)calcExpAddr, &calcExpOpcode, sizeof(calcExpOpcode), nullptr);
      while (success == FALSE)
      {
        Sleep(2);
        success = WriteProcessMemory(hProcess, (BYTE*)calcExpAddr, &calcExpOpcode, sizeof(calcExpOpcode), nullptr);
      }

      printf("exp mod added\n");
    }

    return success;
  }
}