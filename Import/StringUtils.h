#pragma once
#include <charconv>
#include <string>
#include <stdexcept>

float StringToFloat(const std::string& str) {
	float value;
	auto result = std::from_chars(str.data(), str.data() + str.size(), value);
	if (result.ec != std::errc()) {
		throw std::runtime_error("Failed to convert string to float: " + str);
	}
	return value;
}

unsigned int StringToUnsignedInt(const std::string& str) {
	unsigned int value;
	auto result = std::from_chars(str.data(), str.data() + str.size(), value);
	if (result.ec != std::errc()) {
		throw std::runtime_error("Failed to convert string to unsigned int: " + str);
	}
	return value;
}

int StringToInt(const std::string& str) {
	int value;
	auto result = std::from_chars(str.data(), str.data() + str.size(), value);
	if (result.ec != std::errc()) {
		throw std::runtime_error("Failed to convert string to int: " + str);
	}
	return value;
}