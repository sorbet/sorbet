#ifndef RUBY_TYPER_LSP_LSPINDEXEDFILESTORE_H
#define RUBY_TYPER_LSP_LSPINDEXEDFILESTORE_H

#include "ast/ast.h"

namespace sorbet::realmain::lsp {
class LSPIndexedFileStore final {
private:
    std::vector<ast::ParsedFile> indexed;

public:
    ast::ParsedFile &getIndexedFileMutable(size_t id) {
        return indexed[id];
    }

    const ast::ParsedFile &getIndexedFile(size_t id) const {
        return indexed[id];
    }

    void putIndexedFile(ast::ParsedFile &&f) {
        const int id = f.file.id();
        if (id >= indexed.size()) {
            indexed.resize(id + 1);
        }
        indexed[f.file.id()] = std::move(f);
    }

    void overwrite(std::vector<ast::ParsedFile> &&newIndexed) {
        indexed = move(newIndexed);
    }

    size_t getSize() const {
        return indexed.size();
    }
};
} // namespace sorbet::realmain::lsp
#endif
