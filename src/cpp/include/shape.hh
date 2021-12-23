#ifndef SHAPE_HH
#define SHAPE_HH

#include <fstream>

#include <Eigen/Dense>
#include <Eigen/StdVector>

namespace TS {
class Shape {
public:
	Shape(char const* filename);

private:
	std::vector<Eigen::VectorXi> size;
	std::vector<Eigen::VectorXd> vertices;
	std::vector<Eigen::VectorXi> edges;
	std::vector<Eigen::VectorXi> faces;

	template <typename T>
	void read_block(std::ifstream& f, std::vector<Eigen::VectorX<T>>& vec);
};
}

#endif /* SHAPE_HH */
