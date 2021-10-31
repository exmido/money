# CMakeList.txt : CMake project for money, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# boost
find_package (Boost)
if (Boost_FOUND)
	set (Boost_USE_STATIC_LIBS OFF)
	set (Boost_NO_BOOST_CMAKE ON)

	message (STATUS "Boost_INCLUDE_DIRS : ${Boost_INCLUDE_DIRS}")
	message (STATUS "Boost_LIBRARY_DIRS : ${Boost_LIBRARY_DIRS}")
	include_directories (${Boost_INCLUDE_DIRS})
	link_directories (${Boost_LIBRARY_DIRS})
else ()
	message (STATUS)
endif (Boost_FOUND)

# openssl
find_package (OpenSSL)
if (OpenSSL_FOUND)
	message (STATUS "OPENSSL_INCLUDE_DIR : ${OPENSSL_INCLUDE_DIR}")
	message (STATUS "OPENSSL_LIBRARIES : ${OPENSSL_LIBRARIES}")
	include_directories (${OPENSSL_INCLUDE_DIR})
	link_libraries (${OPENSSL_LIBRARIES})
else ()
	message (STATUS)
endif (OpenSSL_FOUND)

# Add source to this project's executable.
add_executable (money "money.cpp")

#gcc clang
target_link_options(money PRIVATE -static-libgcc -static-libstdc++)

# vc
set (CompilerFlags
	CMAKE_CXX_FLAGS
	CMAKE_CXX_FLAGS_DEBUG
	CMAKE_CXX_FLAGS_RELEASE
	CMAKE_C_FLAGS
	CMAKE_C_FLAGS_DEBUG
	CMAKE_C_FLAGS_RELEASE)

foreach (CompilerFlag ${CompilerFlags})
	string (REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
	string (REPLACE "/MDd" "/MTd" ${CompilerFlag} "${${CompilerFlag}}")
endforeach ()

# TODO: Add tests and install targets if needed.
