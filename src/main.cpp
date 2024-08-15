#include "WebServ.hpp"

int main(int argc, char **argv){
	try {
		WebServ server(argc, argv);
		server.run();
	} catch (Config::ParseException &e) {
		std::cout << "Parse exception: " << e.what() << std::endl;
	} catch (WebServ::InitException &e) {
		std::cout << "InitException: " << e.what() << std::endl;
	} catch (std::exception &e) {
		std::cout << "Fuck#2 " << e.what() << std::endl;
	}
}