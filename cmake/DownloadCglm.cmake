cmake_minimum_required(VERSION 3.16)

include(FetchContent)
FetchContent_GetProperties(cglm)

if (NOT cglm_POPULATED)
    FetchContent_Declare(
            cglm
            GIT_REPOSITORY https://github.com/recp/cglm
    )
    FetchContent_MakeAvailable(cglm)
endif ()
