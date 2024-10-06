#pragma once

#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <tuple>
#include <string>

template<typename T, typename = void>
struct __reg_casts_to_bool : std::false_type {};
template<typename T>
struct __reg_casts_to_bool<T, std::void_t<decltype(static_cast<bool>(std::declval<T>()))>> : std::true_type {};
template<typename T>
static constexpr bool __reg_casts_to_bool_v = __reg_casts_to_bool<T>::value;
template<typename T, typename = void>
struct __reg_overloads_equals : std::false_type {};
template<typename T>
struct __reg_overloads_equals<T, std::void_t<decltype(std::declval<T>() == std::declval<T>())>> : std::true_type {};
template<typename T>
static constexpr bool __reg_overloads_equals_v = __reg_overloads_equals<T>::value;
template<typename T, typename = void>
struct __reg_is_hashable : std::false_type {};
template<typename T>
struct __reg_is_hashable < T, std::void_t<decltype(std::hash<T>{}(std::declval<T>())) >> : std::true_type {};
template<typename T>
static constexpr bool __reg_is_hashable_v = __reg_is_hashable<T>::value;

template<typename _Element, typename _Handle, typename... _Constructors>
class Registry
{
	static_assert(std::is_integral_v<_Handle>&& std::is_unsigned_v<_Handle>, "Handle type must be an unsigned integral.");
	static_assert(((__reg_is_hashable_v<_Constructors>&& __reg_overloads_equals_v<_Constructors>) && ...), "All constructor types must be hashable - i.e., specialize std::hash and overload ==.");
	static constexpr bool VALIDATE_CONSTRUCTION = __reg_casts_to_bool_v<_Element>;

public:
	struct Handle
	{
		_Handle _v;
		constexpr explicit Handle(_Handle v = _Handle()) : _v(v) {}
		constexpr Handle(const Handle&) = default;
		constexpr Handle(Handle&&) = default;
		constexpr Handle& operator=(const Handle&) = default;
		constexpr Handle& operator=(Handle&&) = default;
		constexpr operator _Handle() const { return _v; }
		constexpr bool operator==(const Handle&) const = default;
		constexpr Handle operator++(int) { return Handle(_v++); }
	};
	struct HandleHash
	{
		size_t operator()(const Handle& handle) const
		{
			return std::hash<_Handle>{}(handle._v);
		}
	};

private:
	Handle _current = Handle(1);
	std::unordered_map<Handle, _Element, HandleHash> _data;
	std::tuple<std::unordered_map<_Constructors, Handle>...> _lookups;

public:
	static constexpr Handle CAP = Handle(-1);

	Registry() = default;
	Registry(const Registry<_Element, _Handle, _Constructors...>&) = default;
	Registry(Registry<_Element, _Handle, _Constructors...>&&) = default;
	~Registry() = default;

	const _Element* get(Handle handle) const;
	_Element* get(Handle handle);
	bool destroy(Handle handle);
	Handle add(_Element&& element);
	template<typename Constructor>
	Handle construct(const Constructor& constructor);
	template<typename Constructor>
	Handle construct(Constructor&& constructor);

	struct full_error : public std::runtime_error
	{
		full_error() : std::runtime_error("Registry is full: CAP=" + std::to_string(CAP)) {}
	};
};
template<typename _Element, typename _Handle, typename ..._Constructors>
inline const _Element* Registry<_Element, _Handle, _Constructors...>::get(Handle handle) const
{
	if (handle == _Handle(0)) return nullptr;
	auto iter = _data.find(handle);
	return iter != _data.end() ? &iter->second : nullptr;
}
template<typename _Element, typename _Handle, typename ..._Constructors>
inline _Element* Registry<_Element, _Handle, _Constructors...>::get(Handle handle)
{
	if (handle == _Handle(0)) return nullptr;
	auto iter = _data.find(handle);
	return iter != _data.end() ? &iter->second : nullptr;
}
template<typename _Element, typename _Handle, typename ..._Constructors>
inline bool Registry<_Element, _Handle, _Constructors...>::destroy(Handle handle)
{
	if (handle == _Handle(0)) return false;
	auto iter = _data.find(handle);
	if (iter == _data.end())
		return false;
	_data.erase(iter);
	return true;
}
template<typename _Element, typename _Handle, typename ..._Constructors>
inline Registry<_Element, _Handle, _Constructors...>::Handle Registry<_Element, _Handle, _Constructors...>::add(_Element&& element)
{
	if (_current == CAP)
		throw Registry::full_error();
	if constexpr (VALIDATE_CONSTRUCTION)
	{
		if (!element)
			return Handle(0);
	}
	Handle handle = _current++;
	_data.emplace(handle, std::move(element));
	return handle;
}
template<typename _Element, typename _Handle, typename ..._Constructors>
template<typename Constructor>
inline Registry<_Element, _Handle, _Constructors...>::Handle Registry<_Element, _Handle, _Constructors...>::construct(const Constructor& constructor)
{
	auto& lookup = std::get<std::unordered_map<Constructor, Handle>>(_lookups);
	auto iter = lookup.find(constructor);
	if (iter != lookup.end())
		return iter->second;
	if (_current == CAP)
		throw Registry::full_error();
	_Element element(constructor);
	if constexpr (VALIDATE_CONSTRUCTION)
	{
		if (!element)
			return Handle(0);
	}
	Handle handle = _current++;
	_data.emplace(handle, std::move(element));
	lookup.emplace(constructor, handle);
	return handle;
}
template<typename _Element, typename _Handle, typename ..._Constructors>
template<typename Constructor>
inline Registry<_Element, _Handle, _Constructors...>::Handle Registry<_Element, _Handle, _Constructors...>::construct(Constructor&& constructor)
{
	auto& lookup = std::get<std::unordered_map<Constructor, Handle>>(_lookups);
	auto iter = lookup.find(constructor);
	if (iter != lookup.end())
		return iter->second;
	if (_current == CAP)
		throw Registry::full_error();
	_Element element(constructor);
	if constexpr (VALIDATE_CONSTRUCTION)
	{
		if (!element)
			return Handle(0);
	}
	Handle handle = _current++;
	_data.emplace(handle, std::move(element));
	lookup.emplace(std::move(constructor), handle);
	return handle;
}
