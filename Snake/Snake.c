/**
  UEFI Snake Game

  Copyright (c) 2026 Alexis Lecam <alexis.lecam@hexaliker.fr>

  SPDX-License-Identifier: MIT
**/

#include <Uefi.h>

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/LoadedImage.h>

#include "Assets/Logo.bmp.h"
#include "Graphics.h"

EFI_STATUS
EFIAPI
UefiMain(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
)
{
  EFI_STATUS                  Status;
  EFI_LOADED_IMAGE_PROTOCOL   *LoadedImage;
  BOOLEAN                     Success;

  Status = gBS->LocateProtocol(
    &gEfiLoadedImageProtocolGuid,
    NULL,
    (VOID **)&LoadedImage
  );

  if (EFI_ERROR(Status)) {
    ASSERT(EFI_ERROR(Status));
    return EFI_ABORTED;
  }

  Print(L"LoadedImage->ImageBase: 0x%p\n", LoadedImage->ImageBase);

  Success = InitGfx();
  if (!Success) {
    ASSERT(!Success);
    return EFI_ABORTED;
  }

  Print(L"Graphics initialized...\n");

  DrawRectangleToBackbuffer(
    60, 60, 60,
    gGopInfo->HorizontalResolution, gGopInfo->VerticalResolution,
    0, 0
  );

  Print(L"Gop: 0x%p\n", gGop);
  Print(L"Gop->Mode->Mode: %d\n", gGop->Mode->Mode);
  Print(L"Info->HorizontalResolution: %d\n", gGopInfo->HorizontalResolution);
  Print(L"Info->VerticalResolution: %d\n", gGopInfo->VerticalResolution);

  Success = DrawBmpToBackbuffer(
    LogoBmp,
    sizeof(LogoBmp),
    (gGopInfo->HorizontalResolution / 2),
    (gGopInfo->VerticalResolution / 2),
    TRUE
  );

  if (!Success) {
    ASSERT(!Success);
    return EFI_ABORTED;
  }

  gBS->Stall(2e6); // 2s
  PresentBackbuffer();
  gBS->Stall(10e6); // 10s

  ClearBackbuffer();
  DeinitGfx();

  return EFI_SUCCESS;
}
