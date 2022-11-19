#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/drawing.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "include/Graphics.h"
#include "include/cursorgraphics.h"
#include "include/SystemProgramIDs.h"
#define SYSTEMVARS 13

extern uint_fast8_t DISKP0[8192];
extern uint8_t ChkDsk;
extern uint16_t IDCords[360];
extern uint8_t RAMDISK[360];
extern BOOLEAN UsedClusters[250];

uint8_t IconCords[360];
uint8_t ProgMode;
// extern uint32_t DISKSPACE;
uint_fast8_t curx, cury;
uint8_t asciiNum;
BOOLEAN KEYBOARD;

void set_banked_data(uint8_t bank, uint16_t offset, uint8_t data)
{
  uint16_t index = (bank * (360 / 2)) + (offset / 2);
  SWITCH_RAM(index / 4096);
  index = ((index % 4096) * 2) + (offset % 2);
  DISKP0[index] = data;
}

void get_banked_data(uint8_t bank, uint16_t offset, uint8_t data)
{
  uint16_t index = (bank * (360 / 2)) + (offset / 2);
  SWITCH_RAM(index / 4096);
  index = ((index % 4096) * 2) + (offset % 2);
  DISKP0[index] = data;
}

void set_cluster_data(uint8_t bank, uint16_t offset, uint8_t data)
{

  SWITCH_RAM((bank * 360 + offset) / 16);
  DISKP0[(bank * 360 + offset)] = data;
}

uint8_t get_cluster_data(uint8_t bank, uint16_t offset)
{
  return DISKP0[(bank * 360 + offset)];
}

void CPRamDisk2BankOLD(uint8_t bank)
{
  for (uint16_t i = 0; i < 360; i++)
  {
    SWITCH_RAM(SYSTEMVARS);
    set_banked_data(bank, i, RAMDISK[i]);
  }
}

void CPRamDisk2Bank(uint8_t bank)
{
  uint16_t index = bank * (360 / 2);
  uint8_t sramBank = index / 4096;
  SWITCH_RAM(sramBank);
  index = (index % 4096) * 2;
  uint16_t remaining = 8192 - index;
  if (remaining >= 360)
  {
    memcpy(DISKP0 + index, RAMDISK, 360);
  }
  else
  {
    memcpy(DISKP0 + index, RAMDISK, remaining);
    SWITCH_RAM(sramBank + 1);
    memcpy(DISKP0, RAMDISK + remaining, 360 - remaining);
  }
}

void genIconsfromIDs()
{
  for (uint16_t i = 0; i < 360; i++)
  {
    switch (IDCords[i])
    {

    case 0:
      IconCords[i] = 0;
      break;

    case TRASH:
      IconCords[i] = 102;
      break;

    case MYGB:
      IconCords[i] = 77;
      break;

    case PAINT:
      IconCords[i] = 75;
      break;

    case NOTES:
      IconCords[i] = 76;
      break;

    case TRANSFER:
      IconCords[i] = 78;
      break;

    default:
      IconCords[i] = 38;
      break;
    }
  }
}
uint8_t findFreeCluster()
{
  for (uint8_t i = 0; i < 250; i++)
  {
    if (UsedClusters[i] == 0)
    {
      return i;
    }
  }
  return 255;
}

uint16_t findFreeSlotDesktop()
{

  for (uint16_t i = 0; i < 360; i++)
  {
    if (IDCords[i] == 0)
    {
      return i;
    }
  }
  return 255;
}

void initNotepad()
{
  set_sprite_tile(0, 2);
  move_sprite(0, 8, 16);
  curx = 0;
  cury = 0;
  ProgMode = 1;
  for (uint8_t iy = 0; iy < 19; iy++)
  {
    for (uint8_t ix = 0; ix < 18; ix++)
    {

      set_bkg_tile_xy(ix, iy, RAMDISK[(iy * 20 + ix)]);
    }
  }

  set_bkg_tile_xy(1, 17, 98);
  set_bkg_tile_xy(2, 17, 99);
}
void initDesktop()
{
  set_sprite_tile(0, 1);
  move_sprite(0, 88, 72);
  curx = 10;
  cury = 9;
  ProgMode = 0;
  SWITCH_RAM(SYSTEMVARS);

  /*
    set_bkg_tile_xy(1, 1, 102); // trash can
    set_bkg_tile_xy(1, 3, 77);  // My gameboy
    set_bkg_tile_xy(1, 5, 75);  // paint
    set_bkg_tile_xy(1, 7, 76);  // notepad
    set_bkg_tile_xy(1, 9, 78);  // transfer */
  genIconsfromIDs();
  for (uint8_t iy = 0; iy < 20; iy++)
  {
    for (uint8_t ix = 0; ix < 18; ix++)
    {

      set_bkg_tile_xy(ix, iy, IconCords[(iy * 20 + ix)]);
    }
  }
}

void firstBoot()
{
  SWITCH_RAM(SYSTEMVARS);
  for (uint16_t d = 0; d < 360; d++)
  {

    RAMDISK[d] = 0;
    // DISKSPACE++;
  }
  for (uint8_t i = 0; i < 250; i++)
  {

    CPRamDisk2Bank(i);
  }
  SWITCH_RAM(SYSTEMVARS);
  for (uint16_t d = 0; d < 360; d++)
  {
    IDCords[d] = 0;
    // DISKSPACE++;
  }
  IDCords[(1 * 20 + 1)] = 103;
  IDCords[(3 * 20 + 1)] = 104;
  IDCords[(5 * 20 + 1)] = 105;
  IDCords[(7 * 20 + 1)] = 106;
  IDCords[(9 * 20 + 1)] = 107;
  // set_bkg_tile_xy(5, 1, 102); // trash can
}

void controls(uint8_t cur)
{
  switch (ProgMode)
  {
  case 0:
    if (cur & J_A)
    {
      SWITCH_RAM(SYSTEMVARS);
      uint8_t temp = IDCords[(cury * 20 + curx)];
      if (temp != 0)
      {
        if (temp == NOTES)
        {
          initNotepad();
        }
        else
        {
          set_sprite_tile(0, 3);
        }
      }
      else
      {
        set_sprite_tile(0, 1);
      }
    }

    if (cur & J_B)
    {
    }

    if (cur & J_SELECT)
    {
      if(KEYBOARD == 1)
      {
        KEYBOARD = 0; 
      }
      else
      {
        KEYBOARD = 1; 
      }
      
    }

    if (cur & J_START)
    {
    }

    if (cur & J_UP)
    {
      cury -= 1;
      scroll_sprite(0, 0, -8);
    }
    else if (cur & J_DOWN)
    {
      cury += 1;
      scroll_sprite(0, 0, 8);
    }

    if (cur & J_LEFT)
    {
      curx -= 1;
      scroll_sprite(0, -8, 0);
    }
    else if (cur & J_RIGHT)
    {
      curx += 1;
      scroll_sprite(0, 8, 0);
    }
    break;

  case 1: // notepad
    if (cur & J_A)
    {

      RAMDISK[cury * 20 + curx] = asciiNum;
    }

    if (cur & J_B)
    {
      RAMDISK[cury * 20 + curx] = 0;
    }

    if (cur & J_SELECT)
    {
      initDesktop();
    }

    if (cur & J_START)
    {
      uint8_t temp = findFreeCluster();
      IconCords[findFreeSlotDesktop()] = temp;
      CPRamDisk2Bank(temp);
      initDesktop();
    }

    if (cur & J_UP)
    {
      if (asciiNum < 37)
      {
        asciiNum++;
      }
      else
      {
        asciiNum = 1;
      }
      // cury -= 1;
      // scroll_sprite(0, 0, -8);
    }
    else if (cur & J_DOWN)
    {
      if (asciiNum > 0)
      {
        asciiNum--;
      }
      else
      {
        asciiNum = 36;
      }

      // cury += 1;
      // scroll_sprite(0, 0, 8);
    }

    if (cur & J_LEFT)
    {
      if (curx > 0)
      {
        curx -= 1;
        scroll_sprite(0, -8, 0);
      }
      else
      {
        curx += 19;
        scroll_sprite(0, 152, 0);
        cury -= 1;
        scroll_sprite(0, 0, -8);
      }
      set_bkg_tile_xy(curx + 1, cury, RAMDISK[cury * 20 + (curx + 1)]);
    }
    else if (cur & J_RIGHT)
    {
      if (curx < 20)
      {
        curx += 1;
        scroll_sprite(0, 8, 0);
      }
      else
      {
        curx -= 19;
        scroll_sprite(0, -152, 0);
        cury += 1;
        scroll_sprite(0, 0, 8);
      }
      set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
    }
    SWITCH_RAM(SYSTEMVARS);
    if (KEYBOARD == 1)
    {
      receive_byte();
      /* Wait for IO completion... */
      while ((_io_status == IO_RECEIVING) && (cur == 0))
       ;
      if (_io_status == IO_IDLE)
      {
        switch (_io_in)
        {
          case ENTER:
          asciiNum = 0;
          uint8_t temp = findFreeCluster();
          IconCords[findFreeSlotDesktop()] = temp;
          CPRamDisk2Bank(temp);
          initDesktop();
          break;

        case START:
          asciiNum = 0;
          initDesktop();
          break;

        case SPACE:
          asciiNum = 0;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case BKSP:
          asciiNum = 0;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx > 0)
          {
            curx -= 1;
            scroll_sprite(0, -8, 0);
          }
          else
          {
            curx += 19;
            scroll_sprite(0, 152, 0);
            cury -= 1;
            scroll_sprite(0, 0, -8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case ZERO:
          asciiNum = 1;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case ONE:
          asciiNum = 2;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case TWO:
          asciiNum = 3;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case THREE:
          asciiNum = 4;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case FOUR:
          asciiNum = 5;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case FIVE:
          asciiNum = 6;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case SIX:
          asciiNum = 7;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case SEVEN:
          asciiNum = 8;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case EIGHT:
          asciiNum = 9;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case NINE:
          asciiNum = 10;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case A:
          asciiNum = 11;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case B:
          asciiNum = 12;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case C:
          asciiNum = 13;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case D:
          asciiNum = 14;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case E:
          asciiNum = 15;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case F:
          asciiNum = 16;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case G:
          asciiNum = 17;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case H:
          asciiNum = 18;
          RAMDISK[cury * 20 + curx] = asciiNum;
          
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            //set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case I:
          asciiNum = 19;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case J:
          asciiNum = 20;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case K:
          asciiNum = 21;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case L:
          asciiNum = 22;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case M:
          asciiNum = 23;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case N:
          asciiNum = 24;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case O:
          asciiNum = 25;
          RAMDISK[cury * 20 + curx] = asciiNum;
          
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case P:
          asciiNum = 26;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case Q:
          asciiNum = 27;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case R:
          asciiNum = 28;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case S:
          asciiNum = 29;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case T:
          asciiNum = 30;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case U:
          asciiNum = 31;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case V:
          asciiNum = 32;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case W:
          asciiNum = 33;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case X:
          asciiNum = 34;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case Y:
          asciiNum = 35;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        case Z:
          asciiNum = 36;
          RAMDISK[cury * 20 + curx] = asciiNum;
          if (curx < 20)
          {
            curx += 1;
            scroll_sprite(0, 8, 0);
          }
          else
          {
            curx -= 19;
            scroll_sprite(0, -152, 0);
            cury += 1;
            scroll_sprite(0, 0, 8);
          }
          set_bkg_tile_xy(curx - 1, cury, RAMDISK[cury * 20 + (curx - 1)]);
          break;

        default:
          break;
        }
      }
    }
    else
    {
      set_bkg_tile_xy(curx, cury, asciiNum);
    }

    break;

  default:
    break;
  }
}

void main(void)
{
  SHOW_BKG;
  DISPLAY_ON;
  ENABLE_RAM_MBC5;
  cpu_fast();
  SWITCH_RAM(SYSTEMVARS);
  ProgMode = 0;
  
  set_bkg_data(0, 103, graphics);

  set_bkg_tile_xy(4, 8, 13);
  set_bkg_tile_xy(5, 8, 18);
  set_bkg_tile_xy(6, 8, 15);
  set_bkg_tile_xy(7, 8, 13);
  set_bkg_tile_xy(8, 8, 21);
  set_bkg_tile_xy(9, 8, 19);
  set_bkg_tile_xy(10, 8, 24);
  set_bkg_tile_xy(11, 8, 17);
  set_bkg_tile_xy(12, 8, 0);
  set_bkg_tile_xy(13, 8, 14);
  set_bkg_tile_xy(14, 8, 19);
  set_bkg_tile_xy(15, 8, 29);
  set_bkg_tile_xy(16, 8, 21);
  set_bkg_tile_xy(17, 8, 0);
  for (uint8_t i = 4; i < 17; i++)
  {

    set_bkg_tile_xy(i, 9, 0); // clearing nintendo logo
  }

POST:
  SWITCH_RAM(SYSTEMVARS);
  if (IDCords[1 * 20 + 1] != 103)
  {

    firstBoot();
    if (IDCords[1 * 20 + 1] != 103)
    {
      set_bkg_tile_xy(4, 9, 28);
      set_bkg_tile_xy(5, 9, 15);
      set_bkg_tile_xy(6, 9, 30);
      set_bkg_tile_xy(7, 9, 28);
      set_bkg_tile_xy(8, 9, 35);
      set_bkg_tile_xy(9, 9, 19);
      set_bkg_tile_xy(10, 9, 24);
      set_bkg_tile_xy(11, 9, 17);

      goto POST;
    }
  }

  for (uint8_t i = 4; i < 17; i++)
  {
    set_bkg_tile_xy(i, 8, 0); // clearing nintendo logo
    set_bkg_tile_xy(i, 9, 0); // clearing nintendo logo
  }

  SWITCH_RAM(SYSTEMVARS);

  initDesktop();
  set_sprite_data(0, 4, cursorgraphics);
  set_sprite_tile(0, 1);
  move_sprite(0, 88, 72);
  curx = 10;
  cury = 9;
  SHOW_SPRITES;
  while (1)
  {
    controls(joypad());
    
    delay(100);
  }
}
