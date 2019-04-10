/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@alienslab.net>.
 */

#include "taisei.h"

#include "global.h"
#include "plrmodes.h"
#include "marisa.h"
#include "renderer/api.h"

static void marisa_star_trail_draw(Projectile *p, int t) {
	float s = 1 - t / (double)p->timeout;

	Color *clr = COLOR_COPY(&p->color);

	color_mul_scalar(clr, s * 0.5);

	r_mat_push();
	r_mat_translate(creal(p->pos), cimag(p->pos), 0);
	// render_rotate_deg(t*10, 0, 0, 1);
	r_mat_rotate_deg(p->angle*180/M_PI+90, 0, 0, 1);
	r_mat_scale(s, s, 1);
	ProjDrawCore(p, clr);
	r_mat_pop();
}

static int marisa_star_projectile(Projectile *p, int t) {
	if(t == EVENT_DEATH || t == EVENT_BIRTH)
		return ACTION_ACK;
	float c = 0.3 * psin(t * 0.2);
	p->color = *RGB(1 - c, 0.7 + 0.3 * psin(t * 0.1), 0.9 + c/3);
	Color *clr = COLOR_COPY(&p->color);
	clr->a = 0;

	
	float freq = 0.1;
	p->angle = t * freq;
	
	double focus = fabs(global.plr.focus)/30.0;

	double focusfac = 1;
	if(focus > 0) {
		focusfac = t*0.015-1/focus;
		if(focusfac < 0) {
			focusfac = tanh(focusfac);
		} else {
			focusfac = focusfac*exp(focusfac);
		}
	}

	double verticalfac = - 5*t*(1+0.01*t) + 10*t/(0.01*t+1);
	p->pos = global.plr.pos + focusfac*cbrt(0.1*t)*creal(p->args[0])* 70 * sin(freq*t+cimag(p->args[0])) + I*verticalfac;

	/*PARTICLE(
		.sprite_ptr = get_sprite("proj/maristar"),
		.pos = p->pos,
		.color = clr,
		.timeout = 8,
		.draw_rule = marisa_star_trail_draw,
		.angle = p->angle,
		.flags = PFLAG_NOREFLECT,
		.layer = LAYER_PARTICLE_LOW,
	);*/

	return ACTION_NONE;
}

static int marisa_star_slave(Enemy *e, int t) {
	double focus = global.plr.focus/30.0;

	if(abs(global.plr.focus) > 0) {
		e->args[2] += 1;
	} else if(creal(e->args[2]) > 0) {
		e->args[2] -= 5;
	}


	for(int i = 0; i < 3; ++i) {
		if(player_should_shoot(&global.plr, true) && !((global.frames+2*i) % 5)) {
			complex v = e->args[1] * 2;

			v = creal(e->pos-global.plr.pos);
			v /= cabs(v);
			v -= I*0.1*t+I*i;


			PROJECTILE(
				.proto = pp_maristar,
				.pos = e->pos,
				.color = RGB(1.0, 0.5, 1.0),
				.rule = marisa_star_projectile,
				// .draw_rule = marisa_star,
				.args = { v },
				.type = PROJ_PLAYER,
				.damage = e->args[3],
				.shader = "sprite_default",
			);
		}
	}

	double angle = 0.05*global.frames+e->args[0];
	if(cos(angle) < 0) {
		e->ent.draw_layer = LAYER_BACKGROUND;
	} else {
		e->ent.draw_layer = LAYER_PLAYER_SLAVE;
	}

	e->pos = global.plr.pos + (100-40*fabs(focus))*sin(angle)+45*I;

	return 1;
}


static int marisa_star_orbit_star(Projectile *p, int t) { // XXX: because growfade is the worst
	if(t >= 0) {
		p->args[0] += p->args[0]/cabs(p->args[0])*0.15;
		p->angle += 0.1;
	}

	return linear(p,t);
}

static Color marisa_slaveclr(int i, float low) {
	int numslaves = 5;
	assert(i >= 0 && i < numslaves);

	return *HSLA(i/(float)numslaves, 1, low, 1);
}

static int marisa_star_orbit(Enemy *e, int t) {
	Color color = marisa_slaveclr(rint(creal(e->args[0])), 0.6);

	float tb = player_get_bomb_progress(&global.plr);
	if(t == EVENT_BIRTH) {
		global.shake_view = 8;
		return 1;
	} else if(t == EVENT_DEATH) {
		global.shake_view = 0;
	}
	if(t < 0) {
		return 1;
	}

	if(tb >= 1 || !player_is_bomb_active(&global.plr)) {
		return ACTION_DESTROY;
	}

	double r = 100*pow(tanh(t/20.),2);
	complex dir = e->args[1]*r*cexp(I*(sqrt(1000+t*t+0.03*t*t*t))*0.04);
	e->pos = global.plr.pos+dir;

	float fadetime = 3./4;

	if(tb >= fadetime) {
		color.a = 1 - (tb - fadetime) / (1 - fadetime);
	}

	color_mul_alpha(&color);
	color.a = 0;

	if(t%1 == 0) {
		Color *color2 = COLOR_COPY(&color);
		color_mul_scalar(color2, 0.5);
		
		PARTICLE(
			.sprite_ptr = get_sprite("part/maristar_orbit"),
			.pos = e->pos,
			.color = color2,
			.timeout = 10,
			.angle = t*0.1,
			.draw_rule = GrowFade,
			.args = { 1, tb*4},
			.flags = PFLAG_NOREFLECT,
		);
	}

	if(t%(10-t/30) == 0 && tb < fadetime) {
		PARTICLE(
			.sprite = "maristar_orbit",
			.pos = e->pos,
			.color = &color,
			.rule = marisa_star_orbit_star,
			.draw_rule = GrowFade,
			.timeout = 150,
			.flags = PFLAG_NOREFLECT,
			.args = { -5*dir/cabs(dir), 5 },
		);
	}

	return 1;
}


#define NUM_MARISTAR_SLAVES 5
static void marisa_star_orbit_visual(Enemy *e, int t, bool render) {
	if(!render) {
		return;
	}

	float tb = player_get_bomb_progress(&global.plr);
	Color color = marisa_slaveclr(rint(creal(e->args[0])), 0.2);

	float fade = 1;

	if(tb < 1./6) {
		fade = tb*6;
		fade = sqrt(fade);
	}

	if(tb > 3./4) {
		fade = 1-tb*4 + 3;
		fade *= fade;
	}

	color_mul_scalar(&color, fade);

	if(e->args[0] == 0) {
		MarisaBeamInfo beams[NUM_MARISTAR_SLAVES];
		for(int i = 0; i < NUM_MARISTAR_SLAVES; i++) {
			beams[i].origin = global.plr.pos + (e->pos-global.plr.pos)*cexp(I*2*M_PI/NUM_MARISTAR_SLAVES*i);
			beams[i].size = 250*fade+VIEWPORT_H*1.5*I;
			beams[i].angle = carg(e->pos - global.plr.pos) + M_PI/2 + 2*M_PI/NUM_MARISTAR_SLAVES*i;
			beams[i].t = global.plr.bombtotaltime * tb;
		}
		marisa_common_masterspark_draw(NUM_MARISTAR_SLAVES, beams, fade);
	}

	r_mat_push();
	r_mat_translate(creal(e->pos),cimag(e->pos),0);
	color.a = 0;
	r_color(&color);
	r_mat_rotate_deg(t*10,0,0,1);
	draw_sprite_batched(0,0,"fairy_circle");
	r_mat_scale(0.6,0.6,1);
	draw_sprite_batched(0,0,"part/lightningball");
	r_mat_pop();
}

static void marisa_star_bomb(Player *plr) {
	play_sound("bomb_marisa_b");

	for(int i = 0; i < NUM_MARISTAR_SLAVES; i++) {
		complex dir = cexp(2*I*M_PI/NUM_MARISTAR_SLAVES*i);
		Enemy *e = create_enemy2c(plr->pos, ENEMY_BOMB, marisa_star_orbit_visual, marisa_star_orbit, i ,dir);
		e->ent.draw_layer = LAYER_PLAYER_FOCUS - 1;
	}
}
#undef NUM_MARISTAR_SLAVES

static void marisa_star_bombbg(Player *plr) {
	if(!player_is_bomb_active(plr)) {
		return;
	}

	float t = player_get_bomb_progress(&global.plr);
	
	ShaderProgram *s = r_shader_get("maristar_bombbg");
	r_shader_ptr(s);
	r_uniform_float("t", t);
	r_uniform_float("decay", 1);
	r_uniform_vec2("plrpos", creal(global.plr.pos)/VIEWPORT_W, cimag(global.plr.pos)/VIEWPORT_H);
	fill_viewport(0,0,1,"marisa_bombbg");
	r_shader_standard();
}

static void marisa_star_respawn_slaves(Player *plr, short npow) {
	double dmg = 56;

	for(Enemy *e = plr->slaves.first, *next; e; e = next) {
		next = e->next;

		if(e->hp == ENEMY_IMMUNE) {
			delete_enemy(&plr->slaves, e);
		}
	}

	for(int i = 0; i < npow/100; i++) {
		create_enemy_p(&plr->slaves, 40.0*I, ENEMY_IMMUNE, marisa_common_slave_visual, marisa_star_slave, 2*M_PI*i/(npow/100), 0, 0, dmg);
	}

	for(Enemy *e = plr->slaves.first; e; e = e->next) {
		e->ent.draw_layer = LAYER_PLAYER_SLAVE;
	}
}

static void marisa_star_power(Player *plr, short npow) {
	if(plr->power / 100 == npow / 100) {
		return;
	}

	marisa_star_respawn_slaves(plr, npow);
}

static void marisa_star_init(Player *plr) {
	marisa_star_respawn_slaves(plr, plr->power);
}

static void marisa_star_shot(Player *plr) {
	int p = plr->power / 100;
	marisa_common_shot(plr, 175 - 10*p);
}

static double marisa_star_property(Player *plr, PlrProperty prop) {
	switch(prop) {
		case PLR_PROP_SPEED: {
			double s = marisa_common_property(plr, prop);

			if(player_is_bomb_active(plr)) {
				s /= 4.0;
			}

			return s;
		}

		default:
			return marisa_common_property(plr, prop);
	}
}

static void marisa_star_preload(void) {
	const int flags = RESF_DEFAULT;

	preload_resources(RES_SPRITE, flags,
		"proj/marisa",
		"proj/maristar",
		"part/maristar_orbit",
		"hakkero",
		"masterspark_ring",
	NULL);

	preload_resources(RES_TEXTURE, flags,
		"marisa_bombbg",
	NULL);

	preload_resources(RES_SHADER_PROGRAM, flags,
		"masterspark",
		"maristar_bombbg",
		"sprite_hakkero",
	NULL);

	preload_resources(RES_SFX, flags | RESF_OPTIONAL,
		"bomb_marisa_b",
	NULL);
}

PlayerMode plrmode_marisa_b = {
	.name = "Star Sign",
	.character = &character_marisa,
	.dialog = &dialog_marisa,
	.shot_mode = PLR_SHOT_MARISA_STAR,
	.procs = {
		.property = marisa_star_property,
		.bomb = marisa_star_bomb,
		.bombbg = marisa_star_bombbg,
		.shot = marisa_star_shot,
		.power = marisa_star_power,
		.preload = marisa_star_preload,
		.init = marisa_star_init,
		.think = player_placeholder_bomb_logic,
	},
};
