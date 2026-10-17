#pragma once

#include "GraphicsDemo/GraphicsDemo.h"

class CClearScreenDemo final : public CGraphicsDemo
{
public:
	CClearScreenDemo( SDL_Window* window, HWND windowHandle )
		: CGraphicsDemo( window, windowHandle )
	{
	}

	void Render() override;

private:
	void BuildImmediateUi();
};
