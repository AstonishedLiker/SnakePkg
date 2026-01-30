/** @file
  UEFI Snake Game

  Copyright (c) 2026 Alexis Lecam <alexis.lecam@hexaliker.fr>

  SPDX-License-Identifier: MIT
**/

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include "Graphics.h"

EFI_GRAPHICS_OUTPUT_PROTOCOL            *gGop           = NULL;
EFI_GRAPHICS_OUTPUT_MODE_INFORMATION    *gGopInfo       = NULL;
EFI_GRAPHICS_OUTPUT_BLT_PIXEL           *gBackBuffer    = NULL;
UINTN                                   gBackBufferLen;

typedef struct {
  VOID                          *BmpPointer;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer;
  UINTN                         BltBufferSize;
  UINTN                         Width;
  UINTN                         Height;
} BMP_CACHE_ENTRY;

BOOLEAN
InitGfx(
  VOID
)
{
  EFI_STATUS Status;

  Status = gBS->LocateProtocol(
    &gEfiGraphicsOutputProtocolGuid,
    NULL,
    (VOID **)&gGop
  );
  ASSERT_EFI_ERROR(Status);

  gGopInfo = gGop->Mode->Info;
  gBackBufferLen = gGopInfo->HorizontalResolution * gGopInfo->VerticalResolution * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  gBackBuffer = AllocatePool(gBackBufferLen);

  if (!gBackBuffer) {
    Print(L"%a: AllocatePool returned NULL\n", __FUNCTION__);
    CpuBreakpoint();
  }

  ZeroMem(gBackBuffer, gBackBufferLen);

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
      ASSERT(Index < gBackBufferLen);

      gBackBuffer[Index] = Pixel;
    }
  }
}

VOID
DrawImageToBackbuffer(
  IN EFI_IMAGE_INPUT     *Image,
  IN UINTN              DestinationX,
  IN UINTN              DestinationY,
  IN BOOLEAN            IsAnchorMiddle
)
{
  UINTN                         ActualX;
  UINTN                         ActualY;
  UINTN                         y;
  UINTN                         x;
  UINTN                         ScreenX;
  UINTN                         ScreenY;

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
  EFI_STATUS Status;

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
  FreePool(gBackBuffer);
  gBackBufferLen = 0;
}
