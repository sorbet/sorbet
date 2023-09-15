#ifndef CORE_SHOW_OPTIONS_H
#define CORE_SHOW_OPTIONS_H

namespace sorbet::core {

// Options for controlling how the various `show` methods work.
struct ShowOptions final {
    bool useValidSyntax : 1;
    bool concretizeIfAbstract : 1;
    bool forceSelfPrefix : 1;

    ShowOptions() : useValidSyntax{false}, concretizeIfAbstract{false}, forceSelfPrefix{false} {}

    // Only generate generate or suggest syntactically valid code.
    ShowOptions withUseValidSyntax() {
        ShowOptions res{*this};
        res.useValidSyntax = true;
        return res;
    }

    // Replace the `abstract` flag with `override`
    ShowOptions withConcretizeIfAbstract() {
        ShowOptions res{*this};
        res.concretizeIfAbstract = true;
        return res;
    }

    // Always use `self.` for the prefix of a method definition.
    ShowOptions withForceSelfPrefix() {
        ShowOptions res{*this};
        res.forceSelfPrefix = true;
        return res;
    }
};

} // namespace sorbet::core

#endif
