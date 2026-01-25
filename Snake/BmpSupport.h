/** @file
  Copyright (c) 2019, TianoCore and contributors.  All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _BMP_SUPPORT_H_
#define _BMP_SUPPORT_H_

#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>

EFI_STATUS
EFIAPI
TranslateBmpToGopBlt(
  IN     VOID                           *BmpImage,
  IN     UINTN                          BmpImageSize,
  IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL  **GopBlt,
  IN OUT UINTN                          *GopBltSize,
  OUT    UINTN                          *PixelHeight,
  OUT    UINTN                          *PixelWidth
);

#endif // _BMP_SUPPORT_H_
