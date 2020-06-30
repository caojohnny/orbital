#include "system.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

// CODATA 2018 value for G
static const double G = 0.000000000066743;

void add_body(double mass, struct body *out) {
    struct body body = {
            mass, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
    };
    *out = body;
}

static struct vector diff(struct vector a, struct vector b) {
    struct vector diff = {
            a.x - b.x, a.y - b.y, a.z - b.z
    };
    return diff;
}

static double mag(struct vector v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

void recompute_system(double dt, int n_bodies, struct body *bodies) {
    size_t temp_len = n_bodies * sizeof(struct body);
    struct body *temp = malloc(temp_len);
    memcpy(temp, bodies, temp_len);
    
    for (int i = 0; i < n_bodies; ++i) {
        struct body *body = bodies + i;

        struct vector F_net = {0, 0, 0};
        for (int j = 0; j < n_bodies; ++j) {
            if (j == i) {
                continue;
            }

            struct body *other_body = temp + j;
            struct vector r = diff(other_body->pos, body->pos);
            double r_mag = mag(r);
            double F_g = G * body->mass * other_body->mass / (r_mag * r_mag);

            F_net.x += F_g * r.x / r_mag;
            F_net.y += F_g * r.y / r_mag;
            F_net.z += F_g * r.z / r_mag;
        }

        struct vector acl = { F_net.x / body->mass, F_net.y / body->mass, F_net.z / body->mass };
        body->acl = acl;
    }

    free(temp);

    for (int i = 0; i < n_bodies; ++i) {
        struct body *body = bodies + 1;

        struct vector vel = { body->acl.x * dt, body->acl.y * dt, body->acl.z * dt };
        body->vel.x += vel.x;
        body->vel.y += vel.y;
        body->vel.z += vel.z;

        struct vector pos = { body->vel.x * dt, body->vel.y * dt, body->vel.z * dt };
        body->pos.x += pos.x;
        body->pos.y += pos.y;
        body->pos.z += pos.z;
    }
}
