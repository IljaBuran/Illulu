#pragma once

/* Common headers */
#include "Types.hpp"
#include "Debug.hpp"
#include "Constants.hpp"

/* Logging */
#include "Core/Console.hpp"
#define INFO(message, ...) Console::Log(LogType::INFO, message, __VA_ARGS__)
#define WARNING(message, ...) Console::Log(LogType::WARNING, message, __VA_ARGS__)
#define FATAL(message, ...) Console::Log(LogType::FATAL, message, __VA_ARGS__)

#define UNUSED [[maybe_unused]]