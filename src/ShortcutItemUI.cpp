#include "stdafx.h"
#include "ShortcutItemUI.h"

ShortcutItemUI::ShortcutItemUI() {
	currentItem = 1;
}

void ShortcutItemUI::update()
{
	if (device->isKeyPressed(KEY_0))
		currentItem = 10;
	else if (device->isKeyPressed(KEY_1))
		currentItem = 1;
	else if (device->isKeyPressed(KEY_2))
		currentItem = 2;
	else if (device->isKeyPressed(KEY_3))
		currentItem = 3;
	else if (device->isKeyPressed(KEY_4))
		currentItem = 4;
	else if (device->isKeyPressed(KEY_5))
		currentItem = 5;
	else if (device->isKeyPressed(KEY_6))
		currentItem = 6;
	else if (device->isKeyPressed(KEY_7))
		currentItem = 7;
	else if (device->isKeyPressed(KEY_8))
		currentItem = 8;
	else if (device->isKeyPressed(KEY_9))
		currentItem = 9;
	currentItem = (currentItem - int(device->getMouseWheel()) + 9) % 10 + 1;
}

void ShortcutItemUI::render()
{
	int2 bbSize = video->getBackBufferSize();

	/* Draw frame */
	painter->fillRect(bbSize.x / 2 - 209, bbSize.y - 140, 418, 40, Color(0, 0, 0, 100));
	painter->fillRect(bbSize.x / 2 - 211, bbSize.y - 142, 422, 2, Color(200, 200, 200, 255));
	painter->fillRect(bbSize.x / 2 - 211, bbSize.y - 100, 422, 2, Color(200, 200, 200, 255));
	for (int i = 0; i < 11; i++)
		painter->fillRect(bbSize.x / 2 - 211 + i * 42, bbSize.y - 140, 2, 40,
			Color(200, 200, 200, 255));

	/* Draw items */
	for (int i = 0; i < 10; i++)
	{
		PTexture texture = blockType->getTexture(i);
		if (texture)
			painter->drawTexture(bbSize.x / 2 - 206 + i * 42 - 42, bbSize.y - 137, 33, 33, texture, Color(255, 255, 255, 255));
	}

	/* Highlight curren item */
	int x = bbSize.x / 2 - 207 + currentItem * 42 - 42;
	int y = bbSize.y - 138;
	//painter->drawRect(x, y, x + 35, y + 35, rgba(255, 255, 255, 255));
}
