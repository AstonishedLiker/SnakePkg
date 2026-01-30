/** @file
  UEFI Snake Game

  Copyright (c) 2026 Alexis Lecam <alexis.lecam@hexaliker.fr>

  SPDX-License-Identifier: MIT
**/

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <Uefi.h>

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImage.h>

extern EFI_GRAPHICS_OUTPUT_PROTOCOL           *gGop;
extern EFI_GRAPHICS_OUTPUT_MODE_INFORMATION   *gGopInfo;

BOOLEAN
InitGfx(
  VOID
);

VOID
DrawRectangleToBackbuffer(
  IN UINT8  Red,
  IN UINT8  Green,
  IN UINT8  Blue,
  IN UINTN  Width,
  IN UINTN  Height,
  IN UINTN  DestinationX,
  IN UINTN  DestinationY
);

VOID
DrawImageToBackbuffer(
  IN EFI_IMAGE_INPUT  *Image,
  IN UINTN            DestinationX,
  IN UINTN            DestinationY,
  IN BOOLEAN          IsAnchorMiddle
);

VOID
PresentBackbuffer(
  VOID
);

VOID
ClearBackbuffer(
  VOID
);

VOID
DeinitGfx(
  VOID
);

#endif // __GRAPHICS_H__
