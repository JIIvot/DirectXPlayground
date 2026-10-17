#pragma once

#ifdef _DEBUG
	#include <cstdio>
	#include <Windows.h>

	#define ASSERT_SUCCEEDED( hr, ... )                                       \
        if ( FAILED( hr ) )                                                   \
		{                                                                     \
			std::printf( "HRESULT failed in %s @ %d\n", __FILE__, __LINE__ ); \
			std::printf( "hr = 0x%08X\n", hr );                               \
			std::printf( __VA_ARGS__ "\n" );                                  \
			__debugbreak();                                                   \
        }
#else
	#define ASSERT_SUCCEEDED( hr, ... ) ( void )( hr )
#endif
