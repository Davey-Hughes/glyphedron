# Terminal Shape Renderer

![alt text](https://github.com/Davey-Hughes/term-shapes/blob/master/images/dodecahedron.png "dodecahedron")

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
> ./term-shapes
```

By default, the shape rendered is a cube, but currently any object given by the
coordinates of the vertices (and edges) can be rendered by:

```
> make
> ./term-shapes file
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

## Shape Input File
The first line of the file are two comma separated normal numbers describing
the number of vertices n and edges m of the shape.

The next n lines are three comma separated floats that describe the x, y, z
positions for each vertex.

The next m lines are two comma separated normal numbers describing the indices
of the two points (from the previous n lines) that makes up an edge.

Examples of the shape input file are in the shapes directory. It is possible to
input a file that describes 0 edges, but 0 must be specified in the first line.

## Comments
Occlusion for convex shapes has been added, as shown in the image below:

![alt text](https://github.com/Davey-Hughes/term-shapes/blob/master/images/dodecahedron_occlusion.png "dodecahedron_occlusion")
