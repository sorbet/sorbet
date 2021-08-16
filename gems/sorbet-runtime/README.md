# sorbet-runtime

Full docs are at:

- https://sorbet.org/docs/adopting
- https://sorbet.org/docs/runtime
- https://sorbet.org/docs/tconfiguration

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
