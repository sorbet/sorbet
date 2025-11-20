#ifndef SORBET_PARSER_PRISM_FACTORY_H
#define SORBET_PARSER_PRISM_FACTORY_H

#include "common/common.h"
#include "core/LocOffsets.h"
#include <string_view>
#include <vector>
extern "C" {
#include "prism.h"
}

namespace sorbet::parser::Prism {

// Forward declarations
class Parser;

class Factory {
private:
    Parser &parser;

public:
    Factory(Parser &parser) : parser(parser) {}

    template <typename T> T *allocateNode() const {
        return new T{};
    }

    pm_node_t initializeBaseNode(pm_node_type_t type, const pm_location_t loc) const;

    // Basic node creators
    pm_node_t *ConstantReadNode(std::string_view name, core::LocOffsets loc) const;
    pm_node_t *ConstantReadNode(pm_constant_id_t constantId, core::LocOffsets loc) const;
    pm_node_t *ConstantWriteNode(core::LocOffsets loc, pm_constant_id_t nameId, pm_node_t *value) const;
    pm_node_t *ConstantPathNode(core::LocOffsets loc, pm_node_t *parent, std::string_view name) const;
    pm_node_t *SingleArgumentNode(pm_node_t *arg) const;
    pm_node_t *Self(core::LocOffsets loc) const;
    pm_node_t *True(core::LocOffsets loc) const;

    // Symbol and hash node creators
    pm_node_t *Symbol(core::LocOffsets nameLoc, std::string_view name) const;
    pm_node_t *SymbolFromConstant(core::LocOffsets nameLoc, pm_constant_id_t nameId) const;
    pm_node_t *AssocNode(core::LocOffsets loc, pm_node_t *key, pm_node_t *value) const;
    pm_node_t *Hash(core::LocOffsets loc, const std::vector<pm_node_t *> &pairs) const;
    pm_node_t *KeywordHash(core::LocOffsets loc, const std::vector<pm_node_t *> &pairs) const;

    // Low-level method call creation
    pm_call_node_t *createSendNode(pm_node_t *receiver, pm_constant_id_t method_id, pm_node_t *arguments,
                                   pm_location_t message_loc, pm_location_t full_loc, pm_location_t tiny_loc,
                                   pm_node_t *block = nullptr) const;

    // High-level method call builders (similar to ast::MK)
    pm_node_t *Send(core::LocOffsets loc, pm_node_t *receiver, std::string_view method,
                    const std::vector<pm_node_t *> &args, pm_node_t *block = nullptr) const;
    pm_node_t *Send0(core::LocOffsets loc, pm_node_t *receiver, std::string_view method) const;
    pm_node_t *Send1(core::LocOffsets loc, pm_node_t *receiver, std::string_view method, pm_node_t *arg1) const;
    pm_node_t *Send2(core::LocOffsets loc, pm_node_t *receiver, std::string_view method, pm_node_t *arg1,
                     pm_node_t *arg2) const;

    // Utility functions
    pm_constant_id_t addConstantToPool(std::string_view name) const;

    // High-level node creators
    pm_node_t *SorbetPrivateStatic(core::LocOffsets loc) const;
    pm_node_t *TSigWithoutRuntime(core::LocOffsets loc) const;

    // T constant and method helpers
    pm_node_t *T(core::LocOffsets loc) const;
    pm_node_t *THelpers(core::LocOffsets loc) const;
    pm_node_t *TUntyped(core::LocOffsets loc) const;
    pm_node_t *TNilable(core::LocOffsets loc, pm_node_t *type) const;
    pm_node_t *TAny(core::LocOffsets loc, const std::vector<pm_node_t *> &args) const;
    pm_node_t *TAll(core::LocOffsets loc, const std::vector<pm_node_t *> &args) const;
    pm_node_t *TTypeParameter(core::LocOffsets loc, pm_node_t *name) const;
    pm_node_t *TProc(core::LocOffsets loc, pm_node_t *args, pm_node_t *returnType) const;
    pm_node_t *TProcVoid(core::LocOffsets loc, pm_node_t *args) const;
    pm_node_t *TLet(core::LocOffsets loc, pm_node_t *value, pm_node_t *type) const;
    pm_node_t *TCast(core::LocOffsets loc, pm_node_t *value, pm_node_t *type) const;
    pm_node_t *TMust(core::LocOffsets loc, pm_node_t *value) const;
    pm_node_t *TUnsafe(core::LocOffsets loc, pm_node_t *value) const;
    pm_node_t *TAbsurd(core::LocOffsets loc, pm_node_t *value) const;
    pm_node_t *TBindSelf(core::LocOffsets loc, pm_node_t *type) const;
    pm_node_t *TTypeAlias(core::LocOffsets loc, pm_node_t *type) const;
    pm_node_t *T_Array(core::LocOffsets loc) const;
    pm_node_t *T_Class(core::LocOffsets loc) const;
    pm_node_t *T_Enumerable(core::LocOffsets loc) const;
    pm_node_t *T_Enumerator(core::LocOffsets loc) const;
    pm_node_t *T_Hash(core::LocOffsets loc) const;
    pm_node_t *T_Set(core::LocOffsets loc) const;
    pm_node_t *T_Range(core::LocOffsets loc) const;

    pm_node_t *Array(core::LocOffsets loc, const std::vector<pm_node_t *> &elements) const;
    pm_node_t *Block(core::LocOffsets loc, pm_node_t *body) const;
    pm_node_t *StatementsNode(core::LocOffsets loc, const std::vector<pm_node_t *> &body) const;

    // Wrappers around Prism's allocator functions, which raise `std::bad_alloc` instead of returning `nullptr`.
    // Use these to allocate memory that will be owned by Prism.
    void *malloc(size_t size) const;
    void *calloc(size_t count, size_t size) const;
    void *realloc(void *ptr, size_t size) const;
    void free(void *ptr) const;

private:
    pm_node_list_t copyNodesToList(const std::vector<pm_node_t *> &nodes) const;
    pm_arguments_node_t *createArgumentsNode(std::vector<pm_node_t *> args, const pm_location_t loc) const;
};

} // namespace sorbet::parser::Prism

#endif // SORBET_PARSER_PRISM_FACTORY_H
