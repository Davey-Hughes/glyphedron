#ifndef SHAPE_HH
#define SHAPE_HH

#include <fstream>
#include <array>
#include <unordered_map>

#include <Eigen/Dense>
#include <Eigen/StdVector>
#include <ncurses.h>

#define SCALE 0.4

/*
 * Hash function for Eigen matrix and vector.
 * The code is from `hash_combine` function of the Boost library. See
 * http://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine .
 */
template<typename T>
struct matrix_hash : std::unary_function<T, size_t> {
	std::size_t operator()(T const& matrix) const {
		/*
		 * Note that it is oblivious to the storage order of Eigen
		 * matrix (column- or row-major). It will give you the same
		 * hash value for two different matrices if they are the
		 * transpose of each other in different storage order.
		 */
		size_t seed = 0;
		for (ssize_t i = 0; i < matrix.size(); ++i) {
			auto elem = *(matrix.data() + i);
			seed ^= std::hash<typename T::Scalar>()(elem) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}

		return seed;
	}
};

enum t_pixel_print {
	UPPER = '\'',
	LOWER = ',',
	FULL = ';'
};

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
	double e_density = 50.0;           /* number of points to draw along each edge */

	Eigen::Vector3d center = {0, 0, 0}; /* center of the shape */

	std::vector<Eigen::Vector3d> vertices; /* vector of vertices */
	std::vector<edge> edges;               /* vector of edges */
	std::vector<face> faces;               /* vector of faces */

	std::string fname;     /* file name of the shape coordinates */
	WINDOW *win = nullptr; /* pointer to ncurses screen */

	Eigen::Vector3d cop = {0, 0, 10000}; /* center of projection */

	bool b_print_vertices = false; /* bool whether or not to print vertices */
	bool b_print_edges = true;     /* bool whether or not to print edges */

	/* maps for characters to print on the front and behind of the shape */
	std::unordered_map<Eigen::Vector3d, t_pixel_print, matrix_hash<Eigen::Vector3d>> fronts;
	std::unordered_map<Eigen::Vector3d, t_pixel_print, matrix_hash<Eigen::Vector3d>> behinds;

	/* initialize the shape object from file */
	void init(std::string fname);

	/* helper function to read part of a shape's coordinate file into the shape's vectors */
	template <typename T>
	std::vector<std::vector<T>> read_block(std::ifstream& f);

	/* translate x, y coordinates to ncurses coordinate */
	std::tuple<t_pixel_print, Eigen::Vector3d> movexy(Eigen::Vector3d);

	/* print vertices with their indices */
	void print_vertices();

	/* print edges */
	void print_edges();
};
}

#endif /* SHAPE_HH */
