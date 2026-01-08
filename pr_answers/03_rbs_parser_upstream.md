## 3. Can the RBS Parser `module.patch` be contributed upstream?

The patch is minimal - it only adds a 3-line MODULE.bazel file:

```starlark
module(
    name = "rbs_parser",
)
```

This is the pattern used for all non-BCR dependencies in this migration. Upstreaming would require the RBS project to accept and maintain Bazel support. This could be proposed as a PR to https://github.com/shopify/rbs, but the overhead of maintaining it upstream may not be worthwhile for them. The patch approach is low-maintenance for both sides.
