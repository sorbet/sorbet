# sorbet-runtime

Full docs are at:

- https://sorbet.org/docs/adopting
- https://sorbet.org/docs/runtime
- https://sorbet.org/docs/tconfiguration

## Environment variables

There are a number of environment variables that `sorbet-runtime` reads from to change it's
behavior:

- `SORBET_RUNTIME_ENABLE_CHECKING_IN_TESTS`

  Announces to Sorbet that we are currently in a test environment, so it
  should treat any sigs which are marked `.checked(:tests)` as if they were
  just a normal sig.

  This can also be done by calling `T::Configuration.enable_checking_for_sigs_marked_checked_tests`
  but the environment variable ensures this value gets set before any sigs
  are evaluated.

- `SORBET_RUNTIME_DEFAULT_CHECKED_LEVEL`

  Configure the default checked level for a sig with no explicit `.checked`
  builder. When unset, the default checked level is `:always`.

  This can also be done by calling `T::Configuration.default_checked_level = ...`
  but the environment variable ensures this value gets set before any sigs
  are evaluated.

## Testing

To run the tests for sorbet-runtime locally:

```
cd gems/sorbet-runtime
bundle install
bundle exec rake test
```

To run `sorbet` on `sorbet-runtime`:

```
bazel test //gems/sorbet-runtime:runtime-typechecks
```

To test the call_validation.rb generated file tests:

```
bazel test //gems/sorbet-runtime:verify_call_validation
```

To update them:

```
bazel test //gems/sorbet-runtime:update_call_validation
```
