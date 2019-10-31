#include "test/LSPTest.h"

#include <csignal>

#include "main/lsp/json_types.h"
#include "payload/payload.h"

namespace sorbet::test {
using namespace std;

void LSPTest::SetUp() {
    test = GetParam();
    parseTestFile();

    realmain::options::Options opts;
    opts.noStdlib = BooleanPropertyAssertion::getValue("no-stdlib", assertions).value_or(false);
    lspWrapper = make_unique<LSPWrapper>(move(opts), "", fastpathDisabled);
    lspWrapper->enableAllExperimentalFeatures();

    if (test.expectations.find("autogen") != test.expectations.end()) {
        // When autogen is enabled, skip Rewriter passes...
        lspWrapper->opts.skipRewriterPasses = true;
        // Some autogen tests assume that some errors will occur from the resolver step, others assume the resolver
        // won't run.
        if (!RangeAssertion::getErrorAssertions(assertions).empty()) {
            // ...and stop after the resolver phase if there are errors
            lspWrapper->opts.stopAfterPhase = realmain::options::Phase::RESOLVER;
        } else {
            // ...and stop after the namer phase if there are no errors
            lspWrapper->opts.stopAfterPhase = realmain::options::Phase::NAMER;
        }
    }
}

bool LSPTest::fastpathDisabled = false;

void LSPTest::parseTestFile() {
    for (auto &sourceFile : test.sourceFiles) {
        filenames.insert(test.folder + sourceFile);
    }

    assertions = RangeAssertion::parseAssertions(test.sourceFileContents);
}

} // namespace sorbet::test
