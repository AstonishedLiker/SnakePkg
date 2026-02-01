/** @file
  UEFI Snake Game

  Copyright (c) 2026 Alexis Lecam <alexis.lecam@hexaliker.fr>

  SPDX-License-Identifier: MIT
**/

#include <Uefi.h>

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/HiiImage.h>
#include <Protocol/HiiDatabase.h>

#include "GameLogic.h"
#include "Graphics.h"

EFI_STATUS
EFIAPI
UefiMain(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
)
{
  EFI_STATUS                    Status;
  EFI_LOADED_IMAGE_PROTOCOL     *LoadedImage;
  BOOLEAN                       Success;

  Status = gBS->OpenProtocol(
    ImageHandle,
    &gEfiLoadedImageProtocolGuid,
    (VOID **)&LoadedImage,
    ImageHandle,
    NULL,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
  );
  ASSERT_EFI_ERROR(Status);

  Success = InitGfx();
  ASSERT(Success);

  Print(L"LoadedImage->ImageBase: 0x%p\n", LoadedImage->ImageBase);
  DEBUG_CODE(gBS->Stall(1e6);); // Wait until debugger properly attaches

  Print(L"Protocols located & opened!\n");
  Print(L"Gop: 0x%p\n", gGop);
  Print(L"Gop->Mode->Mode: %d\n", gGop->Mode->Mode);
  Print(L"Info->HorizontalResolution: %d\n", gGopInfo->HorizontalResolution);
  Print(L"Info->VerticalResolution: %d\n", gGopInfo->VerticalResolution);

  RunGameLogic();

  // Cleanup  
  ClearBackbuffer();
  DeinitGfx();

  Status = gBS->CloseProtocol(
    ImageHandle,
    &gEfiLoadedImageProtocolGuid,
    ImageHandle,
    NULL
  );
  ASSERT_EFI_ERROR(Status);

  Print(L"Reseting system in 4 seconds!");
  gBS->Stall(4e6);

  gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
  CpuDeadLoop();

  return Status;
}
