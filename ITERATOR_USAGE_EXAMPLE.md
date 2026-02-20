# UnresolvedConstantLit Iterator Usage

## API Overview

The `UnresolvedConstantLit` class now provides iterator support for traversing constant path segments. Instead of manually walking the linked list, you create a range object and iterate over it.

## Usage

### Forward Iteration (Root to Leaf: A, B, C)

```cpp
auto* node = ast::cast_tree<ast::UnresolvedConstantLit>(expr);

// Create the range object (materializes segments once)
for (auto [name, loc] : node->parts()) {
    // name is core::NameRef
    // loc is core::LocOffsets
    std::cout << name.showRaw(gs) << "\n";
}
```

For `A::B::C`, this yields: `A`, then `B`, then `C`

### Reverse Iteration (Leaf to Root: C, B, A)

```cpp
auto* node = ast::cast_tree<ast::UnresolvedConstantLit>(expr);

// Create the range object (no materialization, walks linked list)
for (auto [name, loc] : node->partsReverse()) {
    std::cout << name.showRaw(gs) << "\n";
}
```

For `A::B::C`, this yields: `C`, then `B`, then `A`

### Getting Root Scope

```cpp
auto* node = ast::cast_tree<ast::UnresolvedConstantLit>(expr);

// Get the outermost non-UnresolvedConstantLit scope
const ExpressionPtr& root = node->rootScope();

if (ast::isa_tree<ast::EmptyTree>(root)) {
    // Absolute path from root
} else if (auto* constLit = ast::cast_tree<ast::ConstantLit>(root)) {
    // Scoped under a resolved constant
}
```

## Performance Characteristics

### ForwardRange (`parts()`)
- Materializes all segments into a vector on construction
- Iteration is fast (just indexing into vector)
- Use when: You need root-to-leaf order, or will iterate multiple times

### ReverseRange (`partsReverse()`)
- No materialization, walks linked list directly
- Iteration follows pointers through the chain
- Use when: You need leaf-to-root order (matches linked list structure)

## Migration from Direct Access

### Before (Manual Walking)
```cpp
auto* current = node;
while (current) {
    auto name = current->cnst;
    auto loc = current->loc;
    // ... use name and loc ...

    if (auto* next = ast::cast_tree<ast::UnresolvedConstantLit>(current->scope)) {
        current = next;
    } else {
        break;
    }
}
```

### After (Using Iterators)
```cpp
for (auto [name, loc] : node->partsReverse()) {
    // ... use name and loc ...
}
```

### Before (Building Vector)
```cpp
std::vector<core::NameRef> names;
auto* current = node;
while (current) {
    names.push_back(current->cnst);
    if (auto* next = ast::cast_tree<ast::UnresolvedConstantLit>(current->scope)) {
        current = next;
    } else {
        break;
    }
}
std::reverse(names.begin(), names.end());
```

### After (Using Iterators)
```cpp
std::vector<core::NameRef> names;
for (auto [name, loc] : node->parts()) {
    names.push_back(name);
}
```

## Design Rationale

The API requires creating a range object first (via `parts()` or `partsReverse()`) rather than providing `begin()`/`end()` directly on the node. This design:

1. **Avoids redundant materialization**: Calling `begin()` and `end()` separately would materialize the vector twice for forward iteration.

2. **Makes cost explicit**: Creating `ForwardRange` makes it clear that segments are being collected, while `ReverseRange` makes it clear that it's just walking the list.

3. **Enables future optimizations**: The range object can cache or compute on demand without affecting callers.

## Mutable Iteration (Modifying Fields)

### Using `partsMutable()` to mutate fields

```cpp
auto* node = ast::cast_tree<ast::UnresolvedConstantLit>(expr);

// Create mutable range object (walks linked list, provides mutable access)
for (auto segment : node->partsMutable()) {
    // segment.cnst is a mutable reference to the node's cnst field
    // segment.loc is a mutable reference to the node's loc field
    segment.cnst = subst.substituteSymbolName(segment.cnst);
    // Mutations are reflected in the actual nodes
}
```

For `A::B::C`, this yields mutable segments in leaf-to-root order: `C`, then `B`, then `A`

### Why only reverse (leaf-to-root) mutation?

- `ForwardRange` materializes segments into a vector (copy of data)
- Can't provide mutable access to original nodes (no nodes to mutate)
- Only `ReverseRange` walks actual nodes, so only it can be mutable
- `MutableReverseRange` = `ReverseRange` with mutable field access

### Example: Name Substitution

```cpp
// Before (manual traversal):
ExpressionPtr *cur = &node;
while (true) {
    auto constLit = cast_tree<UnresolvedConstantLit>(*cur);
    if (constLit == nullptr) break;
    constLit->cnst = subst.substituteSymbolName(constLit->cnst);
    cur = &constLit->scope;
}

// After (using partsMutable):
auto* node = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
for (auto segment : node->partsMutable()) {
    segment.cnst = subst.substituteSymbolName(segment.cnst);
}
```

## Example: Partially Resolved Constants

For `ResolvedClass::A::B` where `ResolvedClass` is already resolved:

```cpp
auto* node = /* UnresolvedConstantLit for A::B */;

// Iterate only the unresolved parts
for (auto [name, loc] : node->parts()) {
    // Yields: A, then B
}

// Mutate only the unresolved parts
for (auto segment : node->partsMutable()) {
    // Yields mutable B, then mutable A
    segment.cnst = transform(segment.cnst);
}

// Get the resolved base
const ExpressionPtr& root = node->rootScope();
auto* constLit = ast::cast_tree<ast::ConstantLit>(root);
// constLit represents ResolvedClass
```
