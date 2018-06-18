#version 330 core

#include "lib/render_context.glslh"
#include "interface/sprite.glslh"
#include "lib/util.glslh"

uniform int numslots;
uniform int slotfilling;

void main(void) {
    float r = length(texCoord-0.5)*2.;
    float phi = atan(texCoord.y-0.5,texCoord.x-0.5)+pi*2.5;

    int slot = int(phi/2./pi*numslots+0.5)%numslots;
    bool slotfilled = (slotfilling & (1<<slot)) > 0;
    float inslot = float(cos(phi*numslots) > 0.8+(1-r));
    float fill = step(0.92*(1-inslot),r)*step(r,0.95+0.05*inslot);
    fragColor = mix(vec4(1.0,1.0,1.0,0.2*r_color.a),r_color*float(slotfilled),inslot)*fill;
}
