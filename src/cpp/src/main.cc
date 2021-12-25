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
/* #include <chrono> */
/* #include <vector> */

/* #include <Eigen/Dense> */

/* #define VEC_SIZE 100000 */

#include "shape.hh"

/* void */
/* rotate(std::vector<Eigen::Vector3d>& vecs, Eigen::Quaterniond rot) */
/* { */
	/* auto rot_matrix = rot.toRotationMatrix(); */

	/* for (auto &v: vecs) { */
		/* v = rot_matrix * v; */
	/* } */
/* } */

int
main(int argc, char** argv)
{
	if (argc < 2) {
		std::cerr << "File must be specified" << std::endl;
		exit(1);
	}

	auto s = TS::Shape(argv[1]);

	/* std::vector<Eigen::Vector3d> vecs(VEC_SIZE); */

	/* auto f = []() -> Eigen::Vector3d { */
		/* return Eigen::Vector3d::Random(); */
	/* }; */

	/* std::generate(vecs.begin(), vecs.end(), f); */

	/* Eigen::Quaterniond r; */
	/* r = Eigen::AngleAxisd(M_PI_4, Eigen::Vector3d::UnitX()); */

	/* auto start = std::chrono::high_resolution_clock::now(); */
	/* rotate(vecs, r); */
	/* auto stop = std::chrono::high_resolution_clock::now(); */
	/* auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start); */
	/* std::cout << duration.count() << std::endl; */

	return 0;
}
