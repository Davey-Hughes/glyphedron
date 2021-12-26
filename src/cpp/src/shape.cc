#include "shape.hh"

#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>

#include <Eigen/Dense>
#include <Eigen/StdVector>

#include <ncurses.h>

namespace TS {
Shape::Shape(int argc, char **argv)
{
	if (argc > 1) {
		init(argv[1]);
	} else {
		init("./shapes/platonic_solids/cube.txt");
	}
}

Shape::Shape(std::string fname)
{
	init(fname);
}

void
Shape::set_win(WINDOW *win)
{
	this->win = win;
}

void
Shape::toggle_print_vertices()
{
	this->b_print_vertices = !this->b_print_vertices;
}

void
Shape::toggle_print_edges()
{
	this->b_print_edges = !this->b_print_edges;
}

void
Shape::increase_e_density()
{
	this->e_density++;
}

void
Shape::decrease_e_density()
{
	if (this->e_density > 0) {
		this->e_density--;
	}
}

void
Shape::print()
{
	wclear(this->win);

	if (this->b_print_vertices) {
		print_vertices();
	}

	wrefresh(this->win);
}

void
Shape::rotate(Eigen::Matrix3d rotation)
{
	for (auto &v: this->vertices) {
		v = rotation * v;
	}

	for (auto &f: this->faces) {
		f.calc_normal();
	}
}

void
Shape::scale(double scalar)
{
	for (auto &v: this->vertices) {
		v *= scalar;
	}
}

void
Shape::translate(Eigen::Vector3d translation)
{
	for (auto &v: this->vertices) {
		v += translation;
	}
}

void
Shape::init(std::string fname)
{
	std::ifstream f(fname);
	if (!f) {
		std::cerr << "File \""
			  << fname
			  << "\" could not be opened for reading"
			  << std::endl;
		exit(2);
	}

	/* read over shape size descriptor (for C implementation) */
	read_block<int>(f);

	/* read list of vertices */
	std::vector<std::vector<double>> points = read_block<double>(f);
	for (auto v: points) {
		this->vertices.push_back(Eigen::Map<Eigen::Vector3d>(v.data()));
	}

	/* read list of edges */
	std::vector<std::vector<size_t>> edges = read_block<size_t>(f);
	for (auto e: edges) {
		if (e.size() != 2) {
			std::cerr << "Edges must contain exactly 2 indices" << std::endl;
			exit(3);
		}

		edge cur_edge = {
			&(this->vertices[e[0]]),
			&(this->vertices[e[1]])
		};

		this->edges.push_back(cur_edge);
	}

	/* read list of faces */
	std::vector<std::vector<size_t>> faces = read_block<size_t>(f);
	for (auto face: faces) {
		struct face cur_face;

		for (auto p: face) {
			cur_face.vertices.push_back(&(this->vertices[p]));
		}

		cur_face.calc_normal();

		this->faces.push_back(cur_face);
	}

	f.close();

	this->fname = fname;

	(void) this->e_density;
}

template <typename T>
std::vector<std::vector<T>>
Shape::read_block(std::ifstream& f)
{
	std::vector<std::vector<T>> vec;

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

		vec.push_back(nums);
	}

	return vec;
}

void
Shape::movexy(double &x, double &y)
{
	int winx, winy;
	getmaxyx(this->win, winy, winx);

	double fractionalx = ((x * SCALE * winy) + (0.5 * winx));
	double fractionaly = (-(y * SCALE * .5 * winy) + (0.5 * winy));

	double integralx, integraly;
	fractionalx = modf(fractionalx, &integralx);
	fractionaly = modf(fractionaly, &integraly);

	x = integralx;
	y = integraly;
}

void
Shape::print_vertices()
{
	size_t idx = 0;
	for (auto v: this->vertices) {
		double x = v[0];
		double y = v[1];

		movexy(x, y);
		mvwprintw(this->win, y, x, "%zu", idx);

		idx++;
	}
}
}
