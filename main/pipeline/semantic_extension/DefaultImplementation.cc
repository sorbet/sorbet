#include "main/pipeline/semantic_extension/SemanticExtension.h"

using namespace std;
namespace sorbet::pipeline::semantic_extension {

vector<SemanticExtensionProvider *> SemanticExtensionProvider::getProviders() {
    return {};
}
} // namespace sorbet::pipeline::semantic_extension
