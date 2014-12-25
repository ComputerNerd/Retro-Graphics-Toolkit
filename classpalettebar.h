#define tabsWithPalette 4
class paletteBar{
private:
	unsigned ox[tabsWithPalette],oy[tabsWithPalette];
	unsigned baseOffx[tabsWithPalette],baseOffy[tabsWithPalette];
	bool tiny[tabsWithPalette];
	bool all[tabsWithPalette];
	bool alt[tabsWithPalette];
	uint32_t sysCache;
public:
	Fl_Slider*slide[tabsWithPalette][3];
	unsigned selRow[tabsWithPalette];
	unsigned selBox[tabsWithPalette];
	inline unsigned getEntry(unsigned tab) const{
		return currentProject->pal->perRow*selRow[tab]+selBox[tab];
	}
	void addTab(unsigned tab,bool all=false,bool tiny=false,bool alt=false);
	void setSys(void);
	void updateSize(unsigned tab);
	void updateSlider(unsigned tab);
	void updateSliders(void){
		for(unsigned i=0;i<tabsWithPalette;++i)
			updateSlider(i);
	}
	unsigned toTab(unsigned realtab);
	void changeRow(unsigned row,unsigned tab){
		selRow[tab]=row;
		updateSlider(tab);
	}
	void checkBox(int x,int y,unsigned tab);
	void drawBoxes(unsigned tab);
};
extern paletteBar palBar;
