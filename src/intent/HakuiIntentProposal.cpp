#include "intent/HakuiIntentProposal.hpp"

#include <charconv>
#include <cctype>

namespace hakui::intent {
namespace {
constexpr std::string_view prefix = "HAKUI_PROPOSE ";

std::optional<std::uint64_t> number(std::string_view value)
{
    std::uint64_t result = 0;
    const auto parsed = std::from_chars(value.data(), value.data() + value.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size() || result == 0)
        return std::nullopt;
    return result;
}

std::optional<IntentVerb> verb(std::string_view value)
{
    if (value == "LookAt") return IntentVerb::LookAt;
    if (value == "WalkTo") return IntentVerb::WalkTo;
    if (value == "Sit") return IntentVerb::Sit;
    if (value == "Talk") return IntentVerb::Talk;
    if (value == "Inspect") return IntentVerb::Inspect;
    return std::nullopt;
}
}

ProposalBatch IntentProposalParser::parse(std::string_view text, std::uint64_t nextId)
{
    ProposalBatch batch;
    while (!text.empty()) {
        const std::size_t end = text.find('\n');
        std::string_view line = text.substr(0, end);
        text = end == std::string_view::npos ? std::string_view{} : text.substr(end + 1);
        if (!line.starts_with(prefix)) continue;
        if (batch.proposals.size() == maxProposals) { batch.truncated = true; break; }

        line.remove_prefix(prefix.size());
        const std::size_t vEnd = line.find(' ');
        const auto parsedVerb = verb(line.substr(0, vEnd));
        if (!parsedVerb || vEnd == std::string_view::npos) { ++batch.rejected; continue; }
        line.remove_prefix(vEnd + 1);
        const std::size_t actorEnd = line.find(' ');
        if (actorEnd == std::string_view::npos) { ++batch.rejected; continue; }
        const auto actor = number(line.substr(0, actorEnd));
        line.remove_prefix(actorEnd + 1);
        const std::size_t targetEnd = line.find(' ');
        const auto target = number(line.substr(0, targetEnd));
        std::string_view argument = targetEnd == std::string_view::npos
            ? std::string_view{} : line.substr(targetEnd + 1);
        if (!actor || !target || argument.size() > maxArgumentBytes) {
            ++batch.rejected;
            continue;
        }
        batch.proposals.push_back({nextId++, *parsedVerb, *actor, *target,
                                   std::string(argument), std::string(line)});
    }
    return batch;
}

std::optional<IntentProposal> IntentProposalParser::parsePlayerCommand(
    std::string_view text,
    std::uint64_t proposalId
)
{
    std::string normalized;
    normalized.reserve(text.size());
    bool previousSpace = true;
    for (const unsigned char character : text) {
        if (std::isalnum(character)) {
            normalized.push_back(static_cast<char>(std::tolower(character)));
            previousSpace = false;
        } else if (!previousSpace) {
            normalized.push_back(' ');
            previousSpace = true;
        }
    }
    while (!normalized.empty() && normalized.back() == ' ') normalized.pop_back();
    const bool namesSaelis = normalized.find("saelis") != std::string::npos;
    const bool asksLook = normalized.find("look at me") != std::string::npos ||
        normalized.find("face me") != std::string::npos ||
        normalized.find("look over here") != std::string::npos;
    if (!namesSaelis || !asksLook || proposalId == 0) return std::nullopt;
    return IntentProposal{proposalId, IntentVerb::LookAt, 2001, 1,
                          "player", std::string(text)};
}

std::string_view intentVerbLabel(IntentVerb value) noexcept
{
    switch (value) {
        case IntentVerb::LookAt: return "LookAt";
        case IntentVerb::WalkTo: return "WalkTo";
        case IntentVerb::Sit: return "Sit";
        case IntentVerb::Talk: return "Talk";
        case IntentVerb::Inspect: return "Inspect";
    }
    return "Inspect";
}
} // namespace hakui::intent
