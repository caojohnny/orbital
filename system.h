#ifndef ORBITAL_SYSTEM_H
#define ORBITAL_SYSTEM_H

struct vector {
    double x;
    double y;
    double z;
};

struct body {
    double mass;
    struct vector F_net_ext;
    struct vector pos;
    struct vector vel;
    struct vector acl;
};

void add_body(double mass, struct body *out);

void recompute_system(double dt, int n_bodies, struct body *bodies);

#endif // ORBITAL_SYSTEM_H
