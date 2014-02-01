class tileMap{
public:
	tileMap();
	~tileMap();
	uint8_t * tileMapDat;/*!< Holds tilemap data*/
	uint16_t mapSizeW,mapSizeH;
	bool get_hflip(uint16_t x,uint16_t y);
	bool get_vflip(uint16_t x,uint16_t y);
	bool get_prio(uint16_t x,uint16_t y);
	uint32_t get_tile(uint16_t x,uint16_t y);
	int32_t get_tileRow(uint16_t x,uint16_t y,uint8_t useRow);
	uint8_t get_palette_map(uint16_t x,uint16_t y);
	void set_pal_row(uint16_t x,uint16_t y,uint8_t row);
	bool saveToFile();
	bool loadFromFile();
	void sub_tile_map(uint32_t oldTile,uint32_t newTile,bool hflip,bool vflip);
	void pickRow(uint8_t amount);
	void pickRowDelta(bool showProgress=false,Fl_Progress *progress=0);
	void allRowZero(void);
	bool selection;
	uint16_t cur_x,cur_y;
};
