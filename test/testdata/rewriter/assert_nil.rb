# typed: true

$global_var = T.let("string", T.nilable(String))

class TestAssertNil
  extend T::Sig

  @@class_var = T.let("string", T.nilable(String))

  sig { void }
  def initialize
    @instance_var = T.let("string", T.nilable(String))
  end

  sig { params(value: T.nilable(String)).void }
  def test_assert_nil_local(value)
    T.reveal_type(value) # error: Revealed type: `T.nilable(String)`
    assert_nil(value)
    T.reveal_type(value) # error: Revealed type: `NilClass`
  end

  sig { params(value: T.nilable(String)).void }
  def test_assert_not_nil_local(value)
    T.reveal_type(value) # error: Revealed type: `T.nilable(String)`
    assert_not_nil(value)
    T.reveal_type(value) # error: Revealed type: `String`
  end

  sig { params(value: T.nilable(String)).void }
  def test_refute_nil_local(value)
    T.reveal_type(value) # error: Revealed type: `T.nilable(String)`
    refute_nil(value)
    T.reveal_type(value) # error: Revealed type: `String`
  end

  sig { void }
  def test_assert_nil_instance_variable
    T.reveal_type(@instance_var) # error: Revealed type: `T.nilable(String)`
    assert_nil(@instance_var)
    T.reveal_type(@instance_var) # error: Revealed type: `NilClass`
  end

  sig { void }
  def test_assert_not_nil_instance_variable
    T.reveal_type(@instance_var) # error: Revealed type: `T.nilable(String)`
    assert_not_nil(@instance_var)
    T.reveal_type(@instance_var) # error: Revealed type: `String`
  end

  sig { void }
  def test_refute_nil_instance_variable
    T.reveal_type(@instance_var) # error: Revealed type: `T.nilable(String)`
    refute_nil(@instance_var)
    T.reveal_type(@instance_var) # error: Revealed type: `String`
  end

  sig { void }
  def test_assert_nil_class_variable
    T.reveal_type(@@class_var) # error: Revealed type: `T.nilable(String)`
    assert_nil(@@class_var)
    T.reveal_type(@@class_var) # error: Revealed type: `NilClass`
  end

  sig { void }
  def test_assert_not_nil_class_variable
    T.reveal_type(@@class_var) # error: Revealed type: `T.nilable(String)`
    assert_not_nil(@@class_var)
    T.reveal_type(@@class_var) # error: Revealed type: `String`
  end

  sig { void }
  def test_refute_nil_class_variable
    T.reveal_type(@@class_var) # error: Revealed type: `T.nilable(String)`
    refute_nil(@@class_var)
    T.reveal_type(@@class_var) # error: Revealed type: `String`
  end

  sig { params(value: T.nilable(String)).void }
  def test_assert_nil_message(value)
    T.reveal_type(value) # error: Revealed type: `T.nilable(String)`
    assert_nil(value)
    T.reveal_type(value) # error: Revealed type: `NilClass`
  end

  sig { params(value: T.nilable(String)).void }
  def test_assert_not_nil_message(value)
    T.reveal_type(value) # error: Revealed type: `T.nilable(String)`
    assert_not_nil(value)
    T.reveal_type(value) # error: Revealed type: `String`
  end

  sig { params(value: T.nilable(String)).void }
  def test_refute_nil_message(value)
    T.reveal_type(value) # error: Revealed type: `T.nilable(String)`
    refute_nil(value)
    T.reveal_type(value) # error: Revealed type: `String`
  end

  sig { void }
  def test_no_args
    assert_nil()
    assert_not_nil()
    refute_nil()
  end

  sig { params(value: T.nilable(String)).void }
  def test_too_many_args(value)
    T.reveal_type(value) # error: Revealed type: `T.nilable(String)`
    assert_nil(value, "message", "other")
    assert_not_nil(value, "message", "other")
    refute_nil(value, "message", "other")
    T.reveal_type(value) # error: Revealed type: `T.nilable(String)`
  end

  sig { void }
  def test_global_variable
    T.reveal_type($global_var) # error: Revealed type: `T.untyped`
    assert_nil($global_var)
    assert_not_nil($global_var)
    refute_nil($global_var)
    T.reveal_type($global_var) # error: Revealed type: `T.untyped`
  end

  sig { void }
  def test_non_assignable
    assert_nil("string")
    assert_not_nil("string")
    refute_nil("string")
  end

  sig { params(args: T.anything).void }
  def assert_nil(*args); end

  sig { params(args: T.anything).void }
  def assert_not_nil(*args); end

  sig { params(args: T.anything).void }
  def refute_nil(*args); end
end
