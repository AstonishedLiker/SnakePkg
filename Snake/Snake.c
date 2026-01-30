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
#include <Protocol/HiiImage.h>
#include <Protocol/HiiDatabase.h>

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
  EFI_HII_PACKAGE_LIST_HEADER   *PackageListHeader;
  EFI_HII_IMAGE_PROTOCOL        *HiiImage;
  EFI_HII_DATABASE_PROTOCOL     *HiiDatabase;
  EFI_HII_HANDLE                HiiHandle;
  BOOLEAN                       Success;
  EFI_IMAGE_INPUT               Image;

  Success = InitGfx();
  ASSERT(Success);

  Status = gBS->OpenProtocol(
    ImageHandle,
    &gEfiLoadedImageProtocolGuid,
    (VOID **)&LoadedImage,
    ImageHandle,
    NULL,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
  );

  ASSERT_EFI_ERROR(Status);

  Status = gBS->OpenProtocol (
    ImageHandle,
    &gEfiHiiPackageListProtocolGuid,
    (VOID **)&PackageListHeader,
    ImageHandle,
    NULL,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
  );

  ASSERT_EFI_ERROR(Status);

  Print(L"LoadedImage->ImageBase: 0x%p\n", LoadedImage->ImageBase);
  DEBUG_CODE(gBS->Stall(1e6);); // Wait until debugger properly attaches

  Status = gBS->LocateProtocol(
    &gEfiHiiImageProtocolGuid,
    NULL,
    (VOID **)&HiiImage
  );
  ASSERT_EFI_ERROR(Status);

  Status = gBS->LocateProtocol(
    &gEfiHiiDatabaseProtocolGuid,
    NULL,
    (VOID **)&HiiDatabase
  );
  ASSERT_EFI_ERROR(Status);

  Print(L"Protocols located & opened...\n");

  Status = HiiDatabase->NewPackageList(
    HiiDatabase,
    PackageListHeader,
    NULL,
    &HiiHandle
  );
  ASSERT_EFI_ERROR(Status);
  ASSERT(HiiHandle);

  Status = HiiImage->GetImage(HiiImage, HiiHandle, IMAGE_TOKEN(IMG_LOGO), &Image);
  ASSERT_EFI_ERROR(Status);

  DrawRectangleToBackbuffer(
    60, 60, 60,
    gGopInfo->HorizontalResolution, gGopInfo->VerticalResolution,
    0, 0
  );

  Print(L"Gop: 0x%p\n", gGop);
  Print(L"Gop->Mode->Mode: %d\n", gGop->Mode->Mode);
  Print(L"Info->HorizontalResolution: %d\n", gGopInfo->HorizontalResolution);
  Print(L"Info->VerticalResolution: %d\n", gGopInfo->VerticalResolution);

  DrawImageToBackbuffer(
    &Image,
    (gGopInfo->HorizontalResolution / 2),
    (gGopInfo->VerticalResolution / 2),
    TRUE
  );

  gBS->Stall(2e6);
  PresentBackbuffer();
  gBS->Stall(5e6);

  ClearBackbuffer();

  // Cleanup
  DeinitGfx();

  Status = gBS->CloseProtocol(
    ImageHandle,
    &gEfiLoadedImageProtocolGuid,
    ImageHandle,
    NULL
  );
  ASSERT_EFI_ERROR(Status);

  Status = gBS->CloseProtocol(
    ImageHandle,
    &gEfiHiiPackageListProtocolGuid,
    ImageHandle,
    NULL
  );
  ASSERT_EFI_ERROR(Status);

  return Status;
}
