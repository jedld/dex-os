/***************************************************************************
 *            stdout.c
 *
 *  Sat Apr  7 07:24:30 2007
 *  Copyright  2007  Joseph Emmanuel Dayo
 *  joseph.dayo@gmail.com
 ****************************************************************************/
 
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
 #include "../../devmgr/dex32_devmgr.h"


int stdout_ready_put() {
	return 1;
}

int stdout_ready_get() {
	return 0;
}

int stdout_get_char() {
	return 0;
}

int stdout_put_char(int c) {
	printf("%c",(char)c);
}

int stdout_init() {
}


int register_stdout() {
	devmgr_char_desc stdout_desc;
	memset(&stdout_desc,0,sizeof(devmgr_char_desc));
		
	//fill up driver registration
	stdout_desc.hdr.size = sizeof(devmgr_char_desc);
	stdout_desc.hdr.type = DEVMGR_CHAR;
	strcpy(stdout_desc.hdr.name,"stdout");
	strcpy(stdout_desc.hdr.description,"standard output device");
	stdout_desc.put_char = stdout_put_char;
	stdout_desc.get_char = stdout_get_char;
	stdout_desc.ready_put = stdout_ready_put;
	stdout_desc.ready_get = stdout_ready_get;
	stdout_desc.init_device = stdout_init;
	
    int stdout_deviceid = devmgr_register((devmgr_generic*)&stdout_desc);
	return stdout_deviceid;
	}
