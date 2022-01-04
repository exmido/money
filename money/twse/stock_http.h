#pragma once

#include <ctime>

#include "../stock/stock.h"

//twse
namespace twse
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

			auto t = time(nullptr);
			auto tm = std::localtime(&t);

			size_t year = tm->tm_year + 1900;
			size_t month = tm->tm_mon + 1;
			string str = month < 10 ? "0" : "";

			auto path = "https://www.twse.com.tw";
			if (0 != ::utility::http_load(buffer, path, string("/exchangeReport/STOCK_DAY?response=csv&date=") + std::to_string(year) + str + std::to_string(month) + string("01&stockNo=") + name, cout))
				return __LINE__;

			for (size_t i = 0; i < (filter + training + 1) / 15 + 1; ++i)
			{
				if (--month <= 0)
				{
					month = 12;
					--year;
				}

				str = month < 10 ? "0" : "";

				if (0 != ::utility::http_load(buffer, path, string("/exchangeReport/STOCK_DAY?response=csv&date=") + std::to_string(year) + str + std::to_string(month) + string("01&stockNo=") + name))
					break;
			}

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
				cd.back().push_back(std::move((*it)[3]));
				cd.back().push_back(std::move((*it)[4]));
				cd.back().push_back(std::move((*it)[5]));
				cd.back().push_back(std::move((*it)[6]));
				cd.back().push_back(std::move((*it)[1]));				
			}

			cd.front().resize(cd.back().size());

			//load
			return csv.load(cd, end, filter, training, out);
		}
	};
}
