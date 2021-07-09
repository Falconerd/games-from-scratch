#include "shared.h"

static Physics_Context context = {0};

Physics_Context *physics_setup(u32 max_bodies) {
	context.body_array_max = max_bodies;
	context.body_array = malloc(max_bodies * (sizeof *context.body_array));
}

Hit aabb_intersect_aabb(AABB *self, AABB *other) {
	Hit hit = {0};
	f32 dx = other->position[0] - self->position[0];
	f32 px = other->half_sizes[0] + self->half_sizes[0] - fabs(dx);

	// Must not be intersecting because the distance between the two
	// centre points is greater than the x-axis half sizes of both added
	// together.
	if (px <= 0)
		return hit;

	f32 dy = other->position[1] - self->position[1];
	f32 py = other->half_sizes[1] + self->half_sizes[1] - fabs(dy);

	// Same test as above but on the y-axis.
	if (py <= 0)
		return hit;

	// Calculate how far inside (delta), which side (normal) and the point
	// of contact (position).
	if (px < py) {
		printf("px < py\n");
		f32 sx = fsign(dx);
		hit.delta[0] = px * sx;
		hit.normal[0] = sx;
		hit.position[0] = self->position[0] + self->half_sizes[0] * sx;
		hit.position[1] = other->position[1];
	} else {
		printf("px > py\n");
		f32 sy = fsign(dy);
		hit.delta[1] = py * sy;
		hit.normal[1] = sy;
		hit.position[0] = other->position[0];
		hit.position[1] = self->position[1] + self->half_sizes[1] * sy;
	}

	hit.body = (Body*)self;

	return hit;
}
