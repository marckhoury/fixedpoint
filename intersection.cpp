#include "intersection.h"

#include "mathutil.h" 

bool edge_triangle_intersection(Edge* e, Face* f)
{
    double p[3];
    double d = -dot(f->n, f->v[0]->v);
    
    if(norm(e->dir) < EPSILON) {
        return false;
    }    
 
    if (dot(f->n, e->dir) < EPSILON) {
        return false;
    }

    double t = -(dot(f->n, e->v[0]->v) + d) / dot(f->n, e->dir);
    if (t < 0) {
        return false;
    } else if(!e->is_ray && t > dist(e->v[0]->v, e->v[1]->v)) {
        return false;
    }

    p[0] = e->v[0]->v[0] + t * e->dir[0];
    p[1] = e->v[0]->v[1] + t * e->dir[1];
    p[2] = e->v[0]->v[2] + t * e->dir[2];
   
    double e0[3] = {f->v[1]->v[0] - f->v[0]->v[0], f->v[1]->v[1] - f->v[0]->v[1], f->v[1]->v[2] - f->v[0]->v[2]}; //v1 - v0
    double e1[3] = {f->v[2]->v[0] - f->v[1]->v[0], f->v[2]->v[1] - f->v[1]->v[1], f->v[2]->v[2] - f->v[1]->v[2]}; //v2 - v1
    double e2[3] = {f->v[0]->v[0] - f->v[2]->v[0], f->v[0]->v[1] - f->v[2]->v[1], f->v[0]->v[2] - f->v[2]->v[2]}; //v0 - v2
    
    double vp0[3] = {p[0] - f->v[0]->v[0], p[1] - f->v[0]->v[1], p[2] - f->v[0]->v[2]}; //p - v0
    double vp1[3] = {p[0] - f->v[1]->v[0], p[1] - f->v[1]->v[1], p[2] - f->v[1]->v[2]}; //p - v1
    double vp2[3] = {p[0] - f->v[2]->v[0], p[1] - f->v[2]->v[1], p[2] - f->v[2]->v[2]}; //p - v2

    double u0[3] = {f->v[2]->v[0] - f->v[0]->v[0], f->v[2]->v[1] - f->v[0]->v[1], f->v[2]->v[2] - f->v[0]->v[2]}; //v2 - v0
        
    double c[3];
    
    cross_product(e0, u0, c);
    double denom = dot(f->n, c);
    
    cross_product(e1, vp1, c);
    double alpha = dot(f->n, c)/denom;
    
    cross_product(e2, vp2, c);
    double beta = dot(f->n, c)/denom;
    
    cross_product(e0, vp0, c);
    double gamma = dot(f->n, c)/denom;

    bool in_triangle = alpha + beta + gamma < 1.05 && alpha + beta + gamma > 0.95 && alpha >= 0 && beta >= 0 && gamma >= 0;
       
    if(in_triangle) {
        e->p[0] = p[0];
        e->p[1] = p[1];
        e->p[2] = p[2];
        e->intersected = true;
    }
    return in_triangle;
}
