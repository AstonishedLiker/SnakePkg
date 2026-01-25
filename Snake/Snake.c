/**
  UEFI Snake Game

  Copyright (c) 2026 Alexis Lecam <alexis.lecam@hexaliker.fr>

  SPDX-License-Identifier: MIT
**/

#include <Uefi.h>

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "Assets/Logo.bmp.h"
#include "Graphics.h"
#include "Uefi/UefiBaseType.h"

EFI_STATUS
EFIAPI
UefiMain(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
)
{
  BOOLEAN                       Success;

  Success = InitGfx();
  if (!Success) {
    return EFI_ABORTED;
  }

  Success = DrawRectangle(
    60, 60, 60,
    gGopInfo->HorizontalResolution, gGopInfo->VerticalResolution,
    0, 0
  );

  if (!Success) {
    return EFI_ABORTED;
  }

  Print(L"Gop: 0x%p\n", gGop);
  Print(L"Gop->Mode->Mode: %d\n", gGop->Mode->Mode);
  Print(L"Info->HorizontalResolution: %d\n", gGopInfo->HorizontalResolution);
  Print(L"Info->VerticalResolution: %d\n", gGopInfo->VerticalResolution);
  Print(L"\nNo snake game yet! Exiting in 10 seconds...");

  Success = DrawBmp(
    LogoBmp,
    LogoBmpLen,
    (gGopInfo->HorizontalResolution / 2),
    (gGopInfo->VerticalResolution / 2),
    TRUE
  );

  if (!Success) {
    return EFI_ABORTED;
  }

  gBS->Stall(10e6); // 10s
  CleanupBmpCache();

  return EFI_SUCCESS;
}
