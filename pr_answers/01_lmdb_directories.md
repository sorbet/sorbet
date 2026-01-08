## 1. Why are there two separate lmdb directories (`third_party/lmdb` vs `third_party/lmdb_override`)?

This separation is intentional and follows a consistent pattern throughout the codebase:

- `third_party/lmdb/` - Contains the pre-existing `strdup.patch` that was already used before the Bzlmod migration. It only exports this patch file (`exports_files(["strdup.patch"])`).

- `third_party/lmdb_override/` - Contains the new Bzlmod-specific files: `lmdb.BUILD` (the cc_library definition) and `module.patch` (adds a minimal MODULE.bazel to the upstream archive).

This pattern preserves the original patch location for git history clarity while keeping Bzlmod scaffolding in `_override` directories. The `archive_override` in MODULE.bazel applies both patches from their respective locations.
