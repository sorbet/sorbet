# typed: true
class Issue1777
  extend T::Sig

  def array_hover
    T.reveal_type([ # error: [String("foo"), Integer(123)] (2-tuple)
      #           ^ hover: (nothing)

      # Test hovering over first item in array
      "foo",
      #   ^ hover: String("foo")
      #    ^ hover: String("foo")
      #     ^ hover: (nothing)

      # Test hovering over comment
      # ^ hover: (nothing)

      # Test hovering over last item in array
      123
      # ^ hover: Integer(123)
      #  ^ hover: Integer(123)
      ])
    # ^ hover: (nothing)
  end

  def hash_hover
    T.reveal_type({ # error: {foo: String("foo"), bar: Integer(123)} (shape of T::Hash[T.untyped, T.untyped])
      #           ^ hover: (nothing)
      # Test hovering over first item in array
      foo: "foo",
      #        ^ hover: String("foo")
      #         ^ hover: String("foo")
      #          ^ hover: (nothing)

      # Test hovering over comment
      # ^ hover: (nothing)

      # Test hovering over last item in array
      bar: 123
      #      ^ hover: Integer(123)
      #       ^ hover: Integer(123)
      })
    # ^ hover: (nothing)
  end
end
