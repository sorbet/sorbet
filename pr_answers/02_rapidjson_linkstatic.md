## 2. What motivated the rapidjson linkstatic simplification?

The original code conditionally set `linkstatic` based on build configuration using a `select()` pattern, but this is redundant because:

- The `-fPIC` flags in `--config=shared-libs` are sufficient for shared library builds
- Having a simple `linkstatic = True` works correctly in all build modes
- The select pattern added complexity without benefit
