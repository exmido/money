#pragma once

#include <iostream>
#include <map>

#include "../../miapi/miapi/std/csv_syntax.h"

//stock
namespace stock
{
	using namespace std;
	using namespace miapi;

	//http_csv
	class http_csv
	{
	protected:
		//encode
		double encode(double value, double base)
		{
			return ((value - base) / base + 1.0) / 2.0;
		}

		//decode
		double decode(double value, double base)
		{
			return base + base * (value * 2.0 - 1.0);
		}

		//format
		utf::utf8* format(utf::utf8* begin, utf::utf8* end)
		{
			int32_t index = 0;
			for (auto it = begin; *it != 0 && it != end; ++it)
			{
				if (*it == ',')
				{
					*it++ = 0;
					if (*it == 0)
						break;

					++index;
				}

				if (index)
					std::swap(*it, *(it - index));
			}

			return begin;
		}

	public:
		enum
		{
			VALUE_OPEN,
			VALUE_HIGH,
			VALUE_LOW,
			VALUE_CLOSE,

			VALUE_UNKNOWN
		};

		std::vector<std::vector<double>> value;

		//read
		double read(size_t c, size_t r)
		{
			return encode(value[c][r], value[c - 1][r < http_csv::VALUE_UNKNOWN ? http_csv::VALUE_CLOSE : r]);
		}

		//write
		double write(size_t c, size_t r, double v)
		{
			while (value.size() <= c)
			{
				value.push_back({});
				value.back().resize(value[0].size());
			}

			return value[c][r] = decode(v, value[c - 1][r < http_csv::VALUE_UNKNOWN ? http_csv::VALUE_CLOSE : r]);
		}

		//load
		int32_t load(csv_syntax<utf::utf8*>::data_type& cd, utf::utf8* end, size_t filter, size_t training, std::ostream& out)
		{
			//check data size
			if (cd.size() < 3 + filter) //header + base + filter + 1
				return __LINE__;

			//read data
			std::map<size_t, size_t> dateindex;
			for (size_t i = 1; i < cd.size(); ++i)
			{
				auto tok = utf::wton<size_t>(cd[i][0]);
				auto key = tok.first * 10000;

				tok = utf::wton<size_t>(tok.second + utf::wcnext(tok.second));
				key += tok.first * 100;

				tok = utf::wton<size_t>(tok.second + utf::wcnext(tok.second));
				key += tok.first;

				dateindex.insert(std::make_pair(key, i));
			}

			while (dateindex.size() > filter + training + 1)
				dateindex.erase(dateindex.begin());

			cout << "load : " << cd[0].size() << " x " << dateindex.size() << endl;

			//out
			out << "DATE";
			for (size_t i = 1; i < cd[0].size(); ++i)
			{
				switch (i - 1)
				{
				case VALUE_OPEN:
					out << "\tOPEN";
					break;
				case VALUE_HIGH:
					out << "\tHIGH";
					break;
				case VALUE_LOW:
					out << "\tLOW";
					break;
				case VALUE_CLOSE:
					out << "\tCLOSE";
					break;
				default:
					out << "\tVOLUME";
					break;
				}
			}

			out << endl;

			for (auto it = dateindex.begin(); it != dateindex.end(); ++it)
			{
				value.push_back({});
				out << cd[it->second][0];

				for (int32_t j = 1; j < cd[it->second].size(); ++j)
				{
					value.back().push_back(utf::wton<double>(format(cd[it->second][j], end)).first);
					out << "\t" << cd[it->second][j];
				}

				out << endl;
			}

			return 0;
		}

		int32_t load(std::string filename, size_t filter, size_t training, std::ostream& out)
		{
			//load file
			auto file = mem::load_file<utf::utf8>(filename);
			if (file.first == nullptr)
				return __LINE__;

			csv_syntax<utf::utf8*> c("\t");
			decltype(c)::data_type cd;

			//check csv file
			if (!c.read(cd, utf::skipbom(file.first.get()), mem::offset(file.first.get(), file.second)))
				return __LINE__;

			cout << filename << endl << endl;
			return load(cd, file.first.get() + file.second, filter, training, out);
		}
	};
}
