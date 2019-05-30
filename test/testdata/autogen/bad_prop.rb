# typed: true

# the error below happens because this is an autogen test, which means
# we don't run the DSL pass that would remove `optional`
class A
  optional :plan, String # error: Method `optional` does not exist
end
