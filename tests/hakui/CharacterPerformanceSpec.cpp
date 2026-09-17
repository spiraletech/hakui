#include "character/CharacterActorAuthority.hpp"
#include "character/CharacterPerformanceProfile.hpp"
#include "character/CharacterPoseExecutor.hpp"
#include "core/GameRuntime.hpp"

#include <cassert>
#include <cmath>

int main()
{
    using namespace hakui::character;

    const CharacterPerformanceProfile* agnathos =
        canonicalPerformanceProfile(CharacterId::Agnathos);
    const CharacterPerformanceProfile* saelis =
        canonicalPerformanceProfile(CharacterId::Saelis);
    const CharacterPerformanceProfile* reaper =
        canonicalPerformanceProfile(CharacterId::Reaper);

    assert(agnathos != nullptr);
    assert(saelis != nullptr);
    assert(reaper != nullptr);
    assert(canonicalPerformanceProfile(CharacterId::None) == nullptr);

    assert(agnathos->motion == CharacterMotionGrammar::GuardianStillness);
    assert(agnathos->defaultExpression == CharacterExpression::RasterBlank);
    assert(agnathos->usesHumanFacialMuscles);
    assert(agnathos->usesRasterEyeChannel);

    assert(saelis->motion == CharacterMotionGrammar::SyntheticPrecision);
    assert(saelis->defaultExpression == CharacterExpression::SyntheticNeutral);
    assert(saelis->usesHumanFacialMuscles);
    assert(!saelis->usesRasterEyeChannel);

    assert(reaper->motion == CharacterMotionGrammar::ReaperStaccato);
    assert(reaper->defaultExpression == CharacterExpression::ReaperStill);
    assert(!reaper->usesHumanFacialMuscles);
    assert(reaper->usesJawChannel);

    const CharacterPerformanceFrame agnathosDialogue =
        composeCharacterPerformance(CharacterId::Agnathos,
                                    CharacterPerformanceEvent::Dialogue);
    assert(agnathosDialogue.expression == CharacterExpression::RasterFocused);
    assert(agnathosDialogue.gestureWeight > agnathos->gestureAmplitude);
    assert(agnathosDialogue.cranialIntensity > 0.60f);

    const CharacterPerformanceFrame saelisDialogue =
        composeCharacterPerformance(CharacterId::Saelis,
                                    CharacterPerformanceEvent::Dialogue);
    assert(saelisDialogue.expression == CharacterExpression::SyntheticWarm);
    assert(saelisDialogue.snapWeight < 0.50f);

    const CharacterPerformanceFrame reaperDialogue =
        composeCharacterPerformance(CharacterId::Reaper,
                                    CharacterPerformanceEvent::Dialogue);
    assert(reaperDialogue.expression == CharacterExpression::ReaperOpenJaw);
    assert(reaperDialogue.jawOpen > 0.40f);

    const CharacterPerformanceFrame reaperOverload =
        composeCharacterPerformance(CharacterId::Reaper,
                                    CharacterPerformanceEvent::Overload);
    assert(reaperOverload.expression == CharacterExpression::ReaperOpenJaw);
    assert(reaperOverload.jawOpen == 1.0f);
    assert(reaperOverload.snapWeight == 1.0f);
    assert(reaperOverload.staccatoWeight == 1.0f);

    const CharacterPerformanceFrame agnathosRealm =
        composeCharacterPerformance(CharacterId::Agnathos,
                                    MangaRenderEvent::RealmShift);
    assert(agnathosRealm.expression == CharacterExpression::RasterFocused);
    assert(agnathosRealm.cranialIntensity == 1.0f);

    assert(performanceEventFor(MangaRenderEvent::Dialogue) ==
           CharacterPerformanceEvent::Dialogue);
    assert(performanceEventFor(MangaRenderEvent::CombatImpact) ==
           CharacterPerformanceEvent::CombatImpact);
    assert(performanceEventFor(MangaRenderEvent::Overload) ==
           CharacterPerformanceEvent::Overload);

    // L22: authored performance now resolves into actual rig-space channels.
    const CharacterPoseExecution agnathosPose = executeCharacterPerformance(
        agnathosDialogue,
        0.25f
    );
    assert(agnathosPose.active);
    assert(agnathosPose.rig == CharacterRigArchetype::Humanoid);
    assert(!agnathosPose.skeletalOnly);
    assert(agnathosPose.usesHumanFacialMuscles);
    assert(agnathosPose.usesRasterEyeChannel);
    assert(agnathosPose.rasterEyeWeight > 0.60f);
    assert(findPoseChannel(agnathosPose, PoseJoint::Neck) != nullptr);
    assert(findPoseChannel(agnathosPose, PoseJoint::Skull) != nullptr);
    assert(findPoseChannel(agnathosPose, PoseJoint::Hood) == nullptr);

    const CharacterPoseExecution saelisPose = executeCharacterPerformance(
        saelisDialogue,
        0.25f
    );
    assert(saelisPose.active);
    assert(saelisPose.rig == CharacterRigArchetype::Humanoid);
    assert(saelisPose.usesHumanFacialMuscles);
    assert(!saelisPose.usesRasterEyeChannel);
    assert(findPoseChannel(saelisPose, PoseJoint::SpineUpper) != nullptr);

    const CharacterPoseExecution reaperPose = executeCharacterPerformance(
        reaperOverload,
        0.10f
    );
    assert(reaperPose.active);
    assert(reaperPose.rig == CharacterRigArchetype::ReaperSkeleton);
    assert(reaperPose.skeletalOnly);
    assert(!reaperPose.usesHumanFacialMuscles);
    assert(!reaperPose.usesRasterEyeChannel);
    assert(reaperPose.jawOpen == 1.0f);
    assert(findPoseChannel(reaperPose, PoseJoint::Skull) != nullptr);
    assert(findPoseChannel(reaperPose, PoseJoint::Jaw) != nullptr);
    assert(findPoseChannel(reaperPose, PoseJoint::Hood) != nullptr);
    assert(findPoseChannel(reaperPose, PoseJoint::Cloak) != nullptr);

    const PoseChannel* reaperJaw = findPoseChannel(reaperPose, PoseJoint::Jaw);
    assert(reaperJaw != nullptr);
    assert(reaperJaw->pitch > 0.40f);

    // Held-frame discontinuity is deterministic and visibly different from
    // smooth humanoid performance sampling.
    const CharacterPoseExecution reaperPoseLater = executeCharacterPerformance(
        reaperOverload,
        0.20f
    );
    const PoseChannel* skullA = findPoseChannel(reaperPose, PoseJoint::Skull);
    const PoseChannel* skullB = findPoseChannel(reaperPoseLater, PoseJoint::Skull);
    assert(skullA != nullptr && skullB != nullptr);
    assert(skullA->yaw != skullB->yaw);

    hakui::GameRuntime runtime;
    assert(runtime.playerPerformanceProfile() == agnathos);
    assert(runtime.npcPerformanceProfile(hakui::NpcManager::saelisId) == saelis);
    assert(runtime.characterPerformanceProfile(reaperInstanceId) == reaper);

    const CharacterPerformanceFrame playerDialogue =
        runtime.playerPerformanceFrame(CharacterPerformanceEvent::Dialogue);
    assert(playerDialogue.characterId == CharacterId::Agnathos);
    assert(playerDialogue.expression == CharacterExpression::RasterFocused);

    const CharacterPerformanceFrame runtimeReaper =
        runtime.characterPerformanceFrame(
            reaperInstanceId,
            CharacterPerformanceEvent::CombatReady
        );
    assert(runtimeReaper.characterId == CharacterId::Reaper);
    assert(runtimeReaper.motion == CharacterMotionGrammar::ReaperStaccato);
    assert(runtimeReaper.staccatoWeight == 1.0f);

    const CharacterPoseExecution runtimePlayerPose = runtime.playerPoseExecution(
        CharacterPerformanceEvent::Dialogue,
        0.25f
    );
    assert(runtimePlayerPose.characterId == CharacterId::Agnathos);
    assert(runtimePlayerPose.rig == CharacterRigArchetype::Humanoid);

    const CharacterPoseExecution runtimeSaelisPose = runtime.npcPoseExecution(
        hakui::NpcManager::saelisId,
        CharacterPerformanceEvent::Dialogue,
        0.25f
    );
    assert(runtimeSaelisPose.characterId == CharacterId::Saelis);

    const CharacterPoseExecution runtimeReaperPose = runtime.characterPoseExecution(
        reaperInstanceId,
        CharacterPerformanceEvent::Overload,
        0.10f
    );
    assert(runtimeReaperPose.characterId == CharacterId::Reaper);
    assert(runtimeReaperPose.skeletalOnly);

    // L23: Reaper is no longer only a spawned identity/performance preview. He
    // owns an independent authoritative world actor because he is deliberately
    // not bound to PlayerRuntime or the humanoid NpcManager.
    const CharacterActorState* reaperActor =
        runtime.independentCharacterActor(CharacterId::Reaper);
    assert(reaperActor != nullptr);
    assert(reaperActor->active);
    assert(reaperActor->instanceId == reaperInstanceId);
    assert(reaperActor->characterId == CharacterId::Reaper);
    assert(reaperActor->grounded);
    assert(runtime.characterActors().size() == 1);

    const float authoredSpawnX = reaperActor->spawnX;
    const float authoredSpawnZ = reaperActor->spawnZ;
    assert(std::fabs(reaperActor->x - authoredSpawnX) < 0.0001f);
    assert(std::fabs(reaperActor->z - authoredSpawnZ) < 0.0001f);

    assert(runtime.requestIndependentCharacterWalkTo(
        CharacterId::Reaper,
        authoredSpawnX - 1.0f,
        reaperActor->spawnY,
        authoredSpawnZ
    ));
    runtime.characterActors().tick(
        runtime.player().x,
        runtime.player().y,
        runtime.player().z,
        0.10f
    );
    reaperActor = runtime.independentCharacterActor(CharacterId::Reaper);
    assert(reaperActor != nullptr);
    assert(reaperActor->x < authoredSpawnX);
    assert(reaperActor->activity == CharacterActorActivity::Walking);
    assert(reaperActor->movementBlend > 0.0f);

    // Proximity is authoritative actor state, not a renderer guess.
    runtime.player().x = reaperActor->x;
    runtime.player().y = reaperActor->y;
    runtime.player().z = reaperActor->z;
    runtime.characterActors().tick(
        runtime.player().x,
        runtime.player().y,
        runtime.player().z,
        0.10f
    );
    assert(runtime.independentCharacterInInteractionRange(CharacterId::Reaper));

    const auto published = publishedCharacterActors();
    assert(published.size() == 1);
    assert(published.front().characterId == CharacterId::Reaper);
    assert(std::fabs(published.front().x -
                     runtime.independentCharacterActor(CharacterId::Reaper)->x) < 0.0001f);

    runtime.resetSession();
    assert(runtime.playerPerformanceProfile() == agnathos);
    assert(runtime.characterPerformanceProfile(reaperInstanceId) == reaper);
    reaperActor = runtime.independentCharacterActor(CharacterId::Reaper);
    assert(reaperActor != nullptr);
    assert(std::fabs(reaperActor->x - reaperActor->spawnX) < 0.0001f);
    assert(std::fabs(reaperActor->z - reaperActor->spawnZ) < 0.0001f);

    return 0;
}
