#pragma once

#include "../utility.h"

#include "stock_csv.h"
#include "stock_nn.h"

//stock
namespace stock
{
	//stock
	bool stock(stock_csv& csv, std::ostream& out, string name, size_t filter = 15, size_t training = 9, size_t index = 1, size_t result = 10)
	{
		//load nn
		size_t row = csv.value[0].size();
		size_t inner_size = row * filter + row;

		stock_nn nn;
		nn.net.reset({ row * filter, inner_size, inner_size, inner_size, row }, 1.0f);

		nn.load(name);

		//run
		for (; index < csv.value.size() - filter - 1; ++index)
		{
			cout << "index : " << index << endl;

			if (false == nn.run(csv, out, index, filter, 1000, 0.1, 100))
				continue;

			if (false == nn.run(csv, out, index, filter, 10000, 0.01, 10))
				continue;

			if (false == nn.run(csv, out, index, (size_t)filter, 100000, 0.001, 1, (size_t)result))
				continue;

			cout << endl << "Succeed";
			return true;
		}

		cout << endl << "Fail";
		//cin.get();
		return false;
	}

	int32_t stock(string name, std::function<int32_t(stock::stock_csv& csv, std::string name, size_t filter, size_t training, std::ostream& out)> loader = nullptr, size_t filter = 15, size_t training = 9, size_t index = 1, size_t result = 10)
	{
		//out
		std::ofstream out(name + ".txt", std::ios::binary);
		if (!out.is_open())
			return __LINE__;

		//csv
		stock_csv csv;
		if (csv.load(name, filter, training, out) != 0)
		{
			if (loader == nullptr || loader(csv, name, filter, training, out) != 0)
				return __LINE__;
		}

		cout << endl;

		if (!stock(csv, out, name + ".net", filter, training, index, result))
			return __LINE__;

		return 0;
	}

	//stock_main
	int stock_main(int argc, char** argv, std::function<int32_t(stock::stock_csv&, std::string, size_t, size_t, std::ostream&)> loader = nullptr)
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

		return stock(name, loader, filter, training, index, result);
	}
}
