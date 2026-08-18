# typed: true

T.assert_type!(
  {
    hi: "there",
  },
  {
    hi: String,
  }
)

T.assert_type!(
  {
    hi: "there",
  }.merge({
            llamas: 17
          }),
  {
    hi: String,
    llamas: Integer,
  }
)

T.assert_type!(
  {
    hi: "there",
    llamas: 8,
  }.merge({
            llamas: :none,
            alpacas: 5
          }),
  {
    hi: String,
    llamas: Symbol,
    alpacas: Integer,
  }
)

T.assert_type!(
  {
    hi: "there",
  }.merge!({
             llamas: 17
           }),
  {
    hi: String,
    llamas: Integer,
  }
)

Before = T.type_alias do
  {
    "int" => Integer,
    "changed" => String,
  }
end

After = T.type_alias do
  {
    "int" => Integer,
    "changed" => Symbol,
    "new_int" => Integer,
    "new_nil_str" => T.nilable(String),
  }
end

extend T::Sig

sig {params(to_transform: Before).returns(After)}
def transform_type_alias(to_transform)
  to_transform.merge!(
    "changed" => :new_value,
    "new_int" => 1,
    "new_nil_str" => nil,
  )
end

def has_kwargs(a:, b:, c:)
end

has_kwargs({}.merge(a: 1).merge(b: 3).merge(c: 4))


class Key; end

def has_opts_hash(opts={}); end

key = Key.new

# Exercise non-symbol values being used as keyword args
has_opts_hash(**{a: 10}.merge(key => "value"))

has_opts_hash(key => "value")
