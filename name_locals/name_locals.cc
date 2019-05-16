#include "name_locals.h"
#include "ast/treemap/treemap.h"

using namespace std;

namespace sorbet::name_locals {


class LocalNameInserter {

public:


};


unique_ptr<ast::Expression> NameLocals::run(core::MutableContext ctx, unique_ptr<ast::Expression> tree) {
    LocalNameInserter localNameInserter;
    return ast::TreeMap::apply(ctx, localNameInserter, move(tree));
}

} // namespace sorbet::namer::local
