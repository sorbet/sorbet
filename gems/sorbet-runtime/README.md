# sorbet-runtime

Sorbet's runtime type system.

### Installation

Add to your gemfile:
```ruby
gem 'sorbet-runtime', :git => 'git://github.com/stripe/sorbet.git', glob: 'runtime/*.gemspec'
```

Run bundler:
```
bundler install
```

### Configuration

By default, `sig`s and the `T.cast` method family will raise a `TypeError` if method calls don't comply with the defined method signature.

To override this functionality with custom error handling, you can configure the runtime type system as follows:

- `T::Configuration.type_error_handler` can be set to catch `TypeError`s raised from `cast`s, `T.must`, and sig errors. Example:
```ruby
T::Configuration.type_error_handler = lambda do |error|
  puts error.message
end
```

- `T::Configuration.sig_decl_error_handler` can be set to catch sig declaration errors. Example:
```ruby
T::Configuration.sig_decl_error_handler = lambda do |error, location|
  puts error.message
end
```

- `T::Configuration.sig_build_error_handler` can be set to catch sig build errors. Example:
```ruby
T::Configuration.sig_build_error_handler = lambda do |error, opts|
  puts error.message
end
```

- `T::Configuration.call_validation_error_handler` can be set to catch sig validation errors. Example:
```ruby
T::Configuration.call_validation_error_handler = lambda do |signature, opts|
  puts opts[:message]
end
```

See `lib/types/configuration.rb` for a full description of the arguments passed to these lambdas.
