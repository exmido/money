#pragma once

#include "../utility.h"

#include "stock_csv.h"

//stock
namespace stock
{
	//stock_http
	class stock_http
	{
	protected:
		enum
		{
			EMPTY = 1,
			NEW_LINE,
			STRING,
			TEMP,
		};

		//scanner_param
		class scanner_param
		{
		public:
			using data_type = typename std::list<std::vector<utf::utf8*>>;

			data_type datas;

			//scanner_param
			scanner_param()
			{
				datas.push_back({});
			}
		};

		//scanner_rule
		class scanner_rule : public miw::miw_syntax_rule::rule_base<utf::utf8*, scanner_param>
		{
		public:
			int32_t type;

			//scanner_rule
			scanner_rule(int32_t _type = 0)
				: type(_type)
			{
			}

			//result
			virtual bool result(bool b, iterator& begin, const iterator& end, parameter& param) override
			{
				if (!b)
					return false;

				switch (type)
				{
				case EMPTY:
					for (auto it = begin; it != end; ++it)
						*it = 0;
					break;

				case NEW_LINE:
					param.datas.push_back({});
					break;

				case STRING:
					param.datas.back().push_back(begin);
					break;

				case TEMP:
					if (*begin == '&')
						*begin = 0;
					break;

				default:
					break;
				}

				return true;
			}
		};

		//scanner
		class scanner : public miw::miw_syntax<utf::utf8*, typename scanner_rule::parameter, scanner_rule>
		{
		public: //global
			using base_type = miw::miw_syntax<utf::utf8*, typename scanner_rule::parameter, scanner_rule>;

			using r_and = miw::miw_syntax_rule::rule_and<utf::utf8*, parameter, rule_base_type>;
			using r_or = miw::miw_syntax_rule::rule_or<utf::utf8*, parameter, rule_base_type>;

			using r_range = miw::miw_syntax_rule::scanner_range<utf::utf8*, parameter, rule_base_type>;
			using r_char = miw::miw_syntax_rule::scanner_char<utf::utf8*, parameter, rule_base_type>;
			using r_nchar = miw::miw_syntax_rule::scanner_not_char<utf::utf8*, parameter, rule_base_type>;
			using r_str = miw::miw_syntax_rule::scanner_string<utf::utf8*, parameter, rule_base_type>;
			using r_nstr = miw::miw_syntax_rule::scanner_not_string<utf::utf8*, parameter, rule_base_type>;

		public: //local
			//scanner
			scanner(std::string delimiters = ",")
			{
				using namespace std;

				auto root = make_shared<r_or>();
				rules = root;

				//data
				auto r_data = make_shared<r_and>();
				r_data
					->a(make_shared<r_or>()
						->o(make_shared<r_str>(EMPTY)->reset("<td align=\"center\">"))
						->o(make_shared<r_str>(EMPTY)->reset("<td align=\"right\">")))
					->a(make_shared<r_and>(STRING))
					->a(make_shared<r_nstr>(TEMP)->reset("<"), '*')
					->a(make_shared<r_str>(EMPTY)->reset("</td>"));

				//new line
				char* newline[] = { "<tr class=\"stockalllistbg2\">", "<tr class=\"stockalllistbg1\">" };

				auto r_begin = make_shared<r_or>(NEW_LINE);
				r_begin
					->o(make_shared<r_str>(EMPTY)->reset(newline[0]))
					->o(make_shared<r_str>(EMPTY)->reset(newline[1]));

				auto r_end = make_shared<r_and>();
				r_end
					->a(make_shared<r_and>()
						->a(make_shared<r_nstr>()->reset("<"), '*')
						->a(r_data), '*')
					->a(make_shared<r_nstr>()->reset("<"), '*')
					->a(make_shared<r_str>(EMPTY)->reset("</tr>"));

				root->o(make_shared<r_and>()
					->a(r_begin)
					->a(r_end));

				//empty
				root->o(make_shared<r_nstr>()->reset("<tr class=\"stockalllistbg"), '*');
			}

			//run
			bool run(utf::utf8*& it, utf::utf8* const& end, parameter& param, char c = '\0')
			{
				if (base_type::run(it, end, param, c))
				{
					if (param.datas.back().size() == 0)
						param.datas.pop_back();

					return true;
				}

				return false;
			}
		};

	public:
		//load
		static int32_t load(http_csv& csv, string id, size_t filter, size_t training, std::ostream& out)
		{
			//download
			string buffer;
			buffer.reserve(128 * 1024);

			auto path = "https://stock.wearn.com";
			if (0 != ::utility::http_load(buffer, path, string("/cdata.asp?kind=") + id, cout))
				return __LINE__;

			//year
			size_t year = 0;
			string str = " Selected>";
			size_t pos = buffer.find(str);
			if (pos != std::string::npos)
			{
				pos += str.size();
				year = utf::wton<size_t>(buffer.c_str() + pos).first;
			}

			//month
			size_t month = 0;
			str = "\"selected>";
			pos = buffer.find(str);
			if (pos != std::string::npos)
			{
				pos += str.size();
				month = utf::wton<size_t>(buffer.c_str() + pos).first;
			}

			if (year != 0 && month != 0)
			{
				for (int i = 0; i < (filter + training + 1) / 15 + 1; ++i)
				{
					if (--month <= 0)
					{
						month = 12;
						--year;
					}

					str = month < 10 ? "0" : "";

					if (0 != ::utility::http_load(buffer, path, string("/cdata.asp?Year=") + std::to_string(year) + string("&month=") + str + std::to_string(month) + string("&kind=") + id))
						break;
				}
			}

			cout << endl;

			//scanner
			scanner s;
			typename decltype(s)::parameter p;

			auto begin = (utf::utf8*)buffer.c_str();
			auto end = begin + buffer.size();

			if (false == s.run(begin, end, p, '*'))
				return __LINE__;

			if (p.datas.size() <= 0)
				return __LINE__;

			//format
			std::vector<std::vector<utf::utf8*>> cd;
			cd.reserve(p.datas.size());

			for (auto it = p.datas.begin(); it != p.datas.end(); ++it)
				cd.push_back(std::move(*it));

			cd.front().resize(cd.back().size());

			//load
			return csv.load(cd, end, filter, training, out);
		}
	};
}
