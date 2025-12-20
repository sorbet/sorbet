# Ruby intrinsic analyzer

## Adding more intrinsics

To add more intrinsics, edit the whitelists at the top of wrap-intrinsics.rb.
You will need:

- To know the name of the underlying Ruby class & method
- To know that Sorbet has a SymbolRef with the same name as the Ruby class name

## Running

To re-generate the whitelisted intrinsics:

```
(cd compiler/IREmitter/Intrinsics/ && make)
```

To regenate all intrinsics:

```
# Edit the method_whitelisted? method in wrap-intrinsics.rb to return `true`

# then:
(cd compiler/IREmitter/Intrinsics/ && make)
```

## Debugging

- Look at `intrinsics-report.md`, which serializes the entirety of the
  information the script was working from.

- Half of the script is powered by running `nm` on an object to list it's
  symbols. Sometimes that output can have changed unexpectedly.

- Sometimes the generated diffs can be hard to read. They're usually better when
  passing the `--patience` flag.

