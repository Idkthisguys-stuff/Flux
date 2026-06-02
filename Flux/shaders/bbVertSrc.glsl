#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

out vec2 UV;

uniform vec3  worldPos;
uniform float size;
uniform mat4  view;
uniform mat4  projection;

void main()
{
    // ── BUG 4 FIX ────────────────────────────────────────────────────────────
    // Extract the camera's right and up axes directly from the view matrix.
    // Row-major extraction of the inverse-view columns gives world-space axes.
    // view[col][row] in GLSL column-major layout:
    //   camRight = (view[0][0], view[1][0], view[2][0])
    //   camUp    = (view[0][1], view[1][1], view[2][1])
    //
    // The billboard centre is offset in WORLD SPACE before projection.
    // This is the ONLY projection step — the original shader projected worldPos
    // separately AND added an extra NDC-space offset (screenPos.xy += ...*w),
    // causing icons to drift with distance because the w-division happened twice.
    // ─────────────────────────────────────────────────────────────────────────
    vec3 camRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 camUp    = vec3(view[0][1], view[1][1], view[2][1]);

    // Expand the quad in world space, then project once.
    vec3 vPos = worldPos
              + camRight * aPos.x * size
              + camUp    * aPos.y * size;

    UV          = aUV;
    gl_Position = projection * view * vec4(vPos, 1.0);
}
