#pragma once

#include <functional>
#include <queue>
#include <memory>

#define QUASAR_ACTION_EQUALS_OVERRIDE(structname)\
	bool operator==(const structname&) const = default;\
	virtual bool equals(const ActionBase& other) const override { auto p = dynamic_cast<const structname*>(&other); return p && *this == *p; }

struct ActionBase
{
	size_t weight = sizeof(ActionBase);

	virtual ~ActionBase() = default;

	virtual void forward() = 0;
	virtual void backward() = 0;
	virtual bool equals(const ActionBase&) const { return false; }
	inline bool operator==(const ActionBase&) const = default;
};

class ActionHistory
{
	size_t tracking_size;
	size_t current_size = 0;
	std::deque<std::shared_ptr<ActionBase>> undo_deque;
	std::deque<std::shared_ptr<ActionBase>> redo_deque;

public:
	ActionHistory(size_t tracking_size = 4'000'000) : tracking_size(tracking_size) {}
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

private:
	void add_weight(size_t weight)
	{
		current_size += weight;
		while (current_size > tracking_size && !undo_deque.empty())
		{
			current_size -= undo_deque.front()->weight;
			undo_deque.pop_front();
		}
	}

public:
	void push(const std::shared_ptr<ActionBase>& action)
	{
		add_weight(action->weight);
		if (!redo_deque.empty())
		{
			if (redo_deque.back()->equals(*action))
				redo_deque.pop_back();
			else
				redo_deque.clear();
		}
		undo_deque.push_back(action);
		if (current_size > tracking_size)
			clear_history();
	}

	void push(std::shared_ptr<ActionBase>&& action)
	{
		add_weight(action->weight);
		if (!redo_deque.empty())
		{
			if (redo_deque.back()->equals(*action))
				redo_deque.pop_back();
			else
				redo_deque.clear();
		}
		undo_deque.push_back(std::move(action));
		if (current_size > tracking_size)
			clear_history();
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

	size_t get_current_memory_usage() const
	{
		return current_size;
	}

	float get_fraction_of_memory_usage() const
	{
		return float(current_size) / float(tracking_size);
	}

	size_t get_pool_size() const
	{
		return tracking_size;
	}

	void set_pool_size(size_t pool_size)
	{
		while (current_size > pool_size && !undo_deque.empty())
		{
			current_size -= undo_deque.front()->weight;
			undo_deque.pop_front();
		}
		tracking_size = pool_size;
	}
};
