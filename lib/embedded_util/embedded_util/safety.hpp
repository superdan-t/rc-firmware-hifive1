/// Macros for preventing common mistakes in interface design

#pragma once

#define DISALLOW_COPY_AND_MOVE(classname)                           \
	classname(const classname&) = delete;                           \
	classname& operator=(const classname&) = delete;                \
	classname(classname&&) = delete;                                \
	classname& operator=(classname&&) = delete
