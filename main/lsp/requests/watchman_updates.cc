#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

shared_ptr<core::File> readFile(string_view path, const FileSystem &fs) {
    try {
        auto contents = fs.readFile(path);
        return make_shared<core::File>(string(path), move(contents), core::File::Type::Normal);
    } catch (FileNotFoundException e) {
        // Act as if file is completely empty.
        return make_shared<core::File>(string(path), "", core::File::Type::Normal);
    }
}

unique_ptr<core::GlobalState> LSPLoop::handleWatchmanUpdates(unique_ptr<core::GlobalState> gs,
                                                             vector<string> changedFiles) {
    if (changedFiles.size() == 0) {
        return gs;
    }

    vector<shared_ptr<core::File>> files;
    for (auto &filePath : changedFiles) {
        // Ignore updates to ignored files.
        if (!FileOps::isFileIgnored(rootPath, filePath, opts.absoluteIgnorePatterns, opts.relativeIgnorePatterns)) {
            auto it = openFiles.find(filePath);
            if (it == openFiles.end()) {
                // File is not open in editor. Update from file system.
                files.emplace_back(readFile(filePath, *opts.fs));
            }
        }
    }
    return pushDiagnostics(tryFastPath(move(gs), files));
}
} // namespace sorbet::realmain::lsp