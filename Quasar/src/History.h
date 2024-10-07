#pragma once

#include <functional>
#include <queue>

struct Action
{
	std::function<void(void)> forward;
	std::function<void(void)> backward;

	Action(std::function<void(void)>&& forward, std::function<void(void)>&& backward)
		: forward(forward), backward(backward) {}
};

class ActionHistory
{
	const size_t tracking_length;
	std::deque<Action> undo_deque;
	std::deque<Action> redo_deque;

public:
	ActionHistory(size_t tracking_length = 100) : tracking_length(tracking_length) {}

	void execute(Action&& action)
	{
		action.forward();
		if (undo_deque.size() == tracking_length)
			undo_deque.pop_front();
		undo_deque.push_back(std::move(action));
		redo_deque.clear();
	}

	void execute_no_undo(Action&& action)
	{
		action.forward();
		clear_history();
	}

	void clear_history()
	{
		undo_deque.clear();
		redo_deque.clear();
	}

	void undo()
	{
		if (!undo_deque.empty())
		{
			Action action = std::move(undo_deque.back());
			undo_deque.pop_back();
			action.backward();
			redo_deque.push_back(std::move(action));
		}
	}

	void redo()
	{
		if (!redo_deque.empty())
		{
			Action action = std::move(redo_deque.back());
			redo_deque.pop_back();
			action.forward();
			undo_deque.push_back(std::move(action));
		}
	}
};

inline ActionHistory GlobalActionHistory;
