#pragma once

#include <functional>
#include <queue>
#include <memory>

struct ActionBase
{
	virtual ~ActionBase() = default;

	virtual void forward() = 0;
	virtual void backward() = 0;
};

struct StandardAction : public ActionBase
{
	std::function<void(void)> _forward;
	std::function<void(void)> _backward;

	StandardAction(const std::function<void(void)>& _forward, const std::function<void(void)>& _backward)
		: _forward(_forward), _backward(_backward) {}
	StandardAction(const std::function<void(void)>& _forward, std::function<void(void)>&& _backward)
		: _forward(_forward), _backward(std::move(_backward)) {}
	StandardAction(std::function<void(void)>&& _forward, const std::function<void(void)>& _backward)
		: _forward(std::move(_forward)), _backward(_backward) {}
	StandardAction(std::function<void(void)>&& _forward, std::function<void(void)>&& _backward)
		: _forward(std::move(_forward)), _backward(std::move(_backward)) {}

	void forward() override { _forward(); }
	void backward() override { _backward(); }
};

class ActionHistory
{
	const size_t tracking_length;
	std::deque<std::shared_ptr<ActionBase>> undo_deque;
	std::deque<std::shared_ptr<ActionBase>> redo_deque;

public:
	ActionHistory(size_t tracking_length = 100) : tracking_length(tracking_length) {}

	void execute(std::shared_ptr<ActionBase>&& action)
	{
		action->forward();
		if (undo_deque.size() == tracking_length)
			undo_deque.pop_front();
		undo_deque.push_back(std::move(action));
		redo_deque.clear();
	}

	void execute(const std::shared_ptr<ActionBase>& action)
	{
		action->forward();
		if (undo_deque.size() == tracking_length)
			undo_deque.pop_front();
		undo_deque.push_back(action);
		redo_deque.clear();
	}

	void execute_no_undo(const std::shared_ptr<ActionBase>& action)
	{
		action->forward();
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
			std::shared_ptr<ActionBase> action = std::move(undo_deque.back());
			undo_deque.pop_back();
			action->backward();
			redo_deque.push_back(std::move(action));
		}
	}

	size_t undo_size() const
	{
		return undo_deque.size();
	}

	void redo()
	{
		if (!redo_deque.empty())
		{
			std::shared_ptr<ActionBase> action = std::move(redo_deque.back());
			redo_deque.pop_back();
			action->forward();
			undo_deque.push_back(std::move(action));
		}
	}

	size_t redo_size() const
	{
		return redo_deque.size();
	}
};
