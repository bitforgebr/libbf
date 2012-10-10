/*
 * rawstring.h
 *
 *  Created on: Apr 05, 2012
 *      Author: gianni
 *
 * BitForge http://www.bitforge.com.br
 * Copyright (c) 2012 All Right Reserved,
 */

#ifndef __INCLUDE_LIBBF_RAWSTRING_H_
#define __INCLUDE_LIBBF_RAWSTRING_H_

#include <string>
#include <cstring>
#include <memory>

namespace bitforge
{

/**
 * @class RawString
 * @description Thread-safe string that breaks std::string COW's problems
 */
class RawString
{
private:
	typedef std::shared_ptr<char> CharPtr;
	static void CharDeleter(char* c) { delete[] c; };

	CharPtr		m_data;
	std::size_t	m_length;

public:
	RawString(): m_length(0) {}

	RawString(const char* str, std::size_t length) : m_length(length)
	{
		char *c = new char[m_length + 1];
		memcpy(c, str, m_length);
		c[m_length] = 0;
		m_data = CharPtr(c, CharDeleter);
	}

	RawString(const char* str)
	{
		m_length = strlen(str);
		char *c = new char[m_length + 1];
		memcpy(c, str, m_length);
		c[m_length] = 0;
		m_data = CharPtr(c, CharDeleter);
	}

	RawString(const std::string& str)
	{
		m_length = str.length();
		char *c = new char[m_length + 1];
		memcpy(c, str.c_str(), m_length);
		c[m_length] = 0;
		m_data = CharPtr(c, CharDeleter);
	}

	operator std::string() const
	{
		std::string result;
		if (m_data)
			result.assign(m_data.get(), m_length);
		return result;
	}

	bool operator<(const RawString& other) const
	{
		const char* t = data();
		const char* o = other.data();
		if (t == nullptr)
			return true;
		else if (o == nullptr)
			return false;
		else
			return std::strcmp(t, o) < 0;
	}

	const char* data() const
	{
		if (m_data)
			return m_data.get();
		return nullptr;
	}

	const char* c_str() const
	{
		return data();
	}

	std::size_t length() const
	{
		return m_length;
	}

	void clear()
	{
		m_data.reset();
		m_length = 0;
	}

	bool empty() const
	{
		return m_length == 0;
	}

	int compare(const char* other) const
	{
		return std::strcmp(m_data.get(), other);
	}
};

inline std::ostream& operator<<(std::ostream& out, const RawString& string)
{
	out << string.data();
	return out;
}

} // bitforge

#endif // __INCLUDE_LIBBF_RAWSTRING_H_

