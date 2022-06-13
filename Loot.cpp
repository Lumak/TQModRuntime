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

#include "Loot.h"

//=============================================================================
//=============================================================================
namespace Loot
{
  BOOL Setup(HANDLE hProcess, uintptr_t gameDllBase, unsigned int databaseAddr)
  {
    BOOL success = TRUE;

    //mod LootBase::SetNoBrokenItems routine
    //001C7D10 ?SetNoBrokenItems@LootBase@GAME@@UAEX_N@Z
    //the routine is short enough to copy the entire thing
    //LootBase::SetNoBrokenItems     - 8A 44 24 04 - mov al, [esp + 04]
    //*replace above with            - B0 01         mov al,1
    //LootBase::SetNoBrokenItems + 4 - 88 41 24    - mov[ecx + 24], al
    //LootBase::SetNoBrokenItems + 7 - C2 0400     - ret 0004 { 4 }
    uintptr_t setNoBrokenItemsAddr = gameDllBase + 0x001C7D10;
    BYTE noBrokenReplaceOpcode[8] = { 0xB0, 0x01, 0x88, 0x41, 0x24, 0xC2, 0x04, 0x00 };
    success = WriteProcessMemory(hProcess, (BYTE*)setNoBrokenItemsAddr, &noBrokenReplaceOpcode, sizeof(noBrokenReplaceOpcode), nullptr);
    while (success == FALSE)
    {
      Sleep(2);
      success = WriteProcessMemory(hProcess, (BYTE*)setNoBrokenItemsAddr, &noBrokenReplaceOpcode, sizeof(noBrokenReplaceOpcode), nullptr);
    }

    //mod LootItemTableRandomizer::SetNoBrokenItems
    //001C83B0 ? SetNoBrokenItems@LootItemTableRandomizer@GAME@@UAEXXZ
    //LootItemTableRandomizer::SetNoBrokenItems     - 8B 41 7C          - mov eax, [ecx + 7C]
    //LootItemTableRandomizer::SetNoBrokenItems + 3 - 01 41 70          - add[ecx + 70], eax
    //LootItemTableRandomizer::SetNoBrokenItems + 6 - C7 41 7C 00000000 - mov[ecx + 7C], 00000000 { 0 }
    //*replace above with                           C7 41 7C 01000000 - mov[ecx + 7C], 01000000 { 1 }
    setNoBrokenItemsAddr = gameDllBase + 0x001C83B0;
    BYTE noBrokenReplaceOpcode2[14] = { 0x8B, 0x41, 0x7C, 0x01, 0x41, 0x70, 0xC7, 0x41, 0x7C, 0x01, 0x00, 0x00, 0x00, 0xC3 };
    success = WriteProcessMemory(hProcess, (BYTE*)setNoBrokenItemsAddr, &noBrokenReplaceOpcode2, sizeof(noBrokenReplaceOpcode2), nullptr);
    while (success == FALSE)
    {
      Sleep(2);
      success = WriteProcessMemory(hProcess, (BYTE*)setNoBrokenItemsAddr, &noBrokenReplaceOpcode2, sizeof(noBrokenReplaceOpcode2), nullptr);
    }

    //mod OneShot_Gold::SetGoldValue
    //001FDB60 ?SetGoldValue@OneShot_Gold@GAME@@UAEXM@Z
    uintptr_t setGoldAddr = gameDllBase + 0x001FDB60;
    BYTE setGoldOpcode[0x3F]; // address end to int3 opcode from cheatengine
    success = ReadProcessMemory(hProcess, (BYTE*)setGoldAddr, &setGoldOpcode, sizeof(setGoldOpcode), nullptr);
    while (success == FALSE)
    {
      Sleep(2);
      success = ReadProcessMemory(hProcess, (BYTE*)setGoldAddr, &setGoldOpcode, sizeof(setGoldOpcode), nullptr);
    }
    if (setGoldOpcode[0x21] == 0x89)
    {
      // shift the buffer by 3 bytes to the right for opcodes insertion
      const int shiftSize = 3;
      for (int i = 0x36; i >= 0x21; --i)
      {
        setGoldOpcode[i + shiftSize] = setGoldOpcode[i];
      }

      // insert shl by 6 (or multiply by 64); C1 E0 06 - shl eax,6
      setGoldOpcode[0x21] = 0xC1;
      setGoldOpcode[0x22] = 0xE0;
      setGoldOpcode[0x23] = 0x06;
      success = WriteProcessMemory(hProcess, (BYTE*)setGoldAddr, &setGoldOpcode, sizeof(setGoldOpcode), nullptr);
      while (success == FALSE)
      {
        Sleep(2);
        success = WriteProcessMemory(hProcess, (BYTE*)setGoldAddr, &setGoldOpcode, sizeof(setGoldOpcode), nullptr);
      }

      printf("gold drop mod added\n");
    }

    //------------------
    //NOTE: the following two constructors are never called at runtime, hence, they're ignored
    //001C7C90 ??0LootBase@GAME@@QAE@XZ
    //001CADB0 ??0LootRandomizerTable@GAME@@QAE@XZ
    //------------------

    //mod GAME::0LootMasterTable
    //001CA6D0 ??0LootMasterTable@GAME@@QAE@XZ
    uintptr_t lootMasterTableAddr = gameDllBase + 0x001CA6D0;
    BYTE lootMasterTableOpcode[0x5d]; // address from cheatengine
    success = ReadProcessMemory(hProcess, (BYTE*)lootMasterTableAddr, &lootMasterTableOpcode, sizeof(lootMasterTableOpcode), nullptr);
    while (success == FALSE)
    {
      Sleep(2);
      success = ReadProcessMemory(hProcess, (BYTE*)lootMasterTableAddr, &lootMasterTableOpcode, sizeof(lootMasterTableOpcode), nullptr);
    }
    // mastertable default:
    //+0D - C7 46 28 00000000 - mov[esi + 28], 00000000 { 0 }
    //+14 - C7 46 2C 00000000 - mov[esi + 2C], 00000000 { 0 }
    //+1B - C7 46 30 00000000 - mov[esi + 30], 00000000 { 0 }
    //+22 - C7 46 34 00000000 - mov[esi + 34], 00000000 { 0 }
    //+29 - C7 46 38 00000000 - mov[esi + 38], 00000000 { 0 }
    if (lootMasterTableOpcode[0x20] == 0x0)
    {
      lootMasterTableOpcode[0x10] = 0x01; // 
      lootMasterTableOpcode[0x18] = 0x40; // 
      lootMasterTableOpcode[0x1f] = 0x01; // 
      lootMasterTableOpcode[0x26] = 0x01; // 
      lootMasterTableOpcode[0x2d] = 0x00; // 
      success = WriteProcessMemory(hProcess, (BYTE*)lootMasterTableAddr, &lootMasterTableOpcode, sizeof(lootMasterTableOpcode), nullptr);
      while (success == FALSE)
      {
        Sleep(2);
        success = WriteProcessMemory(hProcess, (BYTE*)lootMasterTableAddr, &lootMasterTableOpcode, sizeof(lootMasterTableOpcode), nullptr);
      }
    }

    //mod GAME::LootItemTable_DynWeight
    //001C8D60 ??0LootItemTable_DynWeight@GAME@@QAE@XZ --- ** monster loot
    uintptr_t lootDynamicWeightAddr = gameDllBase + 0x001C8D60;
    BYTE lootDynWeightOpcode[0x40]; // just to the end of the array vectors
    success = ReadProcessMemory(hProcess, (BYTE*)lootDynamicWeightAddr, &lootDynWeightOpcode, sizeof(lootDynWeightOpcode), nullptr);
    while (success == FALSE)
    {
      Sleep(2);
      success = ReadProcessMemory(hProcess, (BYTE*)lootDynamicWeightAddr, &lootDynWeightOpcode, sizeof(lootDynWeightOpcode), nullptr);
    }
    // dyn weight default:
    //+0D - C7 46 28 00000000 - mov[esi + 28], 00000000 { 0 }
    //+14 - C7 46 2C 00000000 - mov[esi + 2C], 00000000 { 0 }
    //+1B - C7 46 30 00000000 - mov[esi + 30], 00000000 { 0 }
    //+22 - C7 46 34 00000000 - mov[esi + 34], 00000000 { 0 }
    //+29 - C7 46 38 00000000 - mov[esi + 38], 00000000 { 0 }
    if (lootDynWeightOpcode[0x1f] == 0x00)
    {
      // final: [0x18] = 0x20, after numerous tests, the below setup gives the best green/yellow drop combos
      lootDynWeightOpcode[0x10] = 0x00;
      lootDynWeightOpcode[0x18] = 0x20;
      lootDynWeightOpcode[0x1E] = 0x00;
      lootDynWeightOpcode[0x25] = 0x00;
      lootDynWeightOpcode[0x2C] = 0x00;

      success = WriteProcessMemory(hProcess, (BYTE*)lootDynamicWeightAddr, &lootDynWeightOpcode, sizeof(lootDynWeightOpcode), nullptr);
      while (success == FALSE)
      {
        Sleep(2);
        success = WriteProcessMemory(hProcess, (BYTE*)lootDynamicWeightAddr, &lootDynWeightOpcode, sizeof(lootDynWeightOpcode), nullptr);
      }
    }

    //mod GAME::LootItemTable_FixedWeight
    //001C98D0 ??0LootItemTable_FixedWeight@GAME@@QAE@XZ
    uintptr_t lootFixedWeightAddr = gameDllBase + 0x001C98D0;
    BYTE lootFixedWeightOpcode[0xC3]; // address from cheatengine
    success = ReadProcessMemory(hProcess, (BYTE*)lootFixedWeightAddr, &lootFixedWeightOpcode, sizeof(lootFixedWeightOpcode), nullptr);
    while (success == FALSE)
    {
      Sleep(2);
      success = ReadProcessMemory(hProcess, (BYTE*)lootFixedWeightAddr, &lootFixedWeightOpcode, sizeof(lootFixedWeightOpcode), nullptr);
    }
    // fixed weight default:
    //+0D - C7 46 28 00000000 - mov[esi + 28], 00000000 { 0 }
    //+14 - C7 46 2C 00000000 - mov[esi + 2C], 00000000 { 0 }
    //+1B - C7 46 30 00000000 - mov[esi + 30], 00000000 { 0 }
    //+22 - C7 46 34 00000000 - mov[esi + 34], 00000000 { 0 }
    //+29 - C7 46 38 00000000 - mov[esi + 38], 00000000 { 0 }
    if (lootFixedWeightOpcode[0x1f] == 0x0)
    {
      lootFixedWeightOpcode[0x10] = 0x00; // 
      lootFixedWeightOpcode[0x18] = 0x40; // 
      lootFixedWeightOpcode[0x1f] = 0x00; // 
      lootFixedWeightOpcode[0x26] = 0x00; // 
      lootFixedWeightOpcode[0x2C] = 0x00; // 

      success = WriteProcessMemory(hProcess, (BYTE*)lootFixedWeightAddr, &lootFixedWeightOpcode, sizeof(lootFixedWeightOpcode), nullptr);
      while (success == FALSE)
      {
        Sleep(2);
        success = WriteProcessMemory(hProcess, (BYTE*)lootFixedWeightAddr, &lootFixedWeightOpcode, sizeof(lootFixedWeightOpcode), nullptr);
      }
    }

    //mod LootBase::SetRandomizerWeightModifiers fn
    //and finally, override SetRandomizerWeightModifiers to prevent wiping out our settings
    //001C7D20 ? SetRandomizerWeightModifiers@LootBase@GAME@@UAEXABURandomizerWeightModifiers@2@@Z
    //*return is inserted at the top of the function
    //SetRandomizerWeightModifiers - 8B 44 24 04 - mov eax, [esp + 04]
    //change to:                     C2 0400     - ret 0004 { 4 }
    uintptr_t setRandomWeightAddr = gameDllBase + 0x001C7D20;
    BYTE setRandWeightOpcode[0x4]; // overwrite the 4 bytes
    success = ReadProcessMemory(hProcess, (BYTE*)setRandomWeightAddr, &setRandWeightOpcode, sizeof(setRandWeightOpcode), nullptr);
    while (success == FALSE)
    {
      Sleep(2);
      success = ReadProcessMemory(hProcess, (BYTE*)setRandomWeightAddr, &setRandWeightOpcode, sizeof(setRandWeightOpcode), nullptr);
    }
    if (setRandWeightOpcode[0x00] == 0x8B) // 1st opcode (see above)
    {
      setRandWeightOpcode[0x00] = 0xC2;
      setRandWeightOpcode[0x01] = 0x04;
      setRandWeightOpcode[0x02] = 0x00;
      setRandWeightOpcode[0x03] = 0xCC; //int 3 

      success = WriteProcessMemory(hProcess, (BYTE*)setRandomWeightAddr, &setRandWeightOpcode, sizeof(setRandWeightOpcode), nullptr);
      while (success == FALSE)
      {
        Sleep(2);
        success = WriteProcessMemory(hProcess, (BYTE*)setRandomWeightAddr, &setRandWeightOpcode, sizeof(setRandWeightOpcode), nullptr);
      }
    }
    if (success == TRUE)
    {
      printf("loot drop mod added\n");
    }

    return success;
  }
}