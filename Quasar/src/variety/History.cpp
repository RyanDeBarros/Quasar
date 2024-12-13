#include "History.h"

InverseAction::InverseAction(const std::shared_ptr<ActionBase>& normal)
	: normal(normal)
{
	weight = sizeof(InverseAction) + normal->weight;
}

InverseAction::InverseAction(std::shared_ptr<ActionBase>&& normal)
	: normal(std::move(normal))
{
	weight = sizeof(InverseAction) + this->normal->weight;
}

void InverseAction::forward()
{
	normal->backward();
}

void InverseAction::backward()
{
	normal->forward();
}

bool InverseAction::equals(const ActionBase& other) const
{
	if (auto p = dynamic_cast<const InverseAction*>(&other))
		return normal->equals(*p->normal);
	else
		return false;
}

DualAction::DualAction(const std::shared_ptr<ActionBase>& first, const std::shared_ptr<ActionBase>& second)
	: first(first), second(second)
{
	weight = sizeof(DualAction) + first->weight + second->weight;
}

DualAction::DualAction(std::shared_ptr<ActionBase>&& first, std::shared_ptr<ActionBase>&& second)
	: first(std::move(first)), second(std::move(second))
{
	weight = sizeof(DualAction) + this->first->weight + this->second->weight;
}

void DualAction::forward()
{
	first->forward();
	second->forward();
}

void DualAction::backward()
{
	second->backward();
	first->backward();
}

bool DualAction::equals(const ActionBase& other) const
{
	if (auto p = dynamic_cast<const DualAction*>(&other))
		return first->equals(*p->first) && second->equals(*p->second);
	else
		return false;
}

CompositeAction::CompositeAction(const std::vector<std::shared_ptr<ActionBase>>& actions)
	: actions(actions)
{
	weight = sizeof(CompositeAction);
	for (const auto& action : actions)
		weight += action->weight;
}

CompositeAction::CompositeAction(std::vector<std::shared_ptr<ActionBase>>&& actions)
	: actions(std::move(actions))
{
	weight = sizeof(CompositeAction);
	for (const auto& action : this->actions)
		weight += action->weight;
}

void CompositeAction::forward()
{
	for (auto iter = actions.begin(); iter != actions.end(); ++iter)
		(*iter)->forward();
}

void CompositeAction::backward()
{
	for (auto iter = actions.rbegin(); iter != actions.rend(); ++iter)
		(*iter)->backward();
}

bool CompositeAction::equals(const ActionBase& other) const
{
	if (auto p = dynamic_cast<const CompositeAction*>(&other))
	{
		if (actions.size() != p->actions.size())
			return false;
		for (size_t i = 0; i < actions.size(); ++i)
			if (!actions[i]->equals(*p->actions[i]))
				return false;
		return true;
	}
	else
		return false;
}

VoidFuncAction::VoidFuncAction(void(*fore)(), void(*back)())
	: fore(fore), back(back)
{
	weight = sizeof(VoidFuncAction);
}

void VoidFuncAction::forward()
{
	fore();
}

void VoidFuncAction::backward()
{
	back();
}

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

void ActionHistory::remove_from_top(const std::shared_ptr<ActionBase>& action)
{
	auto iter = std::find(undo_deque.rbegin(), undo_deque.rend(), action);
	if (iter != undo_deque.rend())
		undo_deque.erase(--iter.base());
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
