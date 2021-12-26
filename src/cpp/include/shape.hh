#ifndef SHAPE_HH
#define SHAPE_HH

#include <fstream>
#include <array>

#include <Eigen/Dense>
#include <Eigen/StdVector>
#include <ncurses.h>

#define SCALE 0.4

typedef std::array<Eigen::Vector3d*, 2> edge;

struct face {
	std::vector<Eigen::Vector3d*> vertices;
	Eigen::Vector3d normal;

	void calc_normal() {
		normal = (*vertices[0] - *vertices[1]).cross(*vertices[0] - *vertices[2]);
	}
};

namespace TS {
class Shape {
public:
	/* constructors */
	Shape(std::string fname);
	Shape(int argc, char **argv);

	void set_win(WINDOW *win);
	void toggle_print_vertices();
	void toggle_print_edges();
	void increase_e_density();
	void decrease_e_density();

	void print();
	void rotate(Eigen::Matrix3d rotation);
	void scale(double scalar);
	void translate(Eigen::Vector3d translation);

private:
	std::vector<Eigen::VectorXi> size; /* num vertices, edges, faces */
	unsigned int e_density = 50;       /* number of points to draw along each edge */

	Eigen::Vector3d center = {0, 0, 0}; /* center of the shape */

	std::vector<Eigen::Vector3d> vertices; /* vector of vertices */
	std::vector<edge> edges;               /* vector of edges */
	std::vector<face> faces;               /* vector of faces */

	std::string fname; /* file name of the shape coordinates */
	WINDOW *win = nullptr;

	Eigen::Vector3d cop = {0, 0, 10000}; /* center of projection */

	bool b_print_vertices = false; /* bool whether or not to print vertices */
	bool b_print_edges = true;     /* bool whether or not to print edges */

	/* initialize the shape object from file */
	void init(std::string fname);

	/* helper function to read part of a shape's coordinate file into the shape's vectors */
	template <typename T>
	std::vector<std::vector<T>> read_block(std::ifstream& f);

	void movexy(double &x, double &y);
	void print_vertices();
};
}

#endif /* SHAPE_HH */
