# Glyphedron

Polyhedra drawn in terminal glyphs.

![dodecahedron](images/dodecahedron.png "dodecahedron")

## Installation
The only thing that needs to be installed to compile this program is ncurses.

### MacOS Homebrew
```
brew install ncurses
```

### Ubuntu
```
sudo apt install libncurses5-dev
```

## Operation
Without any arguments, the program can be compiled with the makefile and run like so:
```
> make
> ./glyphedron
```

By default, the shape rendered is a cube, but currently any object given by the
coordinates of the vertices (and edges) can be rendered by:

```
> make
> ./glyphedron file
```

### Keyboard Inputs
- q - quits the program
- r - resets the shape
- f,g,h,j,k,l - translates the shape
- t,y,u,i,o,p - rotates the shape
- -,= - enlarge or ensmallen the shape
- 9,0 - increase the density of the points drawn to represent the edges
- 1 - toggle showing the vertices by index
- 2 - toggle printing edges
- 3 - toggle calculating occlusion (iterates through occlusion options)
- a - autorotate (then 'a' again to stop, or 'm' first to enter angles by hand)

## Tests
```
> make test
> make test-asan
```

`make test` runs the suite against a plain build. `make test-asan` rebuilds it
under AddressSanitizer and UBSan into a separate object tree and runs the same
suite.

Run both. They are not interchangeable: the coverage for `init_from_file`'s
cleanup path detects a bad free, which a plain build usually performs in
silence. Only the sanitizer build fails on it.

The tests link just the modules that need neither ncurses nor `main()`, so they
need no terminal. Run them from the repository root, since the fixtures under
`tests/fixtures` are opened by relative path.

## Shape Input File
The first line of the file is three comma separated whole numbers describing
the number of vertices n, edges m, and faces k of the shape.

The next n lines are three comma separated floats that describe the x, y, z
positions for each vertex.

The next m lines are two comma separated whole numbers describing the indices
of the two points (from the previous n lines) that make up an edge.

The next k lines are comma separated whole numbers giving the indices of the
points that make up a face. The points must go around the face in order, along
its edges, so that points across the face are not connected. Faces are what the
convex occlusion mode tests against; a shape with no faces can still be drawn,
but nothing will be occluded.

Examples of the shape input file are in the shapes directory. It is possible to
input a file that describes 0 edges or 0 faces, but 0 must be specified in the
first line.
