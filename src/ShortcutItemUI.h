#pragma once

#include "BlockType.h"

class ShortcutItemUI
{
public:
	ShortcutItemUI();

	int getCurrentItem() const { return currentItem; }
	void update();
	void render();

private:
	int currentItem;
};
