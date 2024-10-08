#pragma once

#include <fstream>
#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <functional>

struct AssetValue;

class IO
{
	IO() = default;
	IO(const IO&) = delete;
	IO(IO&&) = delete;
	~IO() = default;

	static IO io;

	bool _read_file(const char* filepath, std::string& content, std::ios_base::openmode mode = std::ios_base::in);
	std::unordered_map<std::string, AssetValue> _load_asset(const char* filepath, const char* type);

public:
	static bool read_file(const char* filepath, std::string& content, std::ios_base::openmode mode = std::ios_base::in)
	{
		return io._read_file(filepath, content, mode);
	}

	static std::unordered_map<std::string, AssetValue> load_asset(const char* filepath, const char* type)
	{
		return io._load_asset(filepath, type);
	}
};

struct AssetValue
{
	std::string value;

	template<typename T>
	T parse() const;
	std::string&& moving_parse() { return std::move(value); }
	template<typename T>
	static T prim_ext(const std::string&);
	template<typename T>
	T prim() const { return AssetValue::prim_ext<T>(value); }
	template<typename T>
	std::vector<T> vec() const;
};

template<typename T>
inline T AssetValue::prim_ext(const std::string& value)
{
	if constexpr (std::is_integral_v<T>)
		return static_cast<T>(std::stol(value));
	else if constexpr (std::is_floating_point_v<T>)
		return static_cast<T>(std::stod(value));
	else if constexpr (std::is_same_v<T, std::string>)
		return value;
	else
		static_assert(false, "value type not supported in AssetValue::prim()");
};

template<typename T>
inline std::vector<T> AssetValue::vec() const
{
	std::vector<T> vec;
	std::string val;
	for (char c : value)
	{
		if (c == '|')
		{
			vec.push_back(prim_ext<T>(val));
			val.clear();
		}
		else
			val += c;
	}
	if (!val.empty())
		vec.push_back(prim_ext<T>(val));
	return vec;
};

template<typename T> struct __is_std_vector : public std::false_type {};
template<typename T, typename Alloc> struct __is_std_vector<std::vector<T, Alloc>> : public std::true_type {};
template<typename T> constexpr bool __is_std_vector_v = __is_std_vector<T>::value;

template<typename T>
inline T AssetValue::parse() const
{
	if constexpr (__is_std_vector_v<T>)
		return vec<typename T::value_type>();
	else
		return prim<T>();
}
