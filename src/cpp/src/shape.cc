#include "shape.hh"

#include <iostream>
#include <fstream>
#include <sstream>

#include <Eigen/Dense>
#include <Eigen/StdVector>

namespace TS {
Shape::Shape(char const* filename)
{
	std::ifstream f(filename);
	if (!f) {
		std::cerr << "File \""
			  << filename
			  << "\" could not be opened for reading"
			  << std::endl;
		exit(2);
	}

	read_block(f, size);
	read_block(f, vertices);
	read_block(f, edges);
	read_block(f, faces);

	f.close();
}

template <typename T>
void
Shape::read_block(std::ifstream& f, std::vector<Eigen::VectorX<T>>& vec)
{
	std::string line;
	while (std::getline(f, line)) {
		if (line.empty())
			break;

		std::string temp;
		std::vector<T> nums;
		std::istringstream iss(line);
		while (std::getline(iss, temp, ',')) {
			nums.push_back(std::stod(temp));
		}

		vec.push_back(Eigen::Map<Eigen::VectorX<T>>(nums.data(), nums.size()));
	}
}
}
