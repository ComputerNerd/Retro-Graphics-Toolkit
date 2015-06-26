/*
   This file is part of Retro Graphics Toolkit

   Retro Graphics Toolkit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or any later version.

   Retro Graphics Toolkit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Retro Graphics Toolkit. If not, see <http://www.gnu.org/licenses/>.
   Copyright Sega16 (or whatever you wish to call me) (2012-2015)
*/
#include "includes.h"
extern uint32_t currentChunk;
extern unsigned solidBits_G;
extern bool tileEditModeChunk_G;
extern unsigned ChunkOff[2];
extern unsigned scrollChunks_G[2];
extern uint_fast32_t editChunk_G[2];
extern uint32_t selBlock;
void insertChunkCB(Fl_Widget*,void*);
void delChunkAtCB(Fl_Widget*,void*);
void appendChunkCB(Fl_Widget*o,void*);
void saveChunkS1CB(Fl_Widget*o,void*);
void resizeChunkCB(Fl_Widget*o,void*);
void selBlockCB(Fl_Widget*,void*b);
void solidCB(Fl_Widget*,void*s);
void ImportS1CBChunks(Fl_Widget*,void*a);
void currentChunkCB(Fl_Widget*,void*);
void useBlocksCB(Fl_Widget*o,void*);
void scrollChunkCB(Fl_Widget*,void*);
void scrollChunkX(Fl_Widget*,void*);
void scrollChunkY(Fl_Widget*,void*);
