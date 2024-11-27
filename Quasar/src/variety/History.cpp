#include "History.h"

void ActionHistory::execute(std::shared_ptr<ActionBase>&& action)
{
	action->forward();
	push(std::move(action));
}

void ActionHistory::execute(const std::shared_ptr<ActionBase>& action)
{
	action->forward();
	push(action);
}

void ActionHistory::add_weight(size_t weight)
{
	current_size += weight;
	while (current_size > tracking_size && !undo_deque.empty())
	{
		current_size -= undo_deque.front()->weight;
		undo_deque.pop_front();
	}
}

void ActionHistory::push(const std::shared_ptr<ActionBase>& action)
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

void ActionHistory::push(std::shared_ptr<ActionBase>&& action)
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

void ActionHistory::execute_no_undo(const std::shared_ptr<ActionBase>& action)
{
	action->forward();
	clear_history();
}

void ActionHistory::clear_history()
{
	undo_deque.clear();
	redo_deque.clear();
	current_size = 0;
}

void ActionHistory::undo()
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

size_t ActionHistory::undo_size() const
{
	return undo_deque.size();
}

void ActionHistory::redo()
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

size_t ActionHistory::redo_size() const
{
	return redo_deque.size();
}

size_t ActionHistory::get_current_memory_usage() const
{
	return current_size;
}

float ActionHistory::get_fraction_of_memory_usage() const
{
	return float(current_size) / float(tracking_size);
}

size_t ActionHistory::get_pool_size() const
{
	return tracking_size;
}

void ActionHistory::set_pool_size(size_t pool_size)
{
	while (current_size > pool_size && !undo_deque.empty())
	{
		current_size -= undo_deque.front()->weight;
		undo_deque.pop_front();
	}
	tracking_size = pool_size;
}
