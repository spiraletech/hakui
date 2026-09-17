#include "character/CharacterPerformanceProfile.hpp"
#include "character/CharacterPoseExecutor.hpp"
#include "core/GameRuntime.hpp"

#include <cassert>

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

    runtime.resetSession();
    assert(runtime.playerPerformanceProfile() == agnathos);
    assert(runtime.characterPerformanceProfile(reaperInstanceId) == reaper);

    return 0;
}
