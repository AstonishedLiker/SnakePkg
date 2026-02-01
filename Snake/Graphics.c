/** @file
  UEFI Snake Game

  Copyright (c) 2026 Alexis Lecam <alexis.lecam@hexaliker.fr>

  SPDX-License-Identifier: MIT
**/

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/HiiImage.h>
#include <Protocol/HiiDatabase.h>

#include "Graphics.h"

EFI_GRAPHICS_OUTPUT_PROTOCOL            *gGop;
EFI_GRAPHICS_OUTPUT_MODE_INFORMATION    *gGopInfo;
STATIC EFI_GRAPHICS_OUTPUT_BLT_PIXEL    *gBackBuffer;
EFI_HII_IMAGE_PROTOCOL                  *gHiiImage;
EFI_HII_HANDLE                          gHiiHandle;
STATIC EFI_HII_DATABASE_PROTOCOL        *mHiiDatabase;
STATIC UINTN                            gBackBufferLen;
UINTN                                   gMiddleScreenX;
UINTN                                   gMiddleScreenY;

BOOLEAN
InitGfx(
  VOID
)
{
  EFI_STATUS Status;
  EFI_HII_PACKAGE_LIST_HEADER   *PackageListHeader;

  Status = gBS->OpenProtocol (
    gImageHandle,
    &gEfiHiiPackageListProtocolGuid,
    (VOID **)&PackageListHeader,
    gImageHandle,
    NULL,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
  );
  ASSERT_EFI_ERROR(Status);

  Status = gBS->LocateProtocol(
    &gEfiHiiImageProtocolGuid,
    NULL,
    (VOID **)&gHiiImage
  );
  ASSERT_EFI_ERROR(Status);

  Status = gBS->LocateProtocol(
    &gEfiHiiDatabaseProtocolGuid,
    NULL,
    (VOID **)&mHiiDatabase
  );
  ASSERT_EFI_ERROR(Status);

  Status = gBS->LocateProtocol(
    &gEfiGraphicsOutputProtocolGuid,
    NULL,
    (VOID **)&gGop
  );
  ASSERT_EFI_ERROR(Status);

  Status = mHiiDatabase->NewPackageList(
    mHiiDatabase,
    PackageListHeader,
    NULL,
    &gHiiHandle
  );
  ASSERT_EFI_ERROR(Status);

  gGopInfo = gGop->Mode->Info;
  gBackBufferLen = gGopInfo->HorizontalResolution * gGopInfo->VerticalResolution * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  gBackBuffer = AllocateZeroPool(gBackBufferLen);
  ASSERT(gBackBuffer);

  gMiddleScreenX = (gGopInfo->HorizontalResolution / 2) - 1;
  gMiddleScreenY = (gGopInfo->VerticalResolution / 2) - 1;

  return TRUE;
}

VOID
DrawRectangleToBackbuffer(
  IN UINT8  Red,
  IN UINT8  Green,
  IN UINT8  Blue,
  IN UINTN  Width,
  IN UINTN  Height,
  IN UINTN  DestinationX,
  IN UINTN  DestinationY
) {
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   Pixel;
  UINTN                           y;
  UINTN                           x;
  UINTN                           ScreenX;
  UINTN                           ScreenY;
  UINTN                           Index;

  Pixel = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL){
    .Blue = Blue,
    .Green = Green,
    .Red = Red,
    .Reserved = 0
  };

  for (y = 0; y < Height; y++) {
    for (x = 0; x < Width; x++) {
      ScreenX = DestinationX + x;
      ScreenY = DestinationY + y;

      ASSERT(ScreenX < gGopInfo->HorizontalResolution && 
        ScreenY < gGopInfo->VerticalResolution);

      Index = ScreenY * gGopInfo->HorizontalResolution + ScreenX;
      ASSERT(Index * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) < gBackBufferLen);

      gBackBuffer[Index] = Pixel;
    }
  }
}

VOID
DrawImageToBackbuffer(
  IN EFI_IMAGE_INPUT    *Image,
  IN UINTN              DestinationX,
  IN UINTN              DestinationY,
  IN BOOLEAN            IsAnchorMiddle
)
{
  UINTN   ActualX;
  UINTN   ActualY;
  UINTN   y;
  UINTN   x;
  UINTN   ScreenX;
  UINTN   ScreenY;

  ActualX = (!IsAnchorMiddle) ? DestinationX : (gGopInfo->HorizontalResolution / 2) - (Image->Width / 2);
  ActualY = (!IsAnchorMiddle) ? DestinationY : (gGopInfo->VerticalResolution / 2) - (Image->Height / 2);

  for (y = 0; y < Image->Height; y++) {
    for (x = 0; x < Image->Width; x++) {
      ScreenX = ActualX + x;
      ScreenY = ActualY + y;

      ASSERT(ScreenX < gGopInfo->HorizontalResolution && 
        ScreenY < gGopInfo->VerticalResolution);

      gBackBuffer[ScreenY * gGopInfo->HorizontalResolution + ScreenX] = Image->Bitmap[y * Image->Width + x];
    }
  }
}

VOID
PresentBackbuffer(
  VOID
)
{
  EFI_STATUS  Status;

  Status = gGop->Blt(
    gGop,
    gBackBuffer,
    EfiBltBufferToVideo,
    0, 0,
    0, 0,
    gGopInfo->HorizontalResolution, gGopInfo->VerticalResolution,
    0
  );

  ASSERT_EFI_ERROR(Status);
}

VOID
ClearBackbuffer(
  VOID
)
{
  DrawRectangleToBackbuffer(
    0, 0, 0,
    gGopInfo->HorizontalResolution, gGopInfo->VerticalResolution,
    0, 0
  );
}

VOID
DeinitGfx(
  VOID
)
{
  EFI_STATUS    Status;

  // Important!
  Status = mHiiDatabase->RemovePackageList(
    mHiiDatabase,
    gHiiHandle
  );
  ASSERT_EFI_ERROR(Status);

  Status = gBS->CloseProtocol(
    gImageHandle,
    &gEfiHiiPackageListProtocolGuid,
    gImageHandle,
    NULL
  );
  ASSERT_EFI_ERROR(Status);

  FreePool(gBackBuffer);
  gBackBufferLen = 0;
}
