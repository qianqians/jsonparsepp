#include <JsonParse.h>
#include <fstream>

void main()
{
	std::string json_str = "{ \
		\"center\":{ \
		\"ip\":\"127.0.0.1\", \
		\"port\" : 1234, \
		\"gm_ip\" : \"127.0.0.1\", \
		\"gm_port\" : 2234, \
		\"log_level\" : \"debug\", \
		\"log_file\" : \"center.txt\", \
		\"log_dir\" : \"./log/\" \
	}, \
	\"dbproxy\":{ \
		\"ip\":\"127.0.0.1\", \
		\"port\" : 1235, \
		\"db_ip\" : \"122.112.209.196\", \
		\"db_port\" : 27017, \
		\"log_level\" : \"debug\", \
		\"log_file\" : \"dbproxy.txt\", \
		\"log_dir\" : \"./log/\" \
	}, \
	\"gate\":{ \
		\"inside_ip\":\"127.0.0.1\",\
		\"inside_port\" : 1236,\
		\"outside_ip\" : \"0.0.0.0\",\
		\"outside_port\" : 3236,\
		\"udp_outside_ip\" : \"0.0.0.0\",\
		\"udp_outside_port\" : 3237,\
		\"heartbeats\" : true,\
		\"log_level\" : \"debug\",\
		\"log_file\" : \"gate.txt\",\
		\"log_dir\" : \"./log/\"\
	},\
	\"lobby\":{\
		\"hub_name\":\"lobby\",\
		\"ip\" : \"127.0.0.1\",\
		\"port\" : 1237,\
		\"log_level\" : \"debug\",\
		\"log_file\" : \"lobby.txt\",\
		\"log_dir\" : \"./log/\"\
	},\
	\"gm\":{\
		\"gm_ip\":\"127.0.0.1\",\
		\"gm_port\" : 2234,\
		\"log_level\" : \"debug\",\
		\"log_file\" : \"control.txt\",\
		\"log_dir\" : \"./log/\"\
	}}";

	auto fs = std::ifstream("D://workspace//abelkhan//abelkhan_cpp//tools//jsonparsepp//json_test//json_test//test.txt");
	std::stringstream buffer;
	buffer << fs.rdbuf();
	std::string buff(buffer.str());

	boost::any o;
	Fossilizid::JsonParse::unpacker(o, buff);
}