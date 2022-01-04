#pragma once

#include <iostream>

#include "../../miapi/miapi/std/utility.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "C:/repos/cpp-httplib/httplib.h"

//utility
namespace utility
{
	using namespace std;
	using namespace miapi;

	//http_load
	static int32_t http_load(std::string& ret, std::string path, std::string str, std::ostream& out = miapi::utility::nullstream::cnil())
	{
		httplib::Client cli(path);
		cli.set_keep_alive(false);

		auto res = cli.Get(str.c_str());
		if (res == nullptr)
			return __LINE__;

		out << res->status << "\t" << path + str << std::endl;

		if (res->status != 200)
			return __LINE__;

		ret += res->body;
		return 0;
	}

	//arg_string
	template<typename T>
	bool arg_string(T& ret, const char* str, char* arg)
	{
		size_t size = utf::wstrlen(str);

		if (utf::wstrncmp(arg, str, size))
			return false;

		ret = T(arg + size);
		cout << str << ret << endl;
		return true;
	}

	//arg_number
	template<typename T>
	bool arg_number(T& ret, const char* str, char* arg)
	{
		size_t size = utf::wstrlen(str);

		if (utf::wstrncmp(arg, str, size))
			return false;

		ret = utf::wton<T>(arg + size).first;
		cout << str << ret << endl;
		return true;
	}
}
