/**
  UEFI Snake Game

  Copyright (c) 2026 Alexis Lecam <alexis.lecam@hexaliker.fr>

  SPDX-License-Identifier: MIT
**/

#include "Library/BaseMemoryLib.h"
#include <Library/MemoryAllocationLib.h>

#include "BmpSupport.h"
#include "Graphics.h"

EFI_GRAPHICS_OUTPUT_PROTOCOL           *gGop     = NULL;
EFI_GRAPHICS_OUTPUT_MODE_INFORMATION   *gGopInfo = NULL;

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
InitGfx()
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

  ZeroMem(mBmpCache, sizeof(mBmpCache));
  mBmpCacheCount = 0;

  return TRUE;
}

BOOLEAN
DrawRectangle(
  IN UINT8  Red,
  IN UINT8  Green,
  IN UINT8  Blue,
  IN UINTN  Width,
  IN UINTN  Height,
  IN UINTN  DestinationX,
  IN UINTN  DestinationY
) {
  EFI_STATUS                      Status;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   RectangleBuffer;

  RectangleBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL){
    .Blue = Blue,
    .Green = Green,
    .Red = Red,
    .Reserved = 0
  };

  Status = gGop->Blt(
    gGop,
    &RectangleBuffer,
    EfiBltVideoFill,
    0,
    0,
    DestinationX,
    DestinationY,
    Width,
    Height,
    0
  );

  if (EFI_ERROR(Status)) {
    Print(L"%a: Failed to Blt: %r\n", __FUNCTION__, Status);
    return FALSE;
  }

  return TRUE;
}

STATIC
BMP_CACHE_ENTRY *
FindBmpInCache (
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

/**
  Add a BMP to the cache.
  
  @param[in]  BmpPointer     Pointer to the BMP image data.
  @param[in]  BltBuffer      Decoded BLT buffer.
  @param[in]  BltBufferSize  Size of BLT buffer.
  @param[in]  Width          Image width.
  @param[in]  Height         Image height.
  
  @return TRUE if added successfully, FALSE if cache is full.
**/
STATIC
BOOLEAN
AddBmpToCache (
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
DrawBmp(
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

  Status = gGop->Blt(
    gGop,
    BltBuffer,
    EfiBltBufferToVideo,
    0,
    0,
    (!IsAnchorMiddle) ? DestinationX : (gGopInfo->HorizontalResolution / 2) - (BmpWidth / 2),
    (!IsAnchorMiddle) ? DestinationY : (gGopInfo->VerticalResolution / 2) - (BmpHeight / 2),
    BmpWidth,
    BmpHeight,
    0
  );

  if (EFI_ERROR(Status)) {
    Print(L"%a: Failed to Blt: %r\n", __FUNCTION__, Status);
  }

  if (!FromCache) {
    FreePool(BltBuffer);
  }

  return !EFI_ERROR(Status);
}

VOID
CleanupBmpCache (
  VOID
  )
{
  for (UINTN i = 0; i < mBmpCacheCount; i++) {
    if (mBmpCache[i].BltBuffer != NULL) {
      FreePool(mBmpCache[i].BltBuffer);
      mBmpCache[i].BltBuffer = NULL;
    }
  }
  mBmpCacheCount = 0;
}
