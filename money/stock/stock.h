#pragma once

#include "stock_csv.h"
#include "stock_http.h"
#include "stock_nn.h"

//stock
namespace stock
{
	//stock
	int stock(string name, size_t filter = 15, size_t training = 9, size_t index = 1, size_t result = 10)
	{
		http_csv csv;
		stock_nn nn;

		std::ofstream out(name + ".txt", std::ios::binary);
		if (!out.is_open())
			return __LINE__;

		//load csv
		if (csv.load(name, filter, training, out) != 0 && stock_http::load(csv, name, filter, training, out) != 0)
			return __LINE__;

		cout << endl;

		//load nn
		size_t row = csv.value[0].size();
		size_t inner_size = row * filter + row;
		nn.net.reset({ row * filter, inner_size, inner_size, inner_size, row }, 1.0f);

		nn.load(name + ".net");

		//run
		int ret = 0;
		for (; index < csv.value.size() - filter - 1; ++index)
		{
			cout << "index : " << index << endl;

			ret = nn.run(csv, out, index, filter, 1000, 0.1, 100);
			if (ret != 0)
				continue;

			ret = nn.run(csv, out, index, filter, 10000, 0.01, 10);
			if (ret != 0)
				continue;

			ret = nn.run(csv, out, index, (size_t)filter, 100000, 0.001, 1, (size_t)result);
			if (ret != 0)
				continue;

			cout << endl << "Succeed";
			return 0;
		}

		cout << endl << "Fail";
		//cin.get();
		return ret;
	}

	//stock_main
	int stock_main(int argc, char** argv)
	{
		string name(1 < argc ? argv[1] : "");

		if (name == "")
		{
			cout << "(<filename> | <id>) <filter=15>? <training=9>? <index=1>? <result=10>?" << endl;
			return __LINE__;
		}

#ifdef  WIN32
		SetConsoleTitle(name.c_str());
#endif

		size_t filter = 15;
		size_t training = 9;
		size_t index = 1;
		size_t result = 10;

		for (int i = 2; i < argc; ++i)
		{
			if (::utility::arg_number(filter, "filter=", argv[i]))
				continue;

			if (::utility::arg_number(training, "training=", argv[i]))
				continue;

			if (::utility::arg_number(index, "index=", argv[i]))
				continue;

			if (::utility::arg_number(result, "result=", argv[i]))
				continue;
		}

		return stock(name, filter, training, index, result);
	}
}
