set(BOOST_VERSION "1.83.0")
set(BOOST_URL "https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_MAJOR}_${BOOST_VERSION_MINOR}_${BOOST_VERSION_PATCH}.tar.gz")

# 下载 Boost
include(FetchContent)
FetchContent_Declare(
        boost
        URL ${BOOST_URL}
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/boost
)

FetchContent_MakeAvailable(boost)