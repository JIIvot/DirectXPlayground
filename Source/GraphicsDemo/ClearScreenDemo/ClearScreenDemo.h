#pragma once

#include "GraphicsDemo/GraphicsDemo.h"

#include <string>

class CClearScreenDemo final : public CGraphicsDemo
{
public:
	CClearScreenDemo( HWND windowHandle );

	void Render() override;

private:
	void BuildImmediateUi() const;

private:
	std::wstring m_adapterName;
};
