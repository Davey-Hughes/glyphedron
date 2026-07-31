# Occlusion: how it works, where it breaks, and what a full fix looks like

This document exists because the small stellated dodecahedron renders with more
visible line than it looks like it should under `convex` occlusion. That turned
out not to be the defect it appears to be, and the investigation surfaced a
different one. Both are recorded here, along with what a real fix would involve.

## Summary

- Measured against an independent reference, the current method is **exact on
  the cube and icosahedron** and **2% off on the small stellated dodecahedron**,
  where every one of those errors hides a point that should be drawn. It never
  fails to hide something that should be hidden.
- So the busy look of the star is not missed occlusion. It is a 30 edge
  non-convex solid of which roughly 40% of edge length is genuinely visible,
  sampled at 50 points per edge across a large terminal.
- The real weakness is elsewhere: **rays that pass exactly through a vertex or
  along an edge**. Every remaining error traces back to one. No tolerance fixes
  this class robustly, which is the strongest argument for changing approach
  rather than patching.
- A full fix means replacing ray casting with a **depth buffer**. That is
  correct by construction for any shape, removes three separate assumptions, and
  is faster than what runs today.

## How occlusion works today

`occlude_point_convex()` in `src/c/src/convex_occlusion.c` answers one question:
is this 3D point hidden from the eye?

For each face of the shape:

1. Skip the face if the point's edge is one of its edges.
2. Intersect the ray from the point to the centre of projection with the face's
   plane, giving a parameter `t`.
3. Reject unless `0 < t < 1`, so the intersection lies strictly between.
4. Ask whether the intersection lies inside the face polygon, by projecting onto
   the coordinate plane the face's normal is most aligned with and applying the
   crossing number rule.
5. If it does, and the intersection is nearer the eye, the point is hidden.

`print_edges()` in `src/c/src/print.c` adds a shortcut: if an edge's two
endpoints and its midpoint are all visible, it marks the whole edge visible and
skips the per point test for every sample along it.

## The three assumptions

The method is named for convexity and states it in its own comment. Three
distinct assumptions hide inside that word.

**One: the solid is convex.** Step 5 treats "a face is in front of me" as "I am
hidden". For a convex solid that holds. The small stellated dodecahedron is not
convex — all 12 of its faces have vertices on both sides of their own plane.

**Two: faces are simple polygons.** The crossing number rule computes even-odd
fill, which is only equivalent to "inside the face" for a non self-intersecting
polygon. Every face of the small stellated dodecahedron is a pentagram, and the
rule therefore classifies the centre of each face as *outside* that face: 0 of
12 faces contain their own centre, where the cube scores 6 of 6.

**Three: three samples decide an edge.** The shortcut assumes that if both
endpoints and the midpoint of an edge are visible, all of it is. For a convex
polyhedron an edge's interior has constant visibility, so this is sound. For a
non-convex one it is not.

## What was actually measured

The reference is independent of the renderer: the small stellated dodecahedron's
surface was reconstructed as 60 triangles, by intersecting each pentagram face's
own edges to recover the inner pentagon and taking the five spikes of each face.
Rays were then cast against those triangles with the Möller–Trumbore test. For
the convex solids the faces are already the surface, so a triangle fan is valid.

Sampling 41 points along every edge:

| Shape | points | wrong | note |
|---|---|---|---|
| cube | 492 | 0 | exact |
| icosahedron | 1230 | 0 | exact |
| dodecahedron | 1230 | 3 | |
| small stellated dodecahedron | 1230 | 24 | all over-occlusion, none missed |
| tetrahedron | 246 | 39 | 1 with the edge shortcut disabled |
| octahedron | 492 | 82 | 49 with the edge shortcut disabled |

Three things follow.

**The star is not the broken case.** All 24 of its errors hide a point that
should have been drawn. Not one point is drawn that should have been hidden. Whatever
makes the render look busy, it is not the occlusion failing to hide things.

**Assumptions one and two cost nothing measurable.** Switching the fill rule
from even-odd to nonzero winding is the textbook remedy for star polygons, and
it does change the classification of individual intersections — 1315 of 10886
ray/face intersections on this shape land where the two rules disagree, 12.1%.
It changes the rendered result by exactly zero points, because those
intersections lie in the central pentagon of a face, which is interior to the
solid and already blocked by another face. This was verified twice, by
occlusion counts (393 versus 393) and by full frame renders.

**The shapes that suffer are the simplest convex ones.** The tetrahedron and
octahedron carry the largest error counts, and the edge shortcut accounts for
most of it: 38 of the tetrahedron's 39, and 33 of the octahedron's 82.

### A caveat on those numbers

The residual errors that the shortcut does not explain — the octahedron's 49 and
the dodecahedron's 3 — are all boundary cases where the ray passes exactly
through a vertex of the blocking face or exactly along one of its edges. The
octahedron is the extreme: its rear pole sits precisely behind its front apex, so
the ray strikes a vertex shared by all four front faces.

At such a point "inside the polygon" has no answer that is both correct and
stable. The reference has the same difficulty, so a portion of these counts is
disagreement between two implementations at a genuinely undefined position
rather than a defect in either. Treat the exact figures as indicative. The
robust reading is that cube and icosahedron are exact, the star over-occludes
slightly, and the low face count solids are dominated by degeneracies.

## Why patching does not pay

The obvious repairs each address an assumption that costs nothing:

- **Nonzero winding fill** fixes assumption two and changes no rendered point.
- **Detecting non-convexity at load** and disabling the shortcut for such shapes
  fixes assumption three where it matters least. The shortcut's measurable cost
  is on the tetrahedron and octahedron, both convex, so a convexity test would
  not disable it there.
- **Dropping the shortcut unconditionally** is the one change with a real
  payoff: the tetrahedron falls from 39 errors to 1, the octahedron from 82 to
  49. It costs the per point occlusion test on every sample of every edge, which
  is the expense the shortcut exists to avoid.

None of them touch the degeneracy class, which is what actually limits accuracy
once the shortcut is gone. Adding a tolerance does not resolve it either; that
was measured earlier when a boundary-as-inside variant was tried and made
results worse on the non-convex shape.

## What a full fix looks like

### Option A: fix the assumptions in place

Drop the edge shortcut, switch to nonzero winding, and replace the "skip faces
sharing this edge" rule with a distance based self-intersection guard.

Leaves the degeneracy class untouched, and makes the renderer slower in exactly
the case the shortcut was protecting. Not recommended on its own.

### Option B: depth buffer

Allocate a per cell depth value the size of the terminal. Rasterise every face
into it, writing depth only and no glyph. Then draw edge points, keeping a point
only when its depth is nearer than what the buffer holds, with a small bias so a
point does not occlude itself.

This removes every assumption above at once:

- Convexity is irrelevant; nothing is inferred from face orientation.
- Self-intersecting faces are handled by rasterising them, with the fill rule
  becoming a rasteriser detail rather than an occlusion decision.
- No edge shortcut is needed, because the per point cost becomes one buffer
  comparison.
- Degeneracies stop mattering: a ray no longer has to decide whether it passed
  through a vertex, since coverage is decided per cell at rasterisation time.

It is also faster. The current method is `O(points × faces)` with a plane
intersection and a polygon walk inside the inner loop. A depth buffer is
`O(total face area in cells)` once, then `O(1)` per point.

The costs are real but contained. Faces must be triangulated before
rasterisation, which for a pentagram means recovering the inner intersection
points, the same construction used to build the reference for this document.
Depth precision at the sub-cell level needs thought, since the renderer already
treats a cell as two half height pixels. And the buffer must be reallocated on
terminal resize.

### Option C: triangulate at load, keep ray casting

Decompose every face into triangles when the shape is read, then keep the
existing ray cast. Point in polygon on a triangle is unambiguous, which removes
assumption two properly rather than incidentally.

Simpler than B and a genuine improvement, but it keeps the `O(points × faces)`
cost, keeps the degeneracy problem, and increases the face count fivefold or
more, making the inner loop slower.

## Recommendation

Option B. It is the only one that removes the degeneracy class, it is the only
one that makes the renderer faster rather than slower, and it subsumes the other
two rather than competing with them.

If the appetite is smaller, the highest value single change is deleting the edge
shortcut in `print_edges()` and accepting the cost, since it is responsible for
the largest measured error on the two shapes that fare worst. That is a few
lines and is worth doing under a test either way.

Explicitly not worth doing on its own: changing the fill rule. It is more
principled and it is measurably inert.

## Reproducing the measurements

Every figure above came from Python replicas of the shipped routines run against
the files in `shapes/`. Nothing in the C source was modified to obtain them. The
approach, if it needs repeating:

1. Parse a shape file. Note the face lines use both commas and spaces as
   separators, and at least one line in `icosahedron.txt` is missing a comma,
   which `strtok` in `init_from_file` tolerates.
2. For the reference, build triangles. Convex solids fan from the first vertex.
   For a pentagram, intersect each pair of non-adjacent edges of the face to
   recover the inner pentagon, then take the five spikes.
3. Cast rays from sampled edge points toward `{0, 0, 10000}`, the centre of
   projection defined by `COP` in `glyphedron.h`.
4. Compare against a replica of `occlude_point_convex`, including the
   edge-sharing skip and the `0 < t < 1` test.

## Related

- The shape files give phi to five decimals, so faces are not exactly planar. The
  small stellated dodecahedron additionally carried a mistyped digit, corrected
  separately. The residual deviation is around 3e-06 and has no bearing on any
  of the above; it is five orders of magnitude below the effects discussed.
- `occlude_point_approx` is a different method entirely, thresholding the angle
  between the point-to-centre and point-to-eye vectors. It is an approximation by
  construction and is not analysed here.
