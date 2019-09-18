This is super shaky and could(should?) be improved.

This demoes and end-to-end flow. We should start replacing components of this with "real" things.

# How to see it in action:

```
> ruby extconf.rb
> make
> pry
  require './foobar.bundle' # for mac
  require './foobar.so'     # for linux
  DemoModule.return_nil
```

# How to see matching LLVM IR

```
> ruby extconf.rb
> make foobar.ll
> cat foobar.ll
```
