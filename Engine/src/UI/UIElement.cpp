#include <Core.h>

#include "UIEvents.h"
#include "UIEventSource.h"
#include "UIElement.h"

UIElement::UIElement(UIElement *_parent):
	parent(_parent)
{
	if (parent)
		parent->children.push_back(std::unique_ptr<UIElement>(this));
}

void UIElement::mouseDownEvent(const UIMouseEvent &event)
{
	onMouseDown.raise();
}

void UIElement::mouseUpEvent(const UIMouseEvent &event)
{
	onMouseUp.raise();
}

void UIElement::clickEvent(const UIMouseEvent &event)
{
	onClick.raise();
}

bool UIElement::eventFilter(const UIEvent &event)
{
	for (const std::unique_ptr<UIElement> &child: children)
	{
	}
	return false;
}
