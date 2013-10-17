#pragma once

#include "EventReceiver.h"
#include "BlockType.h"

class ShortcutItemUI {
public:
	ShortcutItemUI();
	void show();
	u16 getCurrentItem() const {return handItemId;}
	void getItemSelected();
	void drawFrame();
	void drawItem(u16 id);
	void highLight();

private:
	u16 handItemId;
	u32 w;
	u32 h;
	int wheel;
};
