#pragma once

#include <glm/glm.hpp>

#include <string>
#include <sstream>

inline std::string str_vec(const glm::vec2& vec)
{
	std::stringstream ss;
	ss << "<" << vec.x << ", " << vec.y << ">";
	return ss.str();
}

inline std::string str_vec(const glm::vec3& vec)
{
	std::stringstream ss;
	ss << "<" << vec.x << ", " << vec.y << ", " << vec.z << ">";
	return ss.str();
}

inline std::string str_mat(const glm::mat2& mat)
{
	std::stringstream ss;
	ss << "[" << str_vec(mat[0]) << ", " << str_vec(mat[1]) << "]";
	return ss.str();
}

inline std::string str_mat(const glm::mat3& mat)
{
	std::stringstream ss;
	ss << "[" << str_vec(mat[0]) << ", " << str_vec(mat[1]) << ", " << str_vec(mat[2]) << "]";
	return ss.str();
}
