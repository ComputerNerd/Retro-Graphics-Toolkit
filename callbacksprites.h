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
extern uint32_t curSprite;
extern uint32_t curSpritegroup;
extern int32_t spriteEndDraw[2];
extern bool centerSpriteDraw_G;
void palRowstCB(Fl_Widget*,void*);
void optimizeSpritesCB(Fl_Widget*,void*);
void ditherSpriteAsImage(unsigned which);
void ditherSpriteAsImageAllCB(Fl_Widget*,void*);
void ditherSpriteAsImageCB(Fl_Widget*,void*);
void setDrawSpriteCB(Fl_Widget*,void*m);
void SpriteSheetimportCB(Fl_Widget*o,void*);
void assignSpriteglobalnameCB(Fl_Widget*o,void*);
void exportSonicDPLCCB(Fl_Widget*o,void*t);
void alignSpriteCB(Fl_Widget*,void*t);
void importSonicDPLCCB(Fl_Widget*o,void*t);
void spritePrioCB(Fl_Widget*,void*);
void setoffspriteCB(Fl_Widget*o,void*y);
void exportSonicMappingCB(Fl_Widget*o,void*);
void importSonicMappingCB(Fl_Widget*o,void*);
void assignSpritegroupnameCB(Fl_Widget*o,void*);
void spriteHflipCB(Fl_Widget*,void*);
void spriteVflipCB(Fl_Widget*,void*);
void SpriteimportCB(Fl_Widget*,void*);
void selSpriteCB(Fl_Widget*w,void*);
void selspriteGroup(Fl_Widget*o,void*);
void appendSpriteCB(Fl_Widget*,void*);
void delSpriteCB(Fl_Widget*,void*);
void setvalueSpriteCB(Fl_Widget*o,void*which);
