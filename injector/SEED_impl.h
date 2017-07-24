/*****************************************************************************
HGWC emulator - zyan-dev.com (c) 2012
------------------------------------------------------------------------------
Author:        Ende!
Contact:       ende@zyan-dev.com
Version:       v1.0
******************************************************************************/

#pragma once
#include <Windows.h>

// ============================================================================
// Prototypes for functions implemented in ASM
   
extern "C"
{
   void __stdcall SEEDEncryptDataBlock (const void *pKey, void *pData);
   void __stdcall SEEDDecryptDataBlock (const void *pKey, void *pData);
   void __stdcall SEEDMakeCryptKey     (const void *pShortKey, void *pLongKey);
} // ==> extern "C"

// ============================================================================
