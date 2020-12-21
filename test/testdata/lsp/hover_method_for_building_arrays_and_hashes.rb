# typed: true
class Issue1777
  extend T::Sig

  def array_hover
    T.reveal_type([ # error: [String("foo"), Integer(123)] (2-tuple)
      #           ^ hover: [String("foo"), Integer(123)] (2-tuple)

      # Test hovering over first item in array
      "foo",
      #   ^ hover: String("foo")
      #    ^ hover: String("foo")
      #     ^ hover: [String("foo"), Integer(123)] (2-tuple)

      # Test hovering over comment
      # ^ hover: [String("foo"), Integer(123)] (2-tuple)

      # Test hovering over last item in array
      123
      # ^ hover: Integer(123)
      #  ^ hover: Integer(123)
      #   ^ hover: [String("foo"), Integer(123)] (2-tuple)
      ])
    # ^ hover: [String("foo"), Integer(123)] (2-tuple)
  end

  def hash_hover
    T.reveal_type({ # error: {foo: String("foo"), bar: Integer(123)}
      #           ^ hover: {foo: String("foo"), bar: Integer(123)} (shape of T::Hash[Symbol, T.any(String, Integer)])
      # Test hovering over first item in array
      foo: "foo",
      #        ^ hover: String("foo")
      #         ^ hover: String("foo")
      #          ^ hover: {foo: String("foo"), bar: Integer(123)} (shape of T::Hash[Symbol, T.any(String, Integer)])

      # Test hovering over comment
      # ^ hover: {foo: String("foo"), bar: Integer(123)} (shape of T::Hash[Symbol, T.any(String, Integer)])

      # Test hovering over last item in array
      bar: 123
      #      ^ hover: Integer(123)
      #       ^ hover: Integer(123)
      #        ^ hover: {foo: String("foo"), bar: Integer(123)} (shape of T::Hash[Symbol, T.any(String, Integer)])
      })
    # ^ hover: {foo: String("foo"), bar: Integer(123)} (shape of T::Hash[Symbol, T.any(String, Integer)])
  end
end
