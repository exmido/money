#pragma once

#include <iomanip>

#include "../../miapi/miapi/std/neural_network.h"

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
		neural_network::network_backprop<neural_network::node_fun_lrelu<neural_network::network_backprop_node<double>>> net;

		//size
		size_t size()
		{
			return (net.end() - net.begin()) * sizeof(*net.begin());
		}

		//random_weights
		void random_weights()
		{
			net.random_weights(0, 0.01);
		}

		//load
		int32_t load(std::string _filename = "")
		{
			if (_filename != "")
				filename = _filename;

			if (filename == "")
				return __LINE__;

			//read record
			auto file = mem::load_file<utf::utf8>(filename);
			if (file.first && size() == file.second)
				memcpy(net.begin(), file.first.get(), file.second);
			else
				random_weights();

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

			file.write((char*)net.begin(), size());

			return 0;
		}

		//read_value
		void read_value(typename decltype(net)::node_type* n, size_t size, stock_csv& csv, size_t column)
		{
			size_t row = csv.value[0].size();
			for (size_t i = 0; i < size; ++i)
				n[i].value = csv.read(column + i / row, i % row);
		}

		//write_value
		void write_value(decltype(net)::node_type* n, size_t size, stock_csv& csv, size_t column, std::ostream& out)
		{
			size_t row = csv.value[0].size();

			size_t i = 0;
			double v = csv.write(column + i / row, i % row, n[i].value);
			out << fixed << setprecision(2) << v;

			for (++i; i < size; ++i)
			{
				v = csv.write(column + i / row, i % row, n[i].value);
				out << " \t" << fixed << setprecision(2) << v;
			}

			out << endl;
		}

		//error_value
		double error_value(typename decltype(net)::node_type* n, size_t size, stock_csv& csv, size_t column)
		{
			double ret = 0.0;

			size_t row = csv.value[0].size();
			for (size_t i = 0; i < size; ++i)
				ret += abs(n[i].value - csv.read(column + i / row, i % row));

			return ret;
		}

		//training
		double training(stock_csv& csv, size_t index, size_t filter, double rate, double momentum)
		{
			if (index <= 0)
				return DBL_MAX;

			double ret = 0;
			double t = 0;

			for (size_t i = index; i < csv.value.size() - filter - 1; ++i)
			{
				//forward
				read_value(net.input_array(), net.input_size() - 1, csv, i);
				net.culcate();

				//back
				read_value(net.target_array(), net.target_size(), csv, i + filter);

				//check error
				ret = std::max(ret, error_value(net.output_array(), net.output_size(), csv, i + filter));
				t += 1;

				net.change_weights(rate, momentum);
			}

			return ret;
		}

		//check
		bool check(stock_csv& csv, size_t filter)
		{
			//forward
			read_value(net.input_array(), net.input_size() - 1, csv, csv.value.size() - filter - 1);
			net.culcate();

			//check limit
			if (net.output_array()[stock_csv::VALUE_HIGH].value >= net.output_array()[stock_csv::VALUE_LOW].value		// upper >= lower
				// lower <= start <= upper
				&& net.output_array()[stock_csv::VALUE_OPEN].value >= net.output_array()[stock_csv::VALUE_LOW].value
				&& net.output_array()[stock_csv::VALUE_OPEN].value <= net.output_array()[stock_csv::VALUE_HIGH].value
				// lower <= end <= upper
				&& net.output_array()[stock_csv::VALUE_CLOSE].value >= net.output_array()[stock_csv::VALUE_LOW].value
				&& net.output_array()[stock_csv::VALUE_CLOSE].value <= net.output_array()[stock_csv::VALUE_HIGH].value)
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
					e = training(csv, index, filter, 0.5, 0.05);
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
					random_weights();
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
				write_value(net.output_array(), net.output_size(), csv, csv.value.size(), out);

				size_t index = net.input_size() - 1 - net.output_size();

				for (; result > 1; --result)
				{
					for (size_t i = 0; i < index; ++i)
						net.input_array()[i] = net.input_array()[i + net.output_size()];

					for (size_t i = 0; i < net.output_size(); ++i)
						net.input_array()[i + index] = net.output_array()[i];

					net.culcate();

					//outpot
					out << "999/" << std::setfill('0') << std::setw(2) << ri << "/" << std::setfill('0') << std::setw(2) << rn << "\t";
					write_value(net.output_array(), net.output_size(), csv, csv.value.size(), out);
				}
			}

			return true;
		}
	};
}