#pragma once

#include <functional>
#include <vector>

template <typename... TArg>
class UIEventSource final
{
public:
	typedef std::function<void(TArg...)> handler_type;

	UIEventSource() = default;
	UIEventSource(const UIEventSource &) = delete;

	UIEventSource &operator= (const UIEventSource &) = delete;

	void raise(TArg&&... args)
	{
		for (const handler_type &handler: handlers)
			handler(std::forward(args)...);
	}
	
	void addHandler(const handler_type &handler)
	{
		handlers.insert(handler);
	}

	void removeHandler(const handler_type &handler)
	{
		handlers.erase(handler);
	}

	UIEventSource &operator+= (const handler_type &handler)
	{
		addHandler(handler);
		return *this;
	}

	UIEventSource &operator-= (const handler_type &handler)
	{
		removeHandler(handler);
		return *this;
	}

private:
	std::vector<handler_type> handlers;
};
