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
	currentItem = (currentItem + static_cast<u32>(device->getMouseWheel()) - 1) % 10 + 1;
}

void ShortcutItemUI::render()
{
	Vector2DI bbSize = video->getBackBufferSize();

	/* Draw frame */
	gui->fill2DRect({ bbSize.x / 2 - 209, bbSize.y - 140 }, { bbSize.x / 2 + 209, bbSize.y - 100 }, Color(0, 0, 0, 100));
	gui->fill2DRect({ bbSize.x / 2 - 211, bbSize.y - 142 }, { bbSize.x / 2 + 211, bbSize.y - 140 }, Color(200, 200, 200, 255));
	gui->fill2DRect({ bbSize.x / 2 - 211, bbSize.y - 100 }, { bbSize.x / 2 + 211, bbSize.y - 98 }, Color(200, 200, 200, 255));
	for (s32 i = 0; i < 11; i++)
		gui->fill2DRect({ bbSize.x / 2 - 211 + i * 42, bbSize.y - 140 },
			{ bbSize.x / 2 - 209 + i * 42, bbSize.y - 100 },
			Color(200, 200, 200, 255));

	/* Draw items */
	for (s32 i = 0; i < 10; i++)
	{
		PTexture texture = blockType->getTexture(i);
		if (texture)
			gui->draw2DTexture({ bbSize.x / 2 - 206 + i * 42 - 42, bbSize.y - 137 }, { 33, 33 }, texture);
	}

	/* Highlight curren item */
	s32 x = bbSize.x / 2 - 207 + currentItem * 42 - 42;
	s32 y = bbSize.y - 138;
	gui->draw2DRect({ x, y }, { x + 35, y + 35 }, Color(255, 255, 255, 255));
}
