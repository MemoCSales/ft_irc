# include "Utils.hpp"

std::string Utils::trim(const std::string& str) {
	size_t first = str.find_first_not_of(" \t\n\r");
	size_t last = str.find_last_not_of(" \t\n\r");
	if (first == std::string::npos || last == std::string::npos) {
		return "";
	} else {
		return str.substr(first, last - first + 1);
	}
}

void Utils::printAsciiDecimal(const std::string& str) {
	for (size_t i = 0; i < strlen(str.c_str()); i++)
	{
		printf("%d " , (int)str[i]);
	}
	printf("\n");
	
}
