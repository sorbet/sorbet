# typed: __STDLIB_INTERNAL

# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) Ain't Markup Language
#
# This module provides a Ruby interface for data serialization in
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) format.
#
# The [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) module is an alias
# of [`Psych`](https://docs.ruby-lang.org/en/2.7.0/Psych.html), the
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) engine for Ruby.
#
# ## Usage
#
# Working with [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) can be
# very simple, for example:
#
# ```ruby
# require 'yaml'
# # Parse a YAML string
# YAML.load("--- foo") #=> "foo"
#
# # Emit some YAML
# YAML.dump("foo")     # => "--- foo\n...\n"
# { :a => 'b'}.to_yaml  # => "---\n:a: b\n"
# ```
#
# As the implementation is provided by the
# [`Psych`](https://docs.ruby-lang.org/en/2.7.0/Psych.html) library, detailed
# documentation can be found in that library's docs (also part of standard
# library).
#
# ## Security
#
# Do not use [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) to load
# untrusted data. Doing so is unsafe and could allow malicious input to execute
# arbitrary code inside your application. Please see doc/security.rdoc for more
# information.
#
# ## History
#
# Syck was the original for
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html) implementation in
# Ruby's standard library developed by why the lucky stiff.
#
# You can still use Syck, if you prefer, for parsing and emitting
# [`YAML`](https://docs.ruby-lang.org/en/2.7.0/YAML.html), but you must install
# the 'syck' gem now in order to use it.
#
# In older Ruby versions, ie. <= 1.9, Syck is still provided, however it was
# completely removed with the release of Ruby 2.0.0.
#
# ## More info
#
# For more advanced details on the implementation see
# [`Psych`](https://docs.ruby-lang.org/en/2.7.0/Psych.html), and also check out
# http://yaml.org for spec details and other helpful information.
#
# [`Psych`](https://docs.ruby-lang.org/en/2.7.0/Psych.html) is maintained by
# Aaron Patterson on github: https://github.com/ruby/psych
#
# Syck can also be found on github: https://github.com/ruby/syck
YAML = Psych # :nodoc:

# YAML Ain't Markup Language
#
# This module provides a Ruby interface for data serialization in YAML format.
#
# The YAML module is an alias of Psych, the YAML engine for Ruby.
#
# == Usage
#
# Working with YAML can be very simple, for example:
#
#     require 'yaml'
#     # Parse a YAML string
#     YAML.load("--- foo") #=> "foo"
#
#     # Emit some YAML
#     YAML.dump("foo")     # => "--- foo\n...\n"
#     { :a => 'b'}.to_yaml  # => "---\n:a: b\n"
#
# As the implementation is provided by the Psych library, detailed documentation
# can be found in that library's docs (also part of standard library).
#
# == Security
#
# Do not use YAML to load untrusted data. Doing so is unsafe and could allow
# malicious input to execute arbitrary code inside your application. Please see
# doc/security.rdoc for more information.
#
# == History
#
# Syck was the original for YAML implementation in Ruby's standard library
# developed by why the lucky stiff.
#
# You can still use Syck, if you prefer, for parsing and emitting YAML, but you
# must install the 'syck' gem now in order to use it.
#
# In older Ruby versions, ie. <= 1.9, Syck is still provided, however it was
# completely removed with the release of Ruby 2.0.0.
#
# == More info
#
# For more advanced details on the implementation see Psych, and also check out
# http://yaml.org for spec details and other helpful information.
#
# Psych is maintained by Aaron Patterson on github: https://github.com/ruby/psych
#
# Syck can also be found on github: https://github.com/ruby/syck
