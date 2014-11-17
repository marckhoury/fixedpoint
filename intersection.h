#ifndef INTERSECTION_H
#define INTERSECTION_H

#include "mesh.h"

void compute_intersection(Edge* e, Face* f);
bool edge_triangle_intersection(Edge* e, Face* f);

#endif
