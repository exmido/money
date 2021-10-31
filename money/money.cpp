#include "money.h"
using namespace qua;
using namespace stock;

//main
int main(int argc, char** argv)
{
	string name(1 < argc ? argv[1] : "");
	if (name == "qua")
		return qua_main(argc, argv);

	return stock_main(argc, argv);
}
