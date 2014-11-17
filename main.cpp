#include <iostream>
#include <cstdlib>
#include <algorithm>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include "io.h"
#include "mesh.h"
#include "mathutil.h"
#include "glcontext.h"
#include "intersection.h"
#include "pyramid.c"

using namespace std;


Context ctx;
Mesh mesh;
Mesh vor;
Mesh del;

vector<Mesh> meshes;
int curr_mesh = 0;

GLUquadricObj *quadric = gluNewQuadric();
bool draw_intersection = false;
bool draw_voronoi = false;

void clear_intersection()
{
    for(size_t i = 0; i < vor.edge_count(); i++) {
        Edge* e = vor.get_edge(i);
        e->intersected = false;
    }
}

void create_mesh()
{
    Mesh next;
    for(size_t i = 0; i < vor.edge_count(); i++) {
        Edge* e = vor.get_edge(i);
        if(e->intersected) {
            Face* f = e->dual;
            for(size_t j = 0; j < f->v.size(); j++) {
                Vertex* v = f->v[j];
                next.add(v);
            }
            next.add(f);
        }
    }
    compute_normals(next);
    meshes.push_back(next);
    curr_mesh = meshes.size()-1; 
}

void intersection()
{
    for(size_t i = 0; i < vor.edge_count(); i++) {
        Edge* e = vor.get_edge(i);
        for(size_t j = 0; j < meshes[curr_mesh].face_count(); j++) {
            Face* f = meshes[curr_mesh].get_face(j);
            if(!e->intersected) {
                bool hit = edge_triangle_intersection(e,f);
                if(!hit) {
                    f->n[0] *= -1, f->n[1] *= -1, f->n[2] *= -1;
                    edge_triangle_intersection(e,f);
                    f->n[0] *= -1, f->n[1] *= -1, f->n[2] *= -1;
                }
            }
        }
    }
}

void draw_point(double* v)
{
    glPushMatrix();
    glTranslatef(v[0], v[1], v[2]);
    gluSphere(quadric, 0.02, 10, 10);
    glPopMatrix();
}

void display()
{
    float mat_diffuse[4] = {0,1,0,1};
    float mat_ambient[4] = {0,0,0,1};
    float mat_specular[4] = {0,0,0,1};
    float mat_shininess = 1;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, ctx.wireframe ? GL_LINE : GL_FILL);
    
    glEnable(GL_LIGHT0);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ctx.global_ambient);
    glLightfv(GL_LIGHT0, GL_POSITION, ctx.light_pos);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &mat_shininess);

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0, 0, -4);
    glMultMatrixf(ctx.mat);
    
    for(int i = 0; i < meshes[curr_mesh].face_count(); i++) {
        Face* f = meshes[curr_mesh].get_face(i);
        glBegin(GL_POLYGON);
        for(int j = 0; j < f->v.size(); j++) {
            Vertex* v = f->v[j];
            glNormal3dv(v->n);
            glTexCoord2dv(v->t);
            glVertex3dv(v->v);  
        } 
        glEnd();
    } 

    //Draw Voronoi edges
    if(draw_voronoi) {
        glDisable(GL_LIGHTING);
        for(size_t i = 0; i < vor.edge_count(); i++) {
            Edge* e = vor.get_edge(i);
            if (e->intersected) {
                glColor3f(1,0,0);
            } else {
                glColor3f(1,1,1);
            }
            glBegin(GL_LINES);
            glVertex3dv(e->v[0]->v);
            glVertex3dv(e->v[1]->v);
            glEnd();    
        }
        glEnable(GL_LIGHTING);
    }

    if(draw_intersection) {
        //Draw intersection points
        mat_diffuse[0] = 1, mat_diffuse[1] = 0;
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
        for(size_t i = 0; i < vor.edge_count(); i++) {
            Edge* e = vor.get_edge(i);
            if(e->intersected) {
                draw_point(e->p);
            }
        mat_diffuse[0] = 1, mat_diffuse[1] = 0;
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
        }
    }
    glPopMatrix();
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    ctx.window[0] = -static_cast<double>(w)/h;
    ctx.window[1] = -ctx.window[0];
    ctx.window[2] = 1.0;
    ctx.window[3] = -1.0;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(ctx.fovy, static_cast<double>(w)/h, ctx.znear, ctx.zfar);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
        case 'i':
            intersection();
        break;
        case 'p':
            draw_intersection = !draw_intersection;
        break;
        case 'v':
            draw_voronoi = !draw_voronoi;
        break;
        case 'n':
            clear_intersection();
            intersection();
            create_mesh();
            cout << "Face count: " <<  meshes[curr_mesh].face_count() << endl;
        break;  
        case 'w':
            ctx.wireframe = !ctx.wireframe;
        break;
        case 27: //ESC key
            exit(0);
        break;
        default:
            cout << "Unassigned character: " << key << endl;
        break;
    }
}

void menu()
{
    cout << "\033[1;31m   Fixed Point Menu\033[0m" << endl;
    cout << "\033[1;31mOption                                    Key\033[0m" << endl;
    cout << "Compute Intersections                      i" << endl;
    cout << "Draw Intersections                         p" << endl;
    cout << "Draw Voronoi Edges                         v" << endl;
    cout << "Iterate RDT Operator                       n" << endl;
    cout << "Toggle Wireframe                           w" << endl;
    cout << "Quit                                       ESC" << endl;
}

void world_coords(int x, int y, double* p)
{
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    p[0] = static_cast<double>(x-viewport[0])/viewport[2];
    p[1] = static_cast<double>(y-viewport[1])/viewport[3];

    p[0] = ctx.window[0] + p[0]*(ctx.window[1]-ctx.window[0]);
    p[1] = ctx.window[2] + p[1]*(ctx.window[3]-ctx.window[2]);
    p[2] = ctx.znear;
}

void mouse(int button, int state, int x, int y)
{
    int cursor = GLUT_CURSOR_RIGHT_ARROW;
    if(state == GLUT_DOWN) {
        if(button == GLUT_LEFT_BUTTON) {
            cursor = GLUT_CURSOR_CYCLE;
            ctx.mouse_button[0] = true;
        } else if(button == GLUT_MIDDLE_BUTTON) {
            cursor = GLUT_CURSOR_CROSSHAIR;
            ctx.mouse_button[1] = true;
        } else if(button == GLUT_RIGHT_BUTTON) {
            cursor = GLUT_CURSOR_UP_DOWN;
            ctx.mouse_button[2] = true;
        }
    } else {
        ctx.mouse_button[0] = ctx.mouse_button[1] = ctx.mouse_button[2] = false;
    }
    glutSetCursor(cursor);
    ctx.mouse_pos[0] = x;
    ctx.mouse_pos[1] = y;
    world_coords(x, y, ctx.drag);
}

void motion(int x, int y)
{
    int dx = x - ctx.mouse_pos[0];
    int dy = y - ctx.mouse_pos[1];
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    if(dx == 0 && dy == 0) {
        return;
    } else if(ctx.mouse_button[0]) {
        double angle = sqrt(dy*dy + dx*dx)/static_cast<double>(viewport[2]+1)*180.0;
        double rx = ctx.matinv[0]*dy + ctx.matinv[4]*dx;
        double ry = ctx.matinv[1]*dy + ctx.matinv[5]*dx;
        double rz = ctx.matinv[2]*dy + ctx.matinv[6]*dx;
        glRotatef(angle, rx, ry, rz);
    } else if(ctx.mouse_button[1]) {
        double p[3];
        world_coords(x, y, p);
        glLoadIdentity();
        glTranslatef(p[0] - ctx.drag[0], p[1] - ctx.drag[1], p[2] - ctx.drag[2]);
        glMultMatrixf(ctx.mat);
        ctx.drag[0] = p[0], ctx.drag[1] = p[1], ctx.drag[2] = p[2];
    } else if(ctx.mouse_button[2]) {
        glLoadIdentity();
        glTranslatef(0, 0, dy*0.01);
        glMultMatrixf(ctx.mat);
    }

    ctx.mouse_pos[0] = x;
    ctx.mouse_pos[1] = y;
    
    glGetFloatv(GL_MODELVIEW_MATRIX, ctx.mat);
    inverse(ctx.mat, ctx.matinv);
    glutPostRedisplay();
}

void idle()
{
    glutPostRedisplay();
}

void convert(tetrahedralizeio* out, tetrahedralizeio* vorout)
{
    for(size_t i = 0; i < vorout->numberofpoints; i++) {
        Vertex* v = new Vertex;
        v->v[0] = vorout->pointlist[3*i];
        v->v[1] = vorout->pointlist[3*i+1];
        v->v[2] = vorout->pointlist[3*i+2];
        v->has_normal = v->has_texture = false;
        vor.add(v);
    }
    for(size_t i = 0; i < vorout->numberofedges; i++) {
        Edge* e = new Edge;
        long vi = vorout->edgelist[2*i];
        long ui = vorout->edgelist[2*i+1];
        Vertex *v, *u;    
        
        if(vi == -1) {
            long tmp = vi;
            vi = ui;
            ui = tmp;
        }
         
        if (ui == -1) {
            double t = 10;
            v = vor.get_vertex(vi);
            e->v[0] = v;
            e->dir[0] = vorout->normlist[3*i];
            e->dir[1] = vorout->normlist[3*i+1];
            e->dir[2] = vorout->normlist[3*i+2];
            normalize(e->dir);
            
            u = new Vertex;
            u->v[0] = v->v[0] + t * e->dir[0];
            u->v[1] = v->v[1] + t * e->dir[1];
            u->v[2] = v->v[2] + t * e->dir[2];
            u->index = mesh.vertex_count();
            u->has_normal = u->has_texture = false;
            vor.add(u);

            e->v[1] = u;
            e->is_ray = true;
        } else {
            v = vor.get_vertex(vi);
            u = vor.get_vertex(ui);
            e->v[0] = v;
            e->v[1] = u;
            e->dir[0] = u->v[0] - v->v[0];
            e->dir[1] = u->v[1] - v->v[1];
            e->dir[2] = u->v[2] - v->v[2];

            if(norm(e->dir) > EPSILON) {
                normalize(e->dir);
            }
            e->is_ray = false;
        }
        e->intersected = false;
        vor.add(e);
    }
    
    for(size_t i = 0; i < out->numberofpoints; i++) {
        Vertex* v = new Vertex;
        v->v[0] = out->pointlist[3*i];
        v->v[1] = out->pointlist[3*i+1];
        v->v[2] = out->pointlist[3*i+2];
        v->has_normal = v->has_texture = false;
        del.add(v);
    }
    for(size_t i = 0; i < out->numberoffaces; i++) {
        Face* f = new Face;
        Vertex* v = del.get_vertex(out->facelist[3*i]);
        Vertex* u = del.get_vertex(out->facelist[3*i+1]);
        Vertex* w = del.get_vertex(out->facelist[3*i+2]);
        f->v.push_back(v);
        f->v.push_back(u);
        f->v.push_back(w);

        Edge* e = vor.get_edge(i);
        f->dual = e;
        e->dual = f;

        del.add(f);
    }
    compute_normals(del);
}

void delaunay()
{
    tetrahedralizeio* in = new tetrahedralizeio;
    tetrahedralizeio* out = new tetrahedralizeio;
    tetrahedralizeio* vorout = new tetrahedralizeio;
    
    in->pointlist = new REAL[mesh.vertex_count()*3];
    in->pointattributelist = NULL;
    in->pointmarkerlist = NULL;
    in->numberofpoints = mesh.vertex_count();
    in->numberofpointattributes = 0;

    out->pointlist = NULL;
    out->edgelist = NULL;
    out->facelist = NULL;

    vorout->pointlist = NULL;
    vorout->edgelist = NULL;
    vorout->normlist = NULL;
    vorout->facelist = NULL;
    
    vorout->numberofpointattributes = 0;

    for(size_t i = 0; i < mesh.vertex_count(); i++) {
        Vertex* v = mesh.get_vertex(i);
        in->pointlist[3*i] = v->v[0];
        in->pointlist[3*i+1] = v->v[1];
        in->pointlist[3*i+2] = v->v[2];
    }
    tetrahedralize("vzefQ", in, out, vorout);
    convert(out, vorout);
    
    delete in;
    delete out;
    delete vorout;
}

void init()
{
    glClearColor(0.0, 0.71, 1.0, 1.0);
    glShadeModel(GL_SMOOTH);
    
    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ctx.ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, ctx.diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, ctx.specular);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(ctx.fovy, 1, ctx.znear, ctx.zfar);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(ctx.width, ctx.height);
    glutCreateWindow("Del");
    init();
    menu();

    mesh = read_obj(argv[1]);
    center_on_screen(mesh);
    meshes.push_back(mesh);
    
    delaunay();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);

    glutMainLoop();

    return 0;
}
