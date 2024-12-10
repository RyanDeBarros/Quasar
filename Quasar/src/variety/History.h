#pragma once

#include <functional>
#include <queue>
#include <memory>

#define QUASAR_ACTION_EQUALS_OVERRIDE(structname)\
	bool operator==(const structname&) const = default;\
	virtual bool equals(const ActionBase& other) const override { auto p = dynamic_cast<const structname*>(&other); return p && *this == *p; }

// LATER why use shared_ptrs instead of unique_ptrs?
struct ActionBase
{
	size_t weight = sizeof(ActionBase);

	virtual ~ActionBase() = default;

	virtual void forward() = 0;
	virtual void backward() = 0;
	virtual bool equals(const ActionBase&) const { return false; }
	inline bool operator==(const ActionBase&) const = default;
};

struct InverseAction : public ActionBase
{
	std::shared_ptr<ActionBase> normal;

	InverseAction(const std::shared_ptr<ActionBase>& normal);
	InverseAction(std::shared_ptr<ActionBase>&& normal);
	virtual void forward() override;
	virtual void backward() override;
	virtual bool equals(const ActionBase&) const override;
	inline bool operator==(const InverseAction&) const = default;
};

struct DualAction : public ActionBase
{
	std::shared_ptr<ActionBase> first, second;

	DualAction(const std::shared_ptr<ActionBase>& first, const std::shared_ptr<ActionBase>& second);
	DualAction(std::shared_ptr<ActionBase>&& first, std::shared_ptr<ActionBase>&& second);
	virtual void forward() override;
	virtual void backward() override;
	virtual bool equals(const ActionBase&) const override;
	inline bool operator==(const DualAction&) const = default;
};

struct CompositeAction : public ActionBase
{
	std::vector<std::shared_ptr<ActionBase>> actions;

	CompositeAction(const std::vector<std::shared_ptr<ActionBase>>& actions);
	CompositeAction(std::vector<std::shared_ptr<ActionBase>>&& actions);
	virtual void forward() override;
	virtual void backward() override;
	virtual bool equals(const ActionBase&) const override;
	inline bool operator==(const CompositeAction&) const = default;
};

struct VoidFuncAction : public ActionBase
{
	void(*fore)();
	void(*back)();
	VoidFuncAction(void(*fore)(), void(*back)());
	virtual void forward() override;
	virtual void backward() override;
	QUASAR_ACTION_EQUALS_OVERRIDE(VoidFuncAction)
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

	void execute(std::shared_ptr<ActionBase>&& action);
	void execute(const std::shared_ptr<ActionBase>& action);

private:
	void add_weight(size_t weight);

public:
	void push(const std::shared_ptr<ActionBase>& action);
	void push(std::shared_ptr<ActionBase>&& action);
	void execute_no_undo(const std::shared_ptr<ActionBase>& action);
	void clear_history();
	void undo();
	size_t undo_size() const;
	void redo();
	size_t redo_size() const;
	size_t get_current_memory_usage() const;
	float get_fraction_of_memory_usage() const;
	size_t get_pool_size() const;
	void set_pool_size(size_t pool_size);
};
