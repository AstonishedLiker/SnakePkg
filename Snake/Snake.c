/** @file
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

  Status = gBS->OpenProtocol(
    ImageHandle,
    &gEfiLoadedImageProtocolGuid,
    (VOID **)&LoadedImage,
    gImageHandle,
    NULL,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
  );

  if (EFI_ERROR(Status)) {
    Status = EFI_ABORTED;
    goto light_cleanup;
  }

  Print(L"LoadedImage->ImageBase: 0x%p\n", LoadedImage->ImageBase);
DEBUG_CODE_BEGIN();
  gBS->Stall(1e6);
  Print(L"NOTE: Compiled in DEBUG mode\n");
DEBUG_CODE_END();

  Success = InitGfx();
  if (!Success) {
    Status = EFI_ABORTED;
    goto light_cleanup;
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
    Status = EFI_ABORTED;
    goto cleanup;
  }

  gBS->Stall(2e6);
  PresentBackbuffer();
  gBS->Stall(5e6);

  ClearBackbuffer();

cleanup:
  DeinitGfx();
light_cleanup:
  gBS->CloseProtocol(
    ImageHandle,
    &gEfiLoadedImageProtocolGuid,
    gImageHandle,
    NULL
  );

  return Status;
}
