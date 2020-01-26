#include <JsonParse.h>
#include <fstream>
#include <iostream>
#include <time.h>

void main()
{
	std::ifstream _if("map_0_0.json");
	std::stringstream file_buffer;
	file_buffer << _if.rdbuf();
	std::string json_str(file_buffer.str());

	clock_t begin = clock();
	for (int i = 0; i < 1000000; ++i) {
		std::any o;
		Fossilizid::JsonParse::unpacker(o, json_str);
		(*std::any_cast<Fossilizid::JsonParse::JsonTable>(o))["v"] = i;
		json_str = Fossilizid::JsonParse::packer(o);
	}
	std::cout << "JsonParse time:" << clock() - begin << std::endl;

	return;
}