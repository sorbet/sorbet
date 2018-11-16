#ifndef MAKE_LSP_TYPES_H
#define MAKE_LSP_TYPES_H

#include "main/lsp/tools/generate_lsp_messages.h"

#include <vector>

void makeLSPTypes(std::vector<std::shared_ptr<JSONClassType>> &enumTypes,
                  std::vector<std::shared_ptr<JSONObjectType>> &classTypes);

#endif // MAKE_LSP_TYPES_H