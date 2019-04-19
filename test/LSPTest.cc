#include "test/LSPTest.h"

#include <signal.h>

#include "main/lsp/json_types.h"
#include "payload/payload.h"

namespace sorbet::test {
using namespace std;

void LSPTest::SetUp() {
    test = GetParam();
    lspWrapper = make_unique<LSPWrapper>("", fastpathDisabled);
    lspWrapper->enableAllExperimentalFeatures();
    parseTestFile();
}

bool LSPTest::fastpathDisabled = false;

void LSPTest::parseTestFile() {
    for (auto &sourceFile : test.sourceFiles) {
        filenames.insert(test.folder + sourceFile);
    }

    assertions = RangeAssertion::parseAssertions(test.sourceFileContents);

    if (test.expectations.find("autogen") != test.expectations.end()) {
        // When autogen is enabled, skip DSL passes...
        lspWrapper->opts.skipDSLPasses = true;
        // Some autogen tests assume that some errors will occur from the resolver step, others assume the resolver
        // won't run.
        if (RangeAssertion::getErrorAssertions(assertions).size() > 0) {
            // ...and stop after the resolver phase if there are errors
            lspWrapper->opts.stopAfterPhase = realmain::options::Phase::RESOLVER;
        } else {
            // ...and stop after the namer phase if there are no errors
            lspWrapper->opts.stopAfterPhase = realmain::options::Phase::NAMER;
        }
    }
}

} // namespace sorbet::test
