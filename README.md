# Terminal Shape Renderer

![alt text](https://github.com/Davey-Hughes/term-shapes/blob/master/images/dodecahedron.png "dodecahedron")

## Operation
Without any arguments, the program can be compiled with the makefile and run like so:
```
> make
> ./term-shapes
```

By default, the shape rendered is a cube, but currently any object given by the
coordinates of the vertices can be rendered.

The keyboard inputs are:
- q - quits the program
- r - resets the shape
- f,g,h,j,k,l - translates the shape
- t,y,u,i,o,p - rotates the shape
- -,= - enlarge or ensmallen the shape
- 9,0 - increase the density of the points drawn to represent the edges
- 1 - toggle showing the vertices by index
- 2 - toggle calculating occlusion

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
Currently, the occlusion is just an approximation (described in more details in
the code). Real occlusion is somewhat more difficult, but could be introduced
in a later update. The occlusion approximation also cannot handle concave
shapes in any reasonable way, but without shading concave shapes probably won't
look particularly good anyway. However, wireframes of any 3D shape that can be
described by a set of vertices and edges can be rendered.
