#include "shape.hh"

#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>

#include <Eigen/Dense>
#include <Eigen/StdVector>

#include <ncurses.h>

namespace TS {
/*
 * public methods
 */
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
		this->print_vertices();
	}

	if (this->b_print_edges) {
		this->print_edges();
	}

	wrefresh(this->win);
}

void
Shape::rotate(Eigen::Matrix3d rotation)
{
	for (auto &v: this->vertices) {
		v = (rotation * (v - this->center)) + this->center;
	}

	for (auto &f: this->faces) {
		f.calc_normal();
	}
}

void
Shape::scale(double scalar)
{
	for (auto &v: this->vertices) {
		v = ((v - this->center) * scalar) + this->center;
	}
}

void
Shape::translate(Eigen::Vector3d translation)
{
	for (auto &v: this->vertices) {
		v += translation;
	}

	this->center += translation;
}

/*
 * private methods
 */
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

std::tuple<t_pixel_print, Eigen::Vector3d>
Shape::movexy(Eigen::Vector3d v)
{
	int winx, winy;
	Eigen::Vector3d retv;
	getmaxyx(this->win, winy, winx);

	double fractionalx = ((v[0] * SCALE * winy) + (0.5 * winx));
	double fractionaly = (-(v[1] * SCALE * .5 * winy) + (0.5 * winy));

	double integralx, integraly;
	fractionalx = modf(fractionalx, &integralx);
	fractionaly = modf(fractionaly, &integraly);

	retv[0] = integralx;
	retv[1] = integraly;

	if (fractionaly >= 0.5) {
		return std::make_tuple(LOWER, retv);
	}

	return std::make_tuple(UPPER, retv);
}

void
Shape::print_vertices()
{
	size_t idx = 0;
	for (auto v: this->vertices) {
		Eigen::Vector3d u;

		std::tie(std::ignore, u) = movexy(v);
		mvwprintw(this->win, u[1], u[0], "%zu", idx);

		idx++;
	}
}

void
Shape::print_edges()
{
	int winx, winy;
	getmaxyx(this->win, winy, winx);

	this->fronts.clear();

	/* iterate over the edges */
	for (auto e: this->edges) {
		Eigen::Vector3d v = *e[1] - *e[0];
		double v_len = v.norm();
		Eigen::Vector3d u = v.normalized();

		/*
		 * prints points along the edge
		 *
		 * e_density is a natural number directly corresponding to the
		 * number of points printed along the edge
		 */
		for (auto k = 0; k <= this->e_density; ++k) {
			Eigen::Vector3d w = *e[0] + ((k / this->e_density) * v_len) * u;

			auto [tpp, p] = movexy(w);

			/* skip points that are off the screen */
			if (p[0] < 0 || p[0] > winx ||
			    p[1] < 0 || p[1] > winy) {
				continue;
			}

			/* TODO: clean this up */
			if (this->fronts.count(p)) {
				if (this->fronts[p] != tpp) {
					this->fronts[p] = FULL;
				}
			} else {
				this->fronts[p] = tpp;
			}
		}
	}

	for (auto val: this->fronts) {
		mvwprintw(this->win, val.first[1], val.first[0], "%c", val.second);
	}
}
}
