#include "shared.h"

Physics_State physics_state = {0};
static Physics_State *state = &physics_state;
static Hit *hit_array;
static u32 next_hit_index = 0;

void physics_setup() {
	state->static_body_array = calloc(MAX_STATIC_BODIES, sizeof(*state->static_body_array));
	state->trigger_array = calloc(MAX_TRIGGERS, sizeof(*state->trigger_array));
	hit_array = calloc(MAX_ENTITIES * MAX_ENTITIES, sizeof(*hit_array));
}

Hit *aabb_intersect_aabb(AABB self, AABB other) {
	Hit *hit = &hit_array[next_hit_index++];
	f32 dx = self.position[0] - other.position[0];
	f32 px = self.half_sizes[0] + other.half_sizes[0] - fabs(dx);

	// Must not be intersecting because the distance between the two
	// centre points is greater than the x-axis half sizes of both added
	// together.
	if (px <= 0)
		return NULL;

	f32 dy = self.position[1] - other.position[1];
	f32 py = self.half_sizes[1] + other.half_sizes[1] - fabs(dy);

	// Same test as above but on the y-axis.
	if (py <= 0)
		return NULL;

	// Calculate how far inside (delta), which side (normal) and the point
	// of contact (position).
	if (px < py) {
		f32 sx = fsign(dx);
		hit->delta[0] = px * sx;
		hit->normal[0] = sx;
		hit->position[0] = other.position[0] + other.half_sizes[0] * sx;
		hit->position[1] = self.position[1];
	} else {
		f32 sy = fsign(dy);
		hit->delta[1] = py * sy;
		hit->normal[1] = sy;
		hit->position[0] = self.position[0];
		hit->position[1] = other.position[1] + other.half_sizes[1] * sy;
	}

	return hit;
}

Static_Body *physics_static_body_create(f32 x, f32 y, f32 half_width, f32 half_height, u8 layer_mask) {
	if (state->static_body_array_count == MAX_STATIC_BODIES) {
		error_and_exit(EXIT_FAILURE, "No static bodies left.\n");
	}

	u32 index = state->static_body_array_count++;
	Static_Body static_body = {{{x, y}, {half_width, half_height}}};
	static_body.layer_mask = layer_mask;
	state->static_body_array[index] = static_body;

	return &state->static_body_array[index];
}

Trigger *physics_trigger_create(f32 x, f32 y, f32 half_width, f32 half_height) {
	if (state->trigger_array_count == MAX_TRIGGERS) {
		error_and_exit(EXIT_FAILURE, "No triggers left.\n");
	}

	u32 index = state->trigger_array_count++;
	Trigger trigger = {{{x, y}, {half_width, half_height}}};
	trigger.id = index;
	state->trigger_array[index] = trigger;

	return &state->trigger_array[index];
}

static u8 can_collide(u8 a_id, u8 b_id) {
	u8 a = state->mask_array[a_id];
	return ((1 << b_id & a) > 0);
}

void physics_tick(f32 delta_time, Entity *entity_array) {
	for (u32 i = 0; i < MAX_ENTITIES; ++i) {
		Entity *entity = &entity_array[i];
		if (!entity->is_in_use)
			continue;

		// Triggers. Check first because otherwise the velocity is added and
		// entities can trigger things through static objects.
		for (u32 j = 0; j < state->trigger_array_count; ++j) {
			Trigger *trigger = &state->trigger_array[j];
			Hit *hit = aabb_intersect_aabb(entity->aabb, trigger->aabb);
			if (hit != NULL && trigger->on_trigger != NULL)
				trigger->on_trigger(i, j, *hit);
		}

		// Collision events with other entities.
		for (u32 j = 0; j < MAX_ENTITIES; ++j) {
			Entity *other = &entity_array[j];
			if (i == j || !other->is_in_use || !can_collide(entity->layer_mask, other->layer_mask))
				continue;
			Hit *hit = aabb_intersect_aabb(entity->aabb, other->aabb);
			if (hit != NULL && entity->on_collide != NULL)
				entity->on_collide(i, j, *hit);
		}

		// Integrate.
		if (!entity->is_kinematic) {
			entity->velocity[1] += GRAVITY;
			if (entity->velocity[1] < TERMINAL_VELOCITY)
				entity->velocity[1] = TERMINAL_VELOCITY;
		}

		entity->velocity[0] += entity->acceleration[0];
		entity->velocity[1] += entity->acceleration[1];

		if (entity->desired_velocity[0] != 0 && fabs(entity->velocity[0]) > fabs(entity->desired_velocity[0])) {
			entity->velocity[0] = entity->desired_velocity[0];
		}

		entity->aabb.position[0] += entity->velocity[0] * delta_time;
		entity->aabb.position[1] += entity->velocity[1] * delta_time;

		// Static collisions.
		u32 was_hit = 0;
		for (u32 j = 0; j < state->static_body_array_count; ++j) {
			Static_Body *static_body = &state->static_body_array[j];
			Hit *hit = aabb_intersect_aabb(entity->aabb, static_body->aabb);
			if (hit != NULL) {
				if (!can_collide(entity->layer_mask, static_body->layer_mask))
					continue;

				entity->aabb.position[0] += hit->delta[0];
				entity->aabb.position[1] += hit->delta[1];

				if (hit->normal[0] == 0 && hit->normal[1] == 1) {
					entity->is_grounded = 1;
					entity->velocity[1] = 0;
				}

				if (hit->normal[1] == -1)
					entity->velocity[1] = 0;
				was_hit = 1;

				if (entity->on_collide_static != NULL)
					entity->on_collide_static(i, j, *hit);
			}
		}

		if (was_hit == 0)
			entity->is_grounded = 0;
	}
}

void physics_cleanup() {
	memset(hit_array, 0, next_hit_index * sizeof(Hit));
	next_hit_index = 0;
}
