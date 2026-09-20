#include "render/DebugWorldRenderer.hpp"
#include "render/NeeshegoRenderPass.hpp"
#include "render/NeeshegoPerformancePass.hpp"
#include "render/NeeshegoCharacterActorPass.hpp"

// L20/L22/L23 native binding shim.
//
// L20 routes the mature DebugWorldRenderer primitive draw sites and final
// render-pass close through the Neeshego manga execution layer. L22 extends the
// same game-only seam with character-performance rig execution. L23 adds the
// independent character-actor pass so physically unbound canonical cast members
// such as The Reaper can render from authoritative world transforms without
// being forced through PlayerRuntime or the humanoid NPC path. Mannequin-lab
// targets continue compiling the unmodified renderer directly.
#define SDL_DrawGPUPrimitives(pass, vertex_count, instance_count, first_vertex, first_instance) \
    ::hakui::render::neeshegoDrawGPUPrimitives(                                      \
        scene, player, pass, vertex_count, instance_count, first_vertex, first_instance \
    )

#define SDL_EndGPURenderPass(pass)                                                   \
    ::hakui::render::neeshegoEndCharacterActorRenderPass(                           \
        scene, player, commands, pass, cubeVertexBuffer_, pipeline_, glassPipeline_, \
        kCubeVertexCount, viewProjection                                             \
    )

#include "render/DebugWorldRenderer.cpp"

#undef SDL_EndGPURenderPass
#undef SDL_DrawGPUPrimitives
