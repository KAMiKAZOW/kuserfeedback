add_subdirectory(common)
add_subdirectory(provider)

if (ENABLE_PHP AND ENABLE_PHP_UNIT)
    add_subdirectory(server)
    add_subdirectory(testserver)
endif()

list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_generic_lambdas has_generic_lambdas)
if(Qt5Charts_FOUND AND Qt5Svg_FOUND AND Qt5PrintSupport_FOUND AND NOT has_generic_lambdas LESS 0 AND NOT Qt5Core_VERSION VERSION_LESS 5.8)
    if (ENABLE_CONSOLE)
        add_subdirectory(console)
    endif()

    if (ENABLE_CLI)
        add_subdirectory(cli)
    endif()
endif()
