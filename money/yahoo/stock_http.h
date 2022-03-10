#pragma once

#include <ctime>

#include "../stock/stock.h"

//yahoo
namespace yahoo
{
	using namespace stock;

	//stock_http
	class stock_http
	{
	public:
		//load
		static int32_t load(stock_csv& csv, string name, size_t filter, size_t training, std::ostream& out)
		{
			//download
			string buffer;
			buffer.reserve(128 * 1024);

			auto t = time(nullptr) + 60 * 60 * 24;
			t -= (t % (60 * 60 * 24));

			auto path = "https://query1.finance.yahoo.com";
			if (0 != ::utility::http_load(buffer, path, string("/v7/finance/download/") + name + string("?period1=") + std::to_string(t - 60 * 60 * 24 * 60) + string("&period2=") + std::to_string(t) + string("&interval=1d&events=history&includeAdjustedClose=true"), cout))
				return __LINE__;

			cout << endl;

			//read
			auto begin = (utf::utf8*)buffer.c_str();
			auto end = begin + buffer.size();

			csv_syntax<utf::utf8*> c(",");
			decltype(c)::parameter p;

			if (!c.read(p, begin, end))
				return __LINE__;

			//format
			decltype(c)::data_type cd;
			cd.push_back(decltype(cd)::value_type());

			for (auto it = p.datas.begin(); it != p.datas.end(); ++it)
			{
				if (it->size() <= 1)
					continue;

				if (utf::wton<size_t>((*it)[0]).first == 0)
					continue;

				cd.push_back(decltype(cd)::value_type());
				cd.back().push_back(std::move((*it)[0]));
				cd.back().push_back(std::move((*it)[1]));
				cd.back().push_back(std::move((*it)[2]));
				cd.back().push_back(std::move((*it)[3]));
				cd.back().push_back(std::move((*it)[4]));
				cd.back().push_back(std::move((*it)[6]));
			}

			cd.front().resize(cd.back().size());

			//load
			return csv.load(cd, end, filter, training, out);
		}
	};
}
