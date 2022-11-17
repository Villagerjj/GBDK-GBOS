#include <gb/gb.h>
#include <gb/drawing.h>
#include <stdint.h>
#include <stdio.h>
#include "include/Graphics.h"
#include "include/cursorgraphics.h"
#include "include/SystemProgramIDs.h"
#define SYSTEMVARS 13

extern uint_fast8_t DISKP0[8192];
extern uint8_t ChkDsk;
extern uint8_t iconCords[360];
extern uint8_t ProIDCords[360];
extern uint8_t RAMDISK[360];
//extern uint32_t DISKSPACE;
uint_fast8_t curx, cury;

void set_banked_data(uint8_t bank, uint16_t offset, uint8_t data)
{
  uint16_t index = (bank * (360 / 2)) + (offset / 2);
  SWITCH_RAM(index / 4096);
  index = ((index % 4096) * 2) + (offset % 2);
  DISKP0[index] = data;
}

uint8_t get_banked_data(uint8_t bank, uint16_t offset)
{
  uint16_t index = (bank * (360 / 2)) + (offset / 2);
  SWITCH_RAM(index / 4096);
  index = ((index % 4096) * 2) + (offset % 2);
  return DISKP0[index];
}

void CPRamDisk2Bank(uint8_t bank)
{
  for (uint16_t i = 0; i < 360; i++)
  {
    set_banked_data(bank, i, RAMDISK[i]);
  }
}

void firstBoot()
{
  for (uint8_t i = 0; i < 250; i++)
  {
    for (uint16_t d = 0; d < 360; d++)
    {
      RAMDISK[d] = 0;
      //DISKSPACE++;
    }
    CPRamDisk2Bank(i);
  }

  /*
  set_bkg_tile_xy(1, 1, 102); // trash can
  set_bkg_tile_xy(1, 3, 77);  // My gameboy
  set_bkg_tile_xy(1, 5, 75);  // paint
  set_bkg_tile_xy(1, 7, 76);  // notepad
  set_bkg_tile_xy(1, 9, 78);  // transfer*/

  iconCords[(1 * 20 + 1)] = 2;
  iconCords[(3 * 20 + 1)] = 1;
  iconCords[(5 * 20 + 1)] = 3;
  iconCords[(7 * 20 + 1)] = 4;
  iconCords[(9 * 20 + 1)] = 5;

  ProIDCords[(1 * 20 + 1)] = 2; // trash can
  ProIDCords[(3 * 20 + 1)] = 1; // My gameboy
  ProIDCords[(5 * 20 + 1)] = 3; // paint
  ProIDCords[(7 * 20 + 1)] = 4; // notepad
  ProIDCords[(9 * 20 + 1)] = 5; // transfer
  // set_bkg_tile_xy(5, 1, 102); // trash can
  // set_bkg_tile_xy(5, 1, 102); // trash can
}

void initDesktop()
{
  for (int i = 4; i < 17; i++)
  {
    set_bkg_tile_xy(i, 8, 0); // clearing nintendo logo
    set_bkg_tile_xy(i, 9, 0); // clearing nintendo logo
  }
  
  for (uint8_t iy=0; iy < 18; iy++)
  { 
    for(uint8_t ix=0; ix < 20; ix++)
    {
      set_bkg_tile_xy(ix, iy, iconCords[(iy * 20 + ix)]);
    }
  }
  
}

void main(void)
{
  SHOW_BKG;
  DISPLAY_ON;
  ENABLE_RAM_MBC5;
  SWITCH_RAM(SYSTEMVARS);
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
  SWITCH_RAM(SYSTEMVARS);
  if (ChkDsk != 69)
  {
    SWITCH_RAM(SYSTEMVARS);
    firstBoot();
    ChkDsk = 69;
  }

  SWITCH_RAM(SYSTEMVARS);

  initDesktop();
  set_sprite_data(0, 4, cursorgraphics);
  set_sprite_tile(0, 1);
  move_sprite(0, 88, 72);
  curx = 11;
  cury = 9;
  SHOW_SPRITES;
  while (1)
  {
    uint8_t cur = joypad();
    if (cur & J_A)
    {
      if (iconCords[(cury * 20 + curx)] != 0)
      {
        set_sprite_tile(0, 3);
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
    delay(100);
  }
}
