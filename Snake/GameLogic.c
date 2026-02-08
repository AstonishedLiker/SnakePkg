#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "GameLogic.h"
#include "Graphics.h"

STATIC UINT32         mRngState = 13371337; // I don't want to make it random per launch
STATIC GRID_MATRIX    mGrid;
STATIC SNAKE_STATE    mSnake;
STATIC UINT32         mScore;

UINT32
XorShift32(
  VOID
)
{
  UINT32 x = mRngState;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  mRngState = x;
  return (UINT32)x;
}

// NOTE: In this function, we return the last ARROW pressed! 
STATIC
BOOLEAN
QueryLastKeystroke(
  EFI_INPUT_KEY   *Key
)
{
  EFI_STATUS      Status;
  UINTN           Index;
  EFI_INPUT_KEY   CurrentKey;
  BOOLEAN         ReadAnyKey = FALSE;

  // We have to execute this twice to make sure we do get the key immediately, and not one snake update late
  for (Index = 0; Index < 2; Index++) {
    while (TRUE) {
      Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &CurrentKey);
      if (EFI_ERROR(Status)) {
        break;
      }

      // We do this so that we can continue reading the next key, otherwise it doesn't seem to read for some reason...
      *Key = CurrentKey;
      CurrentKey = (EFI_INPUT_KEY){};
      ReadAnyKey = TRUE;

      if (
        Key->ScanCode == SCAN_LEFT ||
        Key->ScanCode == SCAN_RIGHT ||
        Key->ScanCode == SCAN_UP ||
        Key->ScanCode == SCAN_DOWN
      ) {
        break;
      }
    }
  }

  return ReadAnyKey;
}

STATIC
VOID
SpawnApple(
  VOID
)
{
  UINT16 EmptyCells[GRID_CELL_COUNT];
  UINT16 EmptyCount = 0;
  UINT16 Index;
  UINT16 ChosenIndex;
  UINT16 GridIndex;

  for (Index = 0; Index < GRID_CELL_COUNT; Index++) {
    if (mGrid[Index] == 0) {
      EmptyCells[EmptyCount++] = Index;
    }
  }

  if (EmptyCount == 0) {
    return;
  }

  ChosenIndex = XorShift32() % EmptyCount;
  GridIndex = EmptyCells[ChosenIndex];

  mGrid[GridIndex] = 2;
}

STATIC
VOID
DrawGrid(
  VOID
)
{
  UINTN   CellDisplaySize;
  UINTN   GridSideDisplaySize;
  UINTN   GridDrawStartAreaX;
  UINTN   GridDrawStartAreaY;
  UINTN   Index;

  CellDisplaySize = (gGopInfo->HorizontalResolution * CELL_PERCENTAGE_SCREEN_OCCUPANCY) / 100;
  GridSideDisplaySize = CellDisplaySize * CELLS_PER_AXIS;

  ASSERT(GridSideDisplaySize < gGopInfo->HorizontalResolution
    && GridSideDisplaySize < gGopInfo->VerticalResolution);

  GridDrawStartAreaX = gMiddleScreenX - (GridSideDisplaySize / 2) - 1;
  GridDrawStartAreaY = gMiddleScreenY - (GridSideDisplaySize / 2) - 1;

  for (Index = 0; Index <= CELLS_PER_AXIS; Index++) {
    // X axis borders
    DrawRectangleToBackbuffer(
      60, 60, 60,
      GridSideDisplaySize, CELL_BORDER_SIZE,
      GridDrawStartAreaX,
      GridDrawStartAreaY + (Index * CellDisplaySize)
    );

    // Y axis borders
    DrawRectangleToBackbuffer(
      60, 60, 60,
      CELL_BORDER_SIZE, GridSideDisplaySize,
      GridDrawStartAreaX + (Index * CellDisplaySize),
      GridDrawStartAreaY
    );
  }

  for (Index = 0; Index < GRID_CELL_COUNT; Index++) {
    if (IS_SNAKE_CELL(mGrid[Index])) {
      DrawRectangleToBackbuffer(
        11, 125, 0,
        CellDisplaySize - CELL_BORDER_SIZE,
        CellDisplaySize - CELL_BORDER_SIZE,
        GridDrawStartAreaX + CELL_BORDER_SIZE + (CellDisplaySize * CELL_X(Index)),
        GridDrawStartAreaY + CELL_BORDER_SIZE + (CellDisplaySize * CELL_Y(Index))
      );
    }
    else if (IS_APPLE_CELL(mGrid[Index])) {
      DrawRectangleToBackbuffer(
        255, 0, 0,
        CellDisplaySize - CELL_BORDER_SIZE,
        CellDisplaySize - CELL_BORDER_SIZE,
        GridDrawStartAreaX + CELL_BORDER_SIZE + (CellDisplaySize * CELL_X(Index)),
        GridDrawStartAreaY + CELL_BORDER_SIZE + (CellDisplaySize * CELL_Y(Index))
      );
    }
  }

  // Highlight the snake's head
  DrawRectangleToBackbuffer(
    61, 175, 40,
    CellDisplaySize - CELL_BORDER_SIZE,
    CellDisplaySize - CELL_BORDER_SIZE,
    GridDrawStartAreaX + CELL_BORDER_SIZE + (CellDisplaySize * CELL_X(mSnake.OccupiedCells[0])),
    GridDrawStartAreaY + CELL_BORDER_SIZE + (CellDisplaySize * CELL_Y(mSnake.OccupiedCells[0]))
  );

  // And the snake's tail
  DrawRectangleToBackbuffer(
    108, 8, 0,
    CellDisplaySize - CELL_BORDER_SIZE,
    CellDisplaySize - CELL_BORDER_SIZE,
    GridDrawStartAreaX + CELL_BORDER_SIZE + (CellDisplaySize * CELL_X(mSnake.OccupiedCells[mSnake.Length - 1])),
    GridDrawStartAreaY + CELL_BORDER_SIZE + (CellDisplaySize * CELL_Y(mSnake.OccupiedCells[mSnake.Length - 1]))
  );
}

STATIC
VOID
DrawScene(
  VOID
)
{
  EFI_STATUS        Status;
  EFI_IMAGE_INPUT   Image;

  gST->ConOut->SetCursorPosition(
    gST->ConOut,
    0,
    0
  );

  Status = gHiiImage->GetImage(gHiiImage, gHiiHandle, IMAGE_TOKEN(IMG_LOGO), &Image);
  ASSERT_EFI_ERROR(Status);

  ClearBackbuffer();
  DrawImageToBackbuffer(
    &Image,
    gMiddleScreenX, gMiddleScreenY,
    TRUE
  );
  DrawGrid();
}

STATIC
DIRECTION_VECTOR
TranslateKeyToDirection(
  EFI_INPUT_KEY   *Key
)
{
  switch (Key->ScanCode) {
    case SCAN_LEFT:
      return RelativeLeft;
    case SCAN_RIGHT:
      return RelativeRight;
    case SCAN_UP:
      return RelativeUp;
    case SCAN_DOWN:
      return RelativeDown;
    default:
      return Undefined;
  }
}

STATIC
CONST EFI_STRING
TranslateDirectionToString(
  DIRECTION_VECTOR    Direction
)
{
  switch (Direction) {
    case RelativeLeft:
      return L"RelativeLeft";
    case RelativeRight:
      return L"RelativeRight";
    case RelativeUp:
      return L"RelativeUp";
    case RelativeDown:
      return L"RelativeDown";
    default:
      return L"Undefined";
  }
}

STATIC
VOID
InitSnake(
  VOID
)
{
  UINT16        SpawnCellCoord;
  UINTN         Index;

  SpawnCellCoord = CELL_INDEX(CELLS_PER_AXIS / 2, CELLS_PER_AXIS / 2);

  for (Index = 0; Index < SNAKE_INITIAL_LENGTH; Index++) {
    mGrid[SpawnCellCoord - Index] = 1;
    mSnake.OccupiedCells[Index] = SpawnCellCoord - Index;
  }

  mSnake.Length = SNAKE_INITIAL_LENGTH;
}

STATIC
BOOLEAN
UpdateSnake(
  DIRECTION_VECTOR Direction
)
{
  INT32     Delta;
  UINT16    NewHeadCell;
  UINT16    OldHeadCell;
  UINT16    OldTailCell;
  UINTN     Index;
  BOOLEAN   AteApple;

  switch (Direction) {
    case RelativeLeft:
      Delta = -1;
      break;
    case RelativeRight:
      Delta = +1;
      break;
    case RelativeUp:
      Delta = -CELLS_PER_AXIS;
      break;
    case RelativeDown:
      Delta = CELLS_PER_AXIS;
      break;
    default:
      // Would only happen if we pass Undefined, which should NEVER happen.
      ASSERT(Direction != Undefined);
      return FALSE;
  }

  OldHeadCell = mSnake.OccupiedCells[0];
  NewHeadCell = (OldHeadCell + Delta + GRID_CELL_COUNT) % GRID_CELL_COUNT;
  AteApple = IS_APPLE_CELL(mGrid[NewHeadCell]);

  if (!AteApple) {
    OldTailCell = mSnake.OccupiedCells[mSnake.Length - 1];
    mGrid[OldTailCell] = 0;
  }

  if (IS_SNAKE_CELL(mGrid[NewHeadCell])) {
    if (!AteApple) {
      mGrid[OldTailCell] = 1;
    }
    return FALSE;
  }

  // If we ate an apple, then grow the snake
  if (AteApple) {
    for (Index = mSnake.Length; Index > 0; Index--) {
      mSnake.OccupiedCells[Index] = mSnake.OccupiedCells[Index - 1];
    }

    mSnake.Length++;
    mScore += 100;

    SpawnApple();
  }
  else {
    for (Index = mSnake.Length - 1; Index > 0; Index--) {
      mSnake.OccupiedCells[Index] = mSnake.OccupiedCells[Index - 1];
    }
  }

  mSnake.OccupiedCells[0] = NewHeadCell;
  mGrid[NewHeadCell] = 1; // Mark as snake cell

  return TRUE;
}

VOID
RunGameLogic(
  VOID
)
{
  EFI_INPUT_KEY       Key;
  DIRECTION_VECTOR    Direction             = RelativeRight;
  DIRECTION_VECTOR    RequestedDirection;
  UINTN               DifficultyMultiplier                 = 0;

  InitSnake();
  SpawnApple();

  while (TRUE) {
    QueryLastKeystroke(&Key);
    if (Key.ScanCode == SCAN_ESC) {
      return;
    }

    RequestedDirection = TranslateKeyToDirection(&Key);
    if (
      RequestedDirection != Undefined &&
      (
        (Direction == RelativeLeft && RequestedDirection != RelativeRight) ||
        (Direction == RelativeUp && RequestedDirection != RelativeDown) ||
        (Direction == RelativeRight && RequestedDirection != RelativeLeft) ||
        (Direction == RelativeDown && RequestedDirection != RelativeUp)
      )
    ) {

      Direction = RequestedDirection;
    }

    if (!UpdateSnake(Direction)) {
      Print(L"\nGame Over! You died!\n");
      return;
    }

    DrawScene();
    PresentBackbuffer();

    Print(L"\n\nScore: %d\nDirection Vector: %s\n\n\nPress [ESC] to quit\n", mScore, TranslateDirectionToString(Direction));
    if (mSnake.Length == GRID_CELL_COUNT) {
      Print(L"Congratulations! You've won! Now you can flex on all your friends that you've beat Snake running under UEFI :P\n");
      return;
    }

    DifficultyMultiplier = (UINT32)(mScore / 100) * 2e4;
    DifficultyMultiplier = DifficultyMultiplier > 3e5 ? 4e5 : DifficultyMultiplier;

    gBS->Stall(5e5 - DifficultyMultiplier);
  }
}
