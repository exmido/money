#pragma once

#include "qua_http.h"
#include "stock_http.h"

//wearn
namespace wearn
{
	using namespace std;

	//qua
	int qua(string name = "qua", std::function<int32_t(stock::stock_csv&, std::string, size_t, size_t, std::ostream&)> loader = nullptr, size_t start = 1, size_t count = 50)
	{
		std::vector<string> data;
		if (0 != qua_http::load(data))
			return __LINE__;

		if (start > 0)
			--start;

		size_t end = start + count;
		if (end > data.size())
			end = data.size();

		cout << endl << name << " " << start << " " << end << endl;

		//
		std::ofstream out(name + ".bat", std::ios::binary);
		if (!out.is_open())
			return __LINE__;

		for (size_t i = start; i < end; ++i)
			out << "money " << data[i] << endl;

		out.close();

		//
		for (size_t i = start; i < end; ++i)
		{
#ifdef  WIN32
			SetConsoleTitle((name + " " + data[i] + " " + to_string(i + 1) + "/" + to_string(end)).c_str());
#endif
			stock::stock(data[i], loader);
		}

		return 0;
	}

	//qua_main
	int qua_main(int argc, char** argv, std::function<int32_t(stock::stock_csv&, std::string, size_t, size_t, std::ostream&)> loader = nullptr)
	{
		string name = "qua";
		size_t start = 1;
		size_t count = 50;

		for (int i = 1; i < argc; ++i)
		{
			if (::utility::arg_string(name, "name=", argv[i]))
				continue;

			if (::utility::arg_number(start, "start=", argv[i]))
				continue;

			if (::utility::arg_number(count, "count=", argv[i]))
				continue;
		}

		return qua(name, loader, start, count);
	}
}
