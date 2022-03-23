#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "core/core.h"

#include "main/sig_finder/sig_finder.h"

using namespace std;

namespace sorbet::sig_finder {

std::optional<SigLoc> findSignature(const core::GlobalState &gs, const core::SymbolRef &methodDef) {
    auto defLoc = methodDef.loc(gs);
    // We don't have a direct link to the signature for the method, so we
    // heuristically look for the matching signature.
    string_view source = defLoc.file().data(gs).source();
    core::LocOffsets sigLocOffsets;
    core::LocOffsets bodyLocOffsets;

    // We only care about everything prior to the definition.
    auto line_end = defLoc.beginPos();
    auto method_def_line = source.rfind('\n', line_end);
    auto prev_line_end = source.rfind('\n', method_def_line - 1);
    while (prev_line_end != source.npos) {
        auto line_start = prev_line_end + 1;
        auto line = source.substr(line_start, line_end - line_start);
        line = absl::StripAsciiWhitespace(line);
        // We do skip empty lines between signature and definition
        // Also we are starting from just before the method definition,
        // the beginning of that line could look like an empty line, and
        // we want to move to the previous line in that case.
        if (line.empty()) {
            line_end = prev_line_end;
            prev_line_end = source.rfind('\n', line_end - 1);
            continue;
        }

        const auto singleLineSig = "sig"sv;
        const uint32_t singleLineSigSize = singleLineSig.size();
        const auto alreadyFinalSig = "sig(:final)"sv;
        const auto multiLineSigEnd = "end"sv;
        const uint32_t multiLineSigEndSize = multiLineSigEnd.size();
        const auto multiLineSigStart = "sig do"sv;
        const auto commentLineStart = "#"sv;

        // If something went wrong, we might find ourselves looking at an
        // existing `sig(:final)`.  Bail instead of suggesting an autocorrect
        // to an already final sig.
        if (absl::StartsWith(line, alreadyFinalSig)) {
            break;
        }

        if (absl::StartsWith(line, singleLineSig)) {
            uint32_t offset = line.data() - source.data();
            sigLocOffsets = core::LocOffsets{offset, offset + singleLineSigSize};
            bodyLocOffsets =
                core::LocOffsets{offset + singleLineSigSize + 1, static_cast<uint32_t>(offset + line.size())};
            break;
        }

        if (absl::StartsWith(line, multiLineSigEnd)) {
            // Loop backwards searching for the matching `sig do`.  We're
            // going to:
            // - Find it;
            // - Hit an `end` or an already-final sig;
            // - Hit the start of the file.
            line_end = prev_line_end;
            prev_line_end = source.rfind('\n', line_end - 1);
            auto multiLineSigEndPos = line.data() - source.data() + multiLineSigEndSize;
            while (prev_line_end != source.npos) {
                auto line_start = prev_line_end + 1;
                line = source.substr(line_start, line_end - line_start);
                line = absl::StripAsciiWhitespace(line);

                // Found the start of the multi-line sig we were looking for.
                if (absl::StartsWith(line, multiLineSigStart)) {
                    uint32_t offset = line.data() - source.data();
                    sigLocOffsets = core::LocOffsets{offset, offset + singleLineSigSize};
                    bodyLocOffsets =
                        core::LocOffsets{offset + singleLineSigSize + 1, static_cast<uint32_t>(multiLineSigEndPos)};
                    break;
                }

                if (absl::StartsWith(line, alreadyFinalSig)) {
                    break;
                }

                // We hit an `end` in a place we didn't expect.  Bail.
                if (absl::StartsWith(line, multiLineSigEnd)) {
                    break;
                }

                line_end = prev_line_end;
                prev_line_end = source.rfind('\n', line_end - 1);
            }

            // However we exited the above loop, we are done.
            break;
        }

        // We found a comment between a sig and a method def, look one line higher
        if (absl::StartsWith(line, commentLineStart)) {
            line_end = prev_line_end;
            prev_line_end = source.rfind('\n', line_end - 1);
            continue;
        }

        // If we find anything else assume that the heuristic
        // has failed.
        break;
    }
    if (sigLocOffsets.exists() && bodyLocOffsets.exists()) {
        return SigLoc{sigLocOffsets, bodyLocOffsets};
    } else {
        return std::nullopt;
    }
}
} // namespace sorbet::sig_finder
