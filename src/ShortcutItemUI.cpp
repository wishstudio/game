#include "stdafx.h"
#include "ShortcutItemUI.h"

struct BlockTypeSpec;


ShortcutItemUI::ShortcutItemUI() {
	handItemId = 1;
	wheel = 0;
	w = 0;
	h = 0;
}

void ShortcutItemUI::drawFrame() {
	driver->draw2DRectangle(video::SColor(100, 0, 0, 0),
								rect<s32>(w / 2 - 209, h - 140, w / 2 + 209, h - 100));
	driver->draw2DRectangle(video::SColor(255, 200, 200, 200), 
								rect<s32>(w / 2 - 211, h - 142, w / 2 + 211, h - 140));
	driver->draw2DRectangle(video::SColor(255, 200, 200, 200), 
								rect<s32>(w / 2 - 211, h - 100, w / 2 + 211, h - 98));
	for (int i = 0; i < 11; ++i) {
		driver->draw2DRectangle(video::SColor(255, 200, 200, 200), 
								rect<s32>(w / 2 - 211 + i * 42, h - 140, w / 2 - 209 + i * 42, h - 100));
	}
}

void ShortcutItemUI::drawItem(u16 id) {
	s32 ltx = w / 2 - 206 + id * 42 - 42;
	s32 lth = h - 137;
	ITexture *texture = blockType->getTexture(id);
	if (texture != nullptr) {
		driver->draw2DImage(blockType->getTexture(id), 
			rect<s32>(ltx, lth, ltx + 33, lth + 33), 
			rect<s32>(0, 0, 15, 15));
	}
}

void ShortcutItemUI::highLight() {
	s32 ltx = w / 2 - 207 + handItemId * 42 - 42;
	s32 lth = h - 138;
	driver->draw2DRectangleOutline(recti(ltx, lth, ltx + 35, lth + 35), video::SColor(255, 255, 255, 255));
}

void ShortcutItemUI::getItemSelected() {
	if (eventReceiver->isKeyDown(KEY_KEY_0))
		handItemId = 10;
	else if (eventReceiver->isKeyDown(KEY_KEY_1))
		handItemId = 1;
	else if (eventReceiver->isKeyDown(KEY_KEY_2))
		handItemId = 2;
	else if (eventReceiver->isKeyDown(KEY_KEY_3))
		handItemId = 3;
	else if (eventReceiver->isKeyDown(KEY_KEY_4))
		handItemId = 4;
	else if (eventReceiver->isKeyDown(KEY_KEY_5))
		handItemId = 5;
	else if (eventReceiver->isKeyDown(KEY_KEY_6))
		handItemId = 6;
	else if (eventReceiver->isKeyDown(KEY_KEY_7))
		handItemId = 7;
	else if (eventReceiver->isKeyDown(KEY_KEY_8))
		handItemId = 8;
	else if (eventReceiver->isKeyDown(KEY_KEY_9))
		handItemId = 9;
	wheel = eventReceiver->getMouseWheel();
	handItemId = (handItemId + 9 + wheel) % 10 + 1;

}

void ShortcutItemUI::show() {
	w = driver->getScreenSize().Width;
	h = driver->getScreenSize().Height;
	drawFrame();
	getItemSelected();
	highLight();
	for (u16 i = 0; i < 10; ++i) {
		drawItem(i);
	}
}
