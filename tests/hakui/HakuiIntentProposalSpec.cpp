#include "intent/HakuiIntentProposal.hpp"
#include <cassert>

int main()
{
    using namespace hakui::intent;
    const ProposalBatch batch = IntentProposalParser::parse(
        "hello\nHAKUI_PROPOSE LookAt 2001 1 player\n"
        "HAKUI_PROPOSE Sit 2001 100202 void_couch_right\n"
        "HAKUI_PROPOSE DeleteWorld 2001 1 nope\n", 40);
    assert(batch.proposals.size() == 2);
    assert(batch.rejected == 1);
    assert(batch.proposals[0].id == 40);
    assert(batch.proposals[0].verb == IntentVerb::LookAt);
    assert(batch.proposals[1].targetId == 100202);
    assert(intentVerbLabel(batch.proposals[1].verb) == "Sit");

    const ProposalBatch invalid = IntentProposalParser::parse(
        "HAKUI_PROPOSE Talk 0 1 no_actor\nHAKUI_PROPOSE WalkTo 2001 0 no_target");
    assert(invalid.proposals.empty());
    assert(invalid.rejected == 2);
    return 0;
}
