/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2018, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2018, Andrei Alexeyev <akari@alienslab.net>.
 */

#include "taisei.h"

#include "score.h"
#include "global.h"

static void score_state_reset_slots(ScoreState *s) {
	s->numslots = sqrt(9+2*s->level);
	s->fullslots = 0;
	s->slots = realloc(s->slots,sizeof(bool)*s->numslots);
	memset(s->slots,0,sizeof(bool)*s->numslots);
}

static int score_state_projectile_slot(ScoreState *s, Projectile *p) {
	complex velocity = p->pos-p->prevpos;
	if(cabs(velocity) < 0.01)
		return -1;
	// this is written in the same way as in the shader so the slots are in the right place.
	double phi = carg(velocity)+M_PI*2.5; // this shift is so that phi is positive and phi = 0 points up. Hopefully.
	return (int)(phi/2./M_PI*s->numslots+0.5) % s->numslots;
}

void score_state_create(ScoreState *s) {
	memset(s,0,sizeof(ScoreState));
	score_state_reset_slots(s);
}

void score_state_free(ScoreState *s) {
	free(s->slots);
}

void score_state_highlight_that_bullet(ScoreState *s, Projectile *p) {
	// highlighting every bullet would be too annoying so we only highlight them when they are the last missing ones!
	if(s->fullslots == s->numslots-1 || s->fullslots/(double)s->numslots > 0.75) {
		int slot = score_state_projectile_slot(s,p);
		if(slot < 0)
			return;
		if(s->slots[slot])
			return;

		r_draw_sprite(&(SpriteParams) {
			.sprite = "fairy_circle",
			.scale.both = 1./50*cabs(p->size),
			.rotation.angle = DEG2RAD * global.frames * 10,
			.color = p->color,
			.pos = { creal(p->pos), cimag(p->pos) },
		});
	}
}
		

void score_state_graze(ScoreState *s, Projectile *p) {
	int slot = score_state_projectile_slot(s,p);
	if(slot < 0)
		return;
	if(!s->slots[slot])
		s->fullslots++;
	s->slots[slot] = true;

	bool complete = true;
	for(int i = 0; i < s->numslots; i++) {
		complete = complete && s->slots[i];
		if(!complete)
			break;
	}

	if(complete) {
		s->level++;
		score_state_reset_slots(s);
	}
}

void score_state_draw(ScoreState *s, float alpha) {
	int filling = 0;
	for(int i = 0; i < s->numslots; i++) {
		filling |= (1<<i)*s->slots[i];
	}
	
	ShaderProgram *prog_saved = r_shader_current();
	r_shader("score_ring");
	r_color4(0.8,0.2,0.2,alpha);
	r_blend(BLEND_ADD);
	r_mat_push();
	r_uniform_int("numslots",s->numslots);
	r_uniform_int("slotfilling",filling);
	r_mat_translate(creal(global.plr.pos),cimag(global.plr.pos),0);
	r_mat_scale(100,100,1);
	r_draw_quad();
	r_mat_pop();
	r_color4(1,1,1,1);
	r_shader_ptr(prog_saved);
}
