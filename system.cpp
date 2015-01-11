/*
   This file is part of Retro Graphics Toolkit

   Retro Graphics Toolkit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or any later version.

   Retro Graphics Toolkit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Retro Graphics Toolkit.  If not, see <http://www.gnu.org/licenses/>.
   Copyright Sega16 (or whatever you wish to call me) (2012-2014)
*/
#include "project.h"
#include "global.h"
//System refers to game system and such or device for example sega genesis or NES
static void setbdmask(unsigned bd,unsigned mask){
	if(bd>mask)
		bd=mask;
	currentProject->subSystem&=~mask;
	currentProject->subSystem|=bd;
}
void setBitdepthcurSys(unsigned bd){
	--bd;
	switch(currentProject->gameSystem){
		case segaGenesis:
		case masterSystem:
		case gameGear:
		case TMS9918:
			setbdmask(bd,3);
		break;
		case NES:
			if(bd)
				currentProject->subSystem|=2;
			else
				currentProject->subSystem&=~2;
		break;
		case frameBufferPal:
			setbdmask(bd,7);
		break;
		default:
			show_default_error
	}
	if(currentProject->containsData(pjHaveTiles)){
		currentProject->tileC->tileSize=currentProject->tileC->sizew*currentProject->tileC->sizeh*getBitdepthcurSys()/8;
		currentProject->tileC->resizeAmt();
	}
}
int getBitdepthcurSysraw(void){
	switch(currentProject->gameSystem){
		case segaGenesis:
		case masterSystem:
		case gameGear:
		case TMS9918:
			return (currentProject->subSystem&3);
		break;
		case NES:
			return ((currentProject->subSystem>>1)&1);
		break;
		case frameBufferPal:
			return (currentProject->subSystem&7);
		break;
		default:
			show_default_error
	}
}
