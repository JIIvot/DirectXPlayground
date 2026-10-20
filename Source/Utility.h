#pragma once

#ifdef _DEBUG
    #include <cstdio>
    #include <Windows.h>

    #define MAKE_STRING_INTERNAL( x ) #x
    #define MAKE_STRING( x ) MAKE_STRING_INTERNAL( x )

    #define ASSERT_SUCCEEDED( hr, ... )                                                                           \
        if ( FAILED( hr ) )                                                                                       \
        {                                                                                                         \
            std::printf( "HRESULT failed in " MAKE_STRING( __FILE__ ) " at line " MAKE_STRING( __LINE__ ) "\n" ); \
            std::printf( "hr = 0x%08X\n", hr );                                                                   \
            std::printf( __VA_ARGS__ "\n" );                                                                      \
            __debugbreak();                                                                                       \
        }
#else
    #define ASSERT_SUCCEEDED( hr, ... ) ( void )( hr )
#endif
