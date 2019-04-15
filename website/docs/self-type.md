---
id: self-type
title: T.self_type
---

> TODO: This page is still a fragment. Contributions welcome!

```ruby
T.self_type
```

This type can be used in return types to indicate that calling this method on a
subclass will return the same class as the receiver (the receiver is the thing
we call a method on i.e., x in `x.foo`). For instance, `#dup` returns
`T.self_type`. No matter what class you call it on, you will get back the same
type.
