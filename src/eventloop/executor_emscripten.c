/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@alienslab.net>.
 */

#include "taisei.h"

#include "eventloop_private.h"
#include "events.h"
#include "global.h"
#include <emscripten.h>

static FrameTimes frame_times;

static bool em_handle_resize_event(SDL_Event *event, void *arg);

static void em_loop_callback(void) {
	LoopFrame *frame = evloop.stack_ptr;

	if(!frame) {
		events_unregister_handler(em_handle_resize_event);
		emscripten_cancel_main_loop();
		return;
	}

	if(time_get() < frame_times.next) {
		return;
	}

	frame_times.start = time_get();
	frame_times.target = frame->frametime;
	frame_times.next += frame_times.target;
	global.fps.busy.last_update_time = frame_times.start;

	LogicFrameAction lframe_action = handle_logic(&frame, &frame_times);

	if(!frame || lframe_action == LFRAME_STOP) {
		return;
	}

	run_render_frame(frame);
	fpscounter_update(&global.fps.busy);
}

static bool em_handle_resize_event(SDL_Event *event, void *arg) {
	emscripten_cancel_main_loop();
	emscripten_set_main_loop(em_loop_callback, 0, false);
	emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);
	return false;
}

void eventloop_run(void) {
	frame_times.next = time_get();
	emscripten_set_main_loop(em_loop_callback, 0, false);
	emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);
	events_register_handler(&(EventHandler) {
		em_handle_resize_event, NULL, EPRIO_SYSTEM, MAKE_TAISEI_EVENT(TE_VIDEO_MODE_CHANGED)
	});

	(__extension__ EM_ASM({
		Module['onFirstFrame']();
	}));
}