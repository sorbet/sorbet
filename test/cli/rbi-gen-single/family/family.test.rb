# typed: true

module Test::Family

  class TestFamily < Test::Util::Testing::TestCase
    extend T::Sig

    sig {params(fam: Family::Flanders).void}
    def test_flanders(fam)
    end

    sig {params(fam: Family::Simpsons).void}
    def test_simpsons(fam)
    end

    # Avoid leaking Krabappel through the interface, but still rely on it to
    # typecheck this test file successfully.
    sig {returns(Object)}
    def test_krabappel
      Family::Krabappel.new
    end
  end

end
