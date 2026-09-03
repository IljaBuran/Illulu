#pragma once

/* Common headers */
#include "Types.hpp"
#include "Debug.hpp"
#include "Constants.hpp"

/* Logging */
#include "Core/Console.hpp"
#define INFO(message, ...) Logger::Log(LogType::INFO, message, __VA_ARGS__)
#define WARNING(message, ...) Logger::Log(LogType::WARNING, message, __VA_ARGS__)
#define FATAL(message, ...) do                           \
{                                                        \
	Logger::Log(LogType::FATAL, message, __VA_ARGS__);  \
	__debugbreak();                                      \
} while(0)

#define UNUSED [[maybe_unused]]