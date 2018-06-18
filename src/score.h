/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2018, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2018, Andrei Alexeyev <akari@alienslab.net>.
 */

#pragma once
#include "taisei.h"

#include "projectile.h"

typedef struct ScoreState {
	int level;
	bool *slots;
	int numslots;
	int fullslots;
} ScoreState;

void score_state_create(ScoreState *s);
void score_state_free(ScoreState *s);

void score_state_graze(ScoreState *s, Projectile *p);
void score_state_draw(ScoreState *s, float alpha);

void score_state_highlight_that_bullet(ScoreState *s, Projectile *p);
