# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

# This fixture runs in a subprocess so that ObjectSpace iteration
# is isolated from the main test process.

def check(description, loc: caller_locations(1, 1).first)
  unless yield
    puts "FAIL: #{loc.path}:#{loc.lineno}: #{description}"
  end
end

class NeverMadeNilable; end

class RowsStruct < T::Struct
  prop :rows, T::Array[T::Hash[String, T::Array[Integer]]]
end

row_type = RowsStruct.decorator.props.fetch(:rows).fetch(:type_object).type
simple = T::Utils.coerce(NeverMadeNilable)
loose_type = T::Utils.coerce(T::Hash[String, T::Array[NeverMadeNilable]])

check("prop members unbuilt before") { !row_type.instance_variable_defined?(:@type) }
check("simple nilable unbuilt before") { !simple.instance_variable_defined?(:@nilable) }
check("loose type members unbuilt before") { !loose_type.instance_variable_defined?(:@keys) }

T::Utils.build_all_types

check("prop members built after") { row_type.instance_variable_defined?(:@type) }
check("nested prop members built after") { row_type.values.instance_variable_defined?(:@type) }
check("simple name built after") { simple.instance_variable_defined?(:@name) }
check("simple nilable built after") { simple.instance_variable_defined?(:@nilable) }
check("nilable members built after") { T.nilable(NeverMadeNilable).instance_variable_defined?(:@types) }
check("loose type members built after") { loose_type.instance_variable_defined?(:@keys) }
check("nested loose type members built after") { loose_type.values.instance_variable_defined?(:@type) }

# Building must not change what a type accepts.
check("struct still validates") { RowsStruct.new(rows: [{"a" => [1]}]).rows == [{"a" => [1]}] }
check("struct still rejects") do
  RowsStruct.new(rows: [{"a" => ["x"]}])
  false
rescue TypeError
  true
end

puts "PASS"
