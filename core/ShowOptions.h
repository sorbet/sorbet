#ifndef CORE_SHOW_OPTIONS_H
#define CORE_SHOW_OPTIONS_H

namespace sorbet::core {

// Options for controlling how the various `show` methods work.
struct ShowOptions final {
    bool showForRBI : 1;
    bool showForSigSuggestion : 1;

    ShowOptions() : showForRBI{false}, showForSigSuggestion{false} {}

    // Show types for printing out to an rbi file.
    ShowOptions withShowForRBI() {
        ShowOptions res{*this};
        res.showForRBI = true;
        return res;
    }

    ShowOptions withShowForSigSuggestion() {
        ShowOptions res{*this};
        res.showForSigSuggestion = true;
        return res;
    }
};

} // namespace sorbet::core

#endif
