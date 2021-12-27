/*
 *     Copyright (C) 2022  Davey Hughes
 *
 *     This program is free software: you can redistribute it and/or modify it
 *     under the terms of the GNU General Public License as published by the
 *     Free Software Foundation, either version 3 of the License, or (at your
 *     option) any later version.
 *
 *     This program is distributed in the hope that it will be useful, but
 *     WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *     General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License along
 *     with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <unordered_map>

#include <Eigen/Dense>

#include <ncurses.h>

#include "shape.hh"

void
loop(TS::Shape &s)
{
	/* start ncurses mode */
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	curs_set(0);

	s.set_win(stdscr);

	/* std::chrono::time_point<std::chrono::high_resolution_clock> op_start, op_end, p_start, p_end; */

	/* transformation variables */
	Eigen::Matrix3d rot;
	double theta = M_PI / 200;
	double dist = 0.1;
	double scale = 1.1;

	while (1) {
		/* op_end = std::chrono::high_resolution_clock::now(); */
		/* auto op_diff = std::chrono::duration_cast<std::chrono::microseconds>(op_end - op_start).count(); */

		/* wclear(stdscr); */
		/* mvwprintw(stdscr, 2, 1, "Operaton time: %05luµs", op_diff); */

		/* p_start = std::chrono::high_resolution_clock::now(); */
		s.print();
		/* p_end = std::chrono::high_resolution_clock::now(); */

		/* auto p_diff = std::chrono::duration_cast<std::chrono::microseconds>(p_end - p_start).count(); */
		/* mvwprintw(stdscr, 1, 1, "Print time: %05luµs", p_diff); */

		/* wrefresh(stdscr); */

		int c = getch();

		/* op_start = std::chrono::high_resolution_clock::now(); */

		switch(c) {

		/* quit */
		case 'q':
			endwin();
			return;


		/* ** ROTATIONS ** */
		/* rotate around z axis */
		case 't':
			rot = Eigen::AngleAxisd(theta, Eigen::Vector3d::UnitZ());
			s.rotate(rot);
			break;
		case 'y':
			rot = Eigen::AngleAxisd(-theta, Eigen::Vector3d::UnitZ());
			s.rotate(rot);
			break;

		/* rotate around z axis */
		case 'u':
			rot = Eigen::AngleAxisd(theta, Eigen::Vector3d::UnitX());
			s.rotate(rot);
			break;
		case 'i':
			rot = Eigen::AngleAxisd(-theta, Eigen::Vector3d::UnitX());
			s.rotate(rot);
			break;

		/* rotate around y axis */
		case 'o':
			rot = Eigen::AngleAxisd(theta, Eigen::Vector3d::UnitY());
			s.rotate(rot);
			break;
		case 'p':
			rot = Eigen::AngleAxisd(-theta, Eigen::Vector3d::UnitY());
			s.rotate(rot);
			break;

		/* ** SCALE ** */
		case '=':
			s.scale(scale);
			break;

		case '-':
			s.scale(1.0 / scale);
			break;


		/* ** TRANSLATIONS ** */
		/* translate along x axis */
		case 'h':
			s.translate({-dist, 0, 0});
			break;
		case 'l':
			s.translate({dist, 0, 0});
			break;

		/* translate along y axis */
		case 'j':
			s.translate({0, -dist, 0});
			break;
		case 'k':
			s.translate({0, dist, 0});
			break;

		/* translate along z axis */
		case 'f':
			s.translate({0, 0, -dist});
			break;
		case 'g':
			s.translate({0, 0, dist});
			break;


		/* flip printing of vertices */
		case '1':
			s.toggle_print_vertices();
			break;

		/* flip printing of edges */
		case '2':
			s.toggle_print_edges();
			break;


		/* ** CHANGE EDGE DENSITY ** */
		/* increase edge density */
		case '0':
			s.increase_e_density();
			break;

		/* decrease edge density */
		case '9':
			s.decrease_e_density();
			break;

		default:
			continue;
		}
	}
}

int
main(int argc, char **argv)
{

	auto s = TS::Shape(argc, argv);
	loop(s);

	return 0;
}
