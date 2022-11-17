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
extern uint16_t IDCords[360];
extern uint8_t RAMDISK[360];

uint8_t IconCords[360];

//extern uint32_t DISKSPACE;
uint_fast8_t curx, cury;

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

void CPRamDisk2Bank(uint8_t bank)
{
  for (uint16_t i = 0; i < 360; i++)
  {
    SWITCH_RAM(SYSTEMVARS);
    set_banked_data(bank, i, RAMDISK[i]);
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
  IDCords[(1 * 20 + 1)] = 2;
  IDCords[(3 * 20 + 1)] = 1;
  IDCords[(5 * 20 + 1)] = 3;
  IDCords[(7 * 20 + 1)] = 4;
  IDCords[(9 * 20 + 1)] = 5;
  // set_bkg_tile_xy(5, 1, 102); // trash can
}

void initDesktop()
{
  SWITCH_RAM(SYSTEMVARS);
  
/*
  set_bkg_tile_xy(1, 1, 102); // trash can
  set_bkg_tile_xy(1, 3, 77);  // My gameboy
  set_bkg_tile_xy(1, 5, 75);  // paint
  set_bkg_tile_xy(1, 7, 76);  // notepad
  set_bkg_tile_xy(1, 9, 78);  // transfer */
for (uint8_t iy=0; iy < 18; iy++)
  { 
    for(uint8_t ix=0; ix < 20; ix++)
    {
      switch (IDCords[(iy * 20 + ix)])
      {

      case 0:
        set_bkg_tile_xy(ix, iy, 0);
        break;

      case 1:
        set_bkg_tile_xy(ix, iy, 77);
        break;

      case 2:
        set_bkg_tile_xy(ix, iy, 102);
        break;

      case 3:
        set_bkg_tile_xy(ix, iy, 75);
        break;

      case 4:
        set_bkg_tile_xy(ix, iy, 76);
        break;  

      case 5:
        set_bkg_tile_xy(ix, iy, 78);
        break;

      default:
      set_bkg_tile_xy(ix, iy, 38);
        break;
      }
      
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

POST:
  if (IDCords[1*20+1] != 2)
  {

    firstBoot();
    if (IDCords[1*20+1] != 2)
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
    uint8_t cur = joypad();
    if (cur & J_A)
    {
      SWITCH_RAM(SYSTEMVARS);
      if (IDCords[(cury * 20 + curx)] != 0)
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
