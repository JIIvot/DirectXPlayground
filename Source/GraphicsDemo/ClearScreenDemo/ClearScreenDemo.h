#pragma once

#include "GraphicsDemo/GraphicsDemo.h"

class CClearScreenDemo final : public CGraphicsDemo
{
public:
	CClearScreenDemo()
		: CGraphicsDemo()
	{
	}

	void Render() override;
};
