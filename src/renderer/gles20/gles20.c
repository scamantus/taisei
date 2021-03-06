/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@alienslab.net>.
 */

#include "taisei.h"

#include "gles20.h"
#include "../glescommon/gles.h"
#include "../glescommon/texture.h"
#include "../gl33/gl33.h"
#include "../gl33/vertex_array.h"
#include "index_buffer.h"

static void gles20_init(void) {
	gles_init(&_r_backend_gles20, 2, 0);
}

static void gles20_init_context(SDL_Window *w) {
	gles_init_context(w);

	if(!glext.vertex_array_object) {
		log_fatal("GL doesn't support VAOs; no fallback implemented yet, sorry.");
	}
}

static void gles20_draw_indexed(VertexArray *varr, Primitive prim, uint firstidx, uint count, uint instances, uint base_instance) {
	assert(count > 0);
	assert(varr->index_attachment != NULL);
	GLuint gl_prim = gl33_prim_to_gl_prim(prim);

	void *state;
	gl33_begin_draw(varr, &state);

	if(instances) {
		if(base_instance) {
			glDrawElementsInstancedBaseInstance(gl_prim, count, GLES20_IBO_GL_DATATYPE, varr->index_attachment->elements, instances, base_instance);
		} else {
			glDrawElementsInstanced(gl_prim, count, GLES20_IBO_GL_DATATYPE, varr->index_attachment->elements, instances);
		}
	} else {
		glDrawElements(gl_prim, count, GLES20_IBO_GL_DATATYPE, varr->index_attachment->elements);
	}

	gl33_end_draw(state);
}

static void gles20_vertex_array_attach_index_buffer(VertexArray *varr, IndexBuffer *ibuf) {
	// We override this function to prevent setting the index buffer dirty bit.
	// Otherwise the GL33 VAO code would try to dereference index_attachment as
	// another kind of struct (the GL33 IBO implementation).
	varr->index_attachment = ibuf;
}

RendererBackend _r_backend_gles20 = {
	.name = "gles20",
	.funcs = {
		.init = gles20_init,
		.screenshot = gles_screenshot,
		.index_buffer_create = gles20_index_buffer_create,
		.index_buffer_get_capacity = gles20_index_buffer_get_capacity,
		.index_buffer_get_debug_label = gles20_index_buffer_get_debug_label,
		.index_buffer_set_debug_label = gles20_index_buffer_set_debug_label,
		.index_buffer_set_offset = gles20_index_buffer_set_offset,
		.index_buffer_get_offset = gles20_index_buffer_get_offset,
		.index_buffer_add_indices = gles20_index_buffer_add_indices,
		.index_buffer_destroy = gles20_index_buffer_destroy,
		.draw_indexed = gles20_draw_indexed,
		.vertex_array_attach_index_buffer = gles20_vertex_array_attach_index_buffer,
	},
	.custom = &(GLBackendData) {
		.vtable = {
			.texture_type_info = gles_texture_type_info,
			.texture_format_caps = gles_texture_format_caps,
			.init_context = gles20_init_context,
		}
	},
};
