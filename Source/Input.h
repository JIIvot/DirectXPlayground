#pragma once

#include <vector>
#include <cstdint>
#include <SDL3/SDL_scancode.h>

class CInput
{
public:
	void Init();

	void Update();

	[[nodiscard]]
	bool IsKeyPressed( SDL_Scancode code ) const
	{
		return m_currKeys[code];
	}

	[[nodiscard]]
	bool IsKeyJustPressed( SDL_Scancode code ) const
	{
		return m_currKeys[code] && !m_lastKeys[code];
	}

private:
	int32_t m_keyCount = 0;
	const bool* m_currKeys = nullptr;
	std::vector<uint8_t> m_lastKeys;
};
