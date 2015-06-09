/*
  Name: DEX Operating system VGA subsystem
  Copyright: 
  Author: Original Code by Chris Giese, modified by Joseph Emmanuel DL Dayo
  Date: 26/01/04 18:47
  Description: handles VGA functions without using the BIOS and only through
               the VGA ports.
*/

static void set_plane(unsigned p);
