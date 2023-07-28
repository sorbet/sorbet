#ifndef CORE_SHOW_OPTIONS_H
#define CORE_SHOW_OPTIONS_H

namespace sorbet::core {

// Options for controlling how the various `show` methods work.
struct ShowOptions final {
    bool showForRBI : 1;
    bool concretizeIfAbstract : 1;

    ShowOptions() : showForRBI{false}, concretizeIfAbstract{false} {}

    // Show types for printing out to an rbi file.
    ShowOptions withShowForRBI() {
        ShowOptions res{*this};
        res.showForRBI = true;
        return res;
    }

    // Replace the `abstract` flag with `override`
    ShowOptions withConcretizeIfAbstract() {
        ShowOptions res{*this};
        res.concretizeIfAbstract = true;
        return res;
    }
};

} // namespace sorbet::core

#endif
