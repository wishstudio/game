#pragma once

#include "BlockType.h"

class ShortcutItemUI
{
public:
	ShortcutItemUI();

	u16 getCurrentItem() const { return currentItem; }
	void update();
	void render();

private:
	u16 currentItem;
};
