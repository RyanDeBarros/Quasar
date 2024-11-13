#pragma once

#include <functional>
#include <queue>
#include <memory>

struct ActionBase
{
	float weight = 1;

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
	const float tracking_size;
	float current_size = 0;
	std::deque<std::shared_ptr<ActionBase>> undo_deque;
	std::deque<std::shared_ptr<ActionBase>> redo_deque;

public:
	ActionHistory(float tracking_size = 100) : tracking_size(tracking_size) {}
	ActionHistory(const ActionHistory&) = delete;
	ActionHistory(ActionHistory&&) noexcept = delete;

	void execute(std::shared_ptr<ActionBase>&& action)
	{
		action->forward();
		push(std::move(action));
	}

	void execute(const std::shared_ptr<ActionBase>& action)
	{
		action->forward();
		push(action);
	}

	void push(std::shared_ptr<ActionBase>&& action)
	{
		current_size += action->weight;
		while (current_size > tracking_size)
		{
			current_size -= undo_deque.front()->weight;
			undo_deque.pop_front();
		}
		undo_deque.push_back(std::move(action));
		redo_deque.clear();
	}

	void push(const std::shared_ptr<ActionBase>& action)
	{
		current_size += action->weight;
		while (current_size > tracking_size)
		{
			current_size -= undo_deque.front()->weight;
			undo_deque.pop_front();
		}
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
		current_size = 0;
	}

	void undo()
	{
		if (!undo_deque.empty())
		{
			std::shared_ptr<ActionBase> action = std::move(undo_deque.back());
			current_size -= action->weight;
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
			current_size += action->weight;
			undo_deque.push_back(std::move(action));
		}
	}

	size_t redo_size() const
	{
		return redo_deque.size();
	}

	float tracking_weight() const
	{
		return current_size;
	}
};
