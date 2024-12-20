#pragma once

#include <string>
#include <vector>
#include <variant>

namespace UTF
{
	extern std::u8string encode(const std::u16string& utf16, bool ignore_invalid_chars = false);
	extern std::u16string decode_utf16(const std::u8string& utf8);
	extern std::u8string encode(const std::u32string& utf32, bool ignore_invalid_chars = false);
	extern std::u32string decode_utf32(const std::u8string& utf8);
	extern std::u8string convert(const std::string& str);
	extern std::string convert(const std::u8string& utf8);

	class String
	{
		friend class Iterator;
		std::u8string str = u8"";

	public:
		String(const std::u8string& str) : str(str) {}
		String(std::u8string&& str) : str(std::move(str)) {}
		String(const std::string& str) : str(convert(str)) {}
		String(const std::u16string& str) : str(UTF::encode(str)) {}
		String(const std::u32string& str) : str(UTF::encode(str)) {}
		String(const char8_t* str) : str(str) {}
		String(const char* str) : str(convert(str)) {}
		String(const char16_t* str) : str(encode(std::u16string(str))) {}
		String(const char32_t* str) : str(encode(std::u32string(str))) {}
		String() = default;
		String(const String&) = default;
		String(String&&) noexcept = default;
		String& operator=(const String&) = default;
		String& operator=(String&&) noexcept = default;

		struct Iterator
		{
		private:
			friend class UTF::String;
			const String& string;
			size_t i;

		public:
			Iterator(const String& string, size_t i) : string(string), i(i) {}
			Iterator(const Iterator&) = default;
			Iterator(Iterator&&) noexcept = default;
			Iterator& operator=(const Iterator&) = default;
			Iterator& operator=(Iterator&&) noexcept = default;

			int codepoint() const;
			Iterator& operator++() { i += num_bytes(); return *this; }
			Iterator operator++(int) { Iterator iter(string, i); i += num_bytes(); return iter; }
			Iterator& operator--();
			Iterator operator--(int);
			bool operator==(const Iterator& other) const { return string.str == other.string.str && i == other.i; }
			bool operator!=(const Iterator& other) const { return string.str != other.string.str || i != other.i; }
			char num_bytes() const;
			operator bool() const { return i < string.str.size(); }
			int advance();
		};

		Iterator begin() const { return Iterator(*this, 0); }
		Iterator end() const { return Iterator(*this, str.size()); }
		size_t size() const { return str.size(); }
		bool empty() const { return str.empty(); }
		std::u8string& encoding() { return str; }
		const std::u8string& encoding() const { return str; }

		void push_back(int codepoint);

		String operator+(const String& rhs) const;
		String& operator+=(const String& rhs);
		String operator*(size_t n) const;
		String& operator*=(size_t n);
		bool operator==(const String& other) const { return str == other.str; }
		size_t hash() const { return std::hash<std::u8string>{}(str); }

		String(const Iterator& begin_, const Iterator& end_);
		String substr(size_t begin_, size_t end_) const;
	};
}

template<> struct std::hash<UTF::String> { size_t operator()(const UTF::String& string) const { return string.hash(); } };
