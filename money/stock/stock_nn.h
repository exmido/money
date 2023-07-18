#pragma once

#include <iomanip>

#include "../../miapi/miapi/std/nn.h"

#include "stock_csv.h"

//stock
namespace stock
{
	//stock_nn
	class stock_nn
	{
	protected:
		string filename;

	public:
		nn::network<double> net;
		std::shared_ptr<double> work;
		std::default_random_engine re;

		//neural_reset
		void neural_reset()
		{
			net.neural_reset(re, 0, 0.01);
		}

		//size
		size_t size()
		{
			size_t ret = 0;

			for (int32_t i = 0; i < net.neurals.size(); ++i)
				ret += sizeof(double) * net.neurals[i].row() * net.neurals[i].column();

			return ret;
		}

		//load
		int32_t load(std::string _filename = "")
		{
			if (_filename != "")
				filename = _filename;

			if (filename == "")
				return __LINE__;

			//read record
			auto file = mem::load_file<double>(filename);
			if (file.first && size() == file.second)
			{
				auto m = std::make_pair<double*, size_t>((double*)file.first.get(), (size_t)file.second);

				for (int32_t i = 0; i < static_cast<int32_t>(net.neurals.size()); ++i)
				{
					auto size = sizeof(double) * net.neurals[i].row() * net.neurals[i].column();
					memcpy(&net.neurals[i].weight()[0][0], miapi::mem::serial<double>(m, size), size);
				}
			}
			else
			{
				neural_reset();
			}

			net.io_reset(1);
			return 0;
		}

		//save
		int32_t save(std::string _filename = "")
		{
			if (_filename != "")
				filename = _filename;

			if (filename == "")
				return __LINE__;

			//save net
			std::ofstream file(filename, std::ios::binary);
			if (!file.is_open())
				return __LINE__;

			//
			for (int32_t i = 0; i < static_cast<int32_t>(net.neurals.size()); ++i)
			{
				auto size = sizeof(double) * net.neurals[i].row() * net.neurals[i].column();
				file.write((char*)&net.neurals[i].weight()[0][0], size);
			}

			return 0;
		}

		//read_value
		void read_value(double* n, size_t size, stock_csv& csv, size_t column)
		{
			size_t row = csv.value[0].size();
			for (size_t i = 0; i < size; ++i)
				n[i] = csv.read(column + i / row, i % row);
		}

		//write_value
		void write_value(double* n, size_t size, stock_csv& csv, size_t column, std::ostream& out)
		{
			size_t row = csv.value[0].size();

			size_t i = 0;
			double v = csv.write(column + i / row, i % row, n[i]);
			out << fixed << setprecision(2) << v;

			for (++i; i < size; ++i)
			{
				v = csv.write(column + i / row, i % row, n[i]);
				out << " \t" << fixed << setprecision(2) << v;
			}

			out << endl;
		}

		//error_value
		double error_value(double* n, size_t size, stock_csv& csv, size_t column)
		{
			double ret = 0.0;

			size_t row = csv.value[0].size();
			for (size_t i = 0; i < size; ++i)
			{
				auto v = n[i] - csv.read(column + i / row, i % row);
				ret += (v * v);
			}

			return 0.5 * ret;
		}

		//training
		double training(stock_csv& csv, size_t index, size_t filter)
		{
			if (index <= 0)
				return DBL_MAX;

			double ret = 0;
			double t = 0;

			for (size_t i = index; i < csv.value.size() - filter - 1; ++i)
			{
				//forward
				read_value(net.in(), net.in_size() - 1, csv, i);
				net.forward();

				//back
				read_value(net.io.back().data(), net.out_size(), csv, i + filter);

				//check error
				ret = std::max(ret, error_value(net.out(), net.out_size(), csv, i + filter));
				t += 1;

				work = net.backward(net.io.back().data(), work);

				//reset padding
				for (int32_t j = 0; j < net.neurals.size(); ++j)
					net.neurals[j].in[net.neurals[j].row() - 1] = 1;
			}

			return ret;
		}

		//check
		bool check(stock_csv& csv, size_t filter)
		{
			//forward
			read_value(net.in(), net.in_size() - 1, csv, csv.value.size() - filter - 1);
			net.forward();

			//check limit
			if (net.out()[stock_csv::VALUE_HIGH] >= net.out()[stock_csv::VALUE_LOW]		// upper >= lower
				// lower <= start <= upper
				&& net.out()[stock_csv::VALUE_OPEN] >= net.out()[stock_csv::VALUE_LOW]
				&& net.out()[stock_csv::VALUE_OPEN] <= net.out()[stock_csv::VALUE_HIGH]
				// lower <= end <= upper
				&& net.out()[stock_csv::VALUE_CLOSE] >= net.out()[stock_csv::VALUE_LOW]
				&& net.out()[stock_csv::VALUE_CLOSE] <= net.out()[stock_csv::VALUE_HIGH])
			{
				return true;
			}

			return false;
		}

		//run
		bool run(stock_csv& csv, std::ostream& out, size_t index, size_t filter, size_t epoch, double error, size_t retry, size_t result = 0)
		{
			if (index <= 0)
				return false;

			//run
			for (size_t i = 1; i <= retry; ++i)
			{
				cout << "retry : " << i << "/" << retry << endl;

				double e = 0;
				for (size_t j = 1; j <= epoch; ++j)
				{
					e = training(csv, index, filter);
					if (error > e)
					{
						cout << "epoch : " << i << ", " << j << endl;

						if (check(csv, filter))
							epoch = 0;

						break;
					}
				}

				cout << fixed << setprecision(6) << e << endl;

				if (epoch)
					neural_reset();
				else
					break;
			}

			cout << endl;

			//check result
			if (epoch > 0)
				return false;

			//out
			if (result > 0)
			{
				//save net
				save();

				//save out
				size_t ri = index;
				size_t rn = csv.value.size() - filter - index;

				//outpot
				out << "999/" << std::setfill('0') << std::setw(2) << ri << "/" << std::setfill('0') << std::setw(2) << rn << "\t";
				write_value(net.out(), net.out_size(), csv, csv.value.size(), out);

				size_t index = net.in_size() - 1 - net.out_size();

				for (; result > 1; --result)
				{
					for (size_t i = 0; i < index; ++i)
						net.in()[i] = net.in()[i + net.out_size()];

					for (size_t i = 0; i < net.out_size(); ++i)
						net.in()[i + index] = net.out()[i];

					net.forward();

					//outpot
					out << "999/" << std::setfill('0') << std::setw(2) << ri << "/" << std::setfill('0') << std::setw(2) << rn << "\t";
					write_value(net.out(), net.out_size(), csv, csv.value.size(), out);
				}
			}

			return true;
		}
	};
}