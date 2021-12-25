#ifndef SHAPE_HH
#define SHAPE_HH

#include <fstream>
#include <array>

#include <Eigen/Core>
#include <Eigen/StdVector>

typedef std::array<Eigen::Vector3d*, 2> edge;

struct face {
	std::vector<Eigen::Vector3d*> vertices;
	Eigen::Vector3d normal;
};

namespace TS {
class Shape {
public:
	Shape(const char* fname);

private:
	std::vector<Eigen::VectorXi> size;          /* num vertices, edges, faces */
	unsigned int e_density = 50;                /* number of points to draw along each edge */

	Eigen::Vector3d center = {0, 0, 0}; /* center of the shape */

	std::vector<Eigen::Vector3d> vertices; /* vector of vertices */
	std::vector<edge> edges;               /* vector of edges */
	std::vector<face> faces;               /* vector of faces */

	const char* fname; /* file name of the shape coordinates */

	Eigen::Vector3d cop = {0, 0, 10000}; /* center of projection */

	/* helper function to read part of a shape's coordinate file into the shape's vectors */
	template <typename T>
	std::vector<std::vector<T>> read_block(std::ifstream& f);
};
}

#endif /* SHAPE_HH */
