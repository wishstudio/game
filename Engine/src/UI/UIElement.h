#pragma once

#include <vector>

class UIElement
{
public:
	UIEventSource<> onMouseDown, onMouseUp, onClick;

	UIElement(UIElement *parent = nullptr);
	virtual ~UIElement() {}

	virtual void mouseDownEvent(const UIMouseEvent &event);
	virtual void mouseUpEvent(const UIMouseEvent &event);
	virtual void clickEvent(const UIMouseEvent &event);

	virtual bool eventFilter(const UIEvent &event);

private:
	UIElement *parent;
	std::vector<std::unique_ptr<UIElement>> children;
};
