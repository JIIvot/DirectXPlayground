#pragma once

#include "GraphicsDemo/GraphicsDemo.h"

class CClearScreenDemo final : public CGraphicsDemo
{
public:
	CClearScreenDemo( HWND windowHandle )
		: CGraphicsDemo( windowHandle )
	{
	}

	void Render() override;
};
