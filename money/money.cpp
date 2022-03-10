#include "money.h"

//loader
int32_t load(stock::stock_csv& csv, std::string name, size_t filter, size_t training, std::ostream& out)
{
	if (0 == twse::stock_http::load(csv, name, filter, training, out))
		return 0;

	if (0 == yahoo::stock_http::load(csv, name, filter, training, out))
		return 0;

	if (0 == wearn::stock_http::load(csv, name, filter, training, out))
		return 0;

	return __LINE__;
}

//main
int main(int argc, char** argv)
{
#ifdef  WIN32
	SetConsoleOutputCP(CP_UTF8);
#endif

	std::string name(1 < argc ? argv[1] : "");
	if (name == "qua")
		return wearn::qua_main(argc, argv, load);

	return stock::stock_main(argc, argv, load);
}
