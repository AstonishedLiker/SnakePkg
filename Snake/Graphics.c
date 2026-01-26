/**
  UEFI Snake Game

  Copyright (c) 2026 Alexis Lecam <alexis.lecam@hexaliker.fr>

  SPDX-License-Identifier: MIT
**/

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include "BmpSupport.h"
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

#define BMP_CACHE_SIZE 5
STATIC BMP_CACHE_ENTRY mBmpCache[BMP_CACHE_SIZE];
STATIC UINTN           mBmpCacheCount = 0;

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

  if (EFI_ERROR(Status)) {
    Print(L"%a: Failed to locate GOP: %r\n", __FUNCTION__, Status);
    return FALSE;
  }

  gGopInfo = gGop->Mode->Info;
  gBackBufferLen = gGopInfo->HorizontalResolution * gGopInfo->VerticalResolution;
  gBackBuffer = AllocatePool(gBackBufferLen);
  mBmpCacheCount = 0;

  ZeroMem(gBackBuffer, gBackBufferLen);
  ZeroMem(mBmpCache, sizeof(mBmpCache));

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

      gBackBuffer[ScreenY * gGopInfo->HorizontalResolution + ScreenX] = Pixel;
    }
  }
}

STATIC
BMP_CACHE_ENTRY *
FindBmpInCache(
  IN VOID *BmpPointer
)
{
  for (UINTN i = 0; i < mBmpCacheCount; i++) {
    if (mBmpCache[i].BmpPointer == BmpPointer) {
      return &mBmpCache[i];
    }
  }
  return NULL;
}

STATIC
BOOLEAN
AddBmpToCache(
  IN VOID                          *BmpPointer,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer,
  IN UINTN                         BltBufferSize,
  IN UINTN                         Width,
  IN UINTN                         Height
)
{
  // For the amount of images I have, I don't want to implement any eviction algorithm... Would be overengineering.
  if (mBmpCacheCount >= BMP_CACHE_SIZE) {
    Print(L"%a: Warning: BMP cache is full\n", __FUNCTION__);
    return FALSE;
  }

  mBmpCache[mBmpCacheCount].BmpPointer = BmpPointer;
  mBmpCache[mBmpCacheCount].BltBuffer = BltBuffer;
  mBmpCache[mBmpCacheCount].BltBufferSize = BltBufferSize;
  mBmpCache[mBmpCacheCount].Width = Width;
  mBmpCache[mBmpCacheCount].Height = Height;
  mBmpCacheCount++;

  return TRUE;
}

BOOLEAN
DrawBmpToBackbuffer(
  IN VOID     *BmpImage,
  IN UINTN    BmpImageLen,
  IN UINTN    DestinationX,
  IN UINTN    DestinationY,
  IN BOOLEAN  IsAnchorMiddle
)
{
  EFI_STATUS                    Status;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer;
  UINTN                         BltBufferSize;
  UINTN                         BmpHeight;
  UINTN                         BmpWidth;
  BMP_CACHE_ENTRY               *CacheEntry;
  BOOLEAN                       FromCache;
  UINTN                         ActualX;
  UINTN                         ActualY;
  UINTN                         y;
  UINTN                         x;
  UINTN                         ScreenX;
  UINTN                         ScreenY;

  CacheEntry = FindBmpInCache(BmpImage);

  if (CacheEntry != NULL) {
    BltBuffer = CacheEntry->BltBuffer;
    BltBufferSize = CacheEntry->BltBufferSize;
    BmpWidth = CacheEntry->Width;
    BmpHeight = CacheEntry->Height;
    FromCache = TRUE;
  } else {
    Status = TranslateBmpToGopBlt(
      BmpImage,
      BmpImageLen,
      &BltBuffer,
      &BltBufferSize,
      &BmpHeight,
      &BmpWidth
    );

    if (EFI_ERROR(Status)) {
      Print(L"%a: Failed to translate BMP to GOP blt: %r\n", __FUNCTION__, Status);
      return FALSE;
    }

    if (!AddBmpToCache(BmpImage, BltBuffer, BltBufferSize, BmpWidth, BmpHeight)) {
      FromCache = FALSE;
    } else {
      FromCache = TRUE;
    }
  }

  ActualX = (!IsAnchorMiddle) ? DestinationX : (gGopInfo->HorizontalResolution / 2) - (BmpWidth / 2);
  ActualY = (!IsAnchorMiddle) ? DestinationY : (gGopInfo->VerticalResolution / 2) - (BmpHeight / 2);

  for (y = 0; y < BmpHeight; y++) {
    for (x = 0; x < BmpWidth; x++) {
      ScreenX = ActualX + x;
      ScreenY = ActualY + y;

      ASSERT(ScreenX < gGopInfo->HorizontalResolution && 
        ScreenY < gGopInfo->VerticalResolution);

      gBackBuffer[ScreenY * gGopInfo->HorizontalResolution + ScreenX] = BltBuffer[y * BmpWidth + x];
    }
  }

  if (!FromCache) {
    FreePool(BltBuffer);
  }

  return !EFI_ERROR(Status);
}

BOOLEAN
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

  if (EFI_ERROR(Status)) {
    Print(L"%a: Failed to Blt: %r\n", __FUNCTION__, Status);
    return FALSE;
  }

  return TRUE;
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
  UINTN i;

  FreePool(gBackBuffer);
  gBackBufferLen = 0;

  for (i = 0; i < mBmpCacheCount; i++) {
    if (mBmpCache[i].BltBuffer != NULL) {
      FreePool(mBmpCache[i].BltBuffer);
      mBmpCache[i].BltBuffer = NULL;
    }
  }

  mBmpCacheCount = 0;
}
