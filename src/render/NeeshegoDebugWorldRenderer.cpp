#include "render/DebugWorldRenderer.hpp"
#include "render/NeeshegoRenderPass.hpp"

// L20 native binding shim.
//
// Keep the mature DebugWorldRenderer implementation intact while routing its
// two primitive draw sites and final render-pass close through the Neeshego
// execution layer. Mannequin-lab targets continue compiling the unmodified
// renderer directly, so this presentation layer is scoped to the actual HAKUI
// game client.
#define SDL_DrawGPUPrimitives(pass, vertex_count, instance_count, first_vertex, first_instance) \
    ::hakui::render::neeshegoDrawGPUPrimitives(                                      \
        scene, player, pass, vertex_count, instance_count, first_vertex, first_instance \
    )

#define SDL_EndGPURenderPass(pass)                                                   \
    ::hakui::render::neeshegoEndGPURenderPass(                                      \
        scene, player, commands, pass, cubeVertexBuffer_, pipeline_, glassPipeline_, \
        kCubeVertexCount                                                             \
    )

#include "render/DebugWorldRenderer.cpp"

#undef SDL_EndGPURenderPass
#undef SDL_DrawGPUPrimitives
