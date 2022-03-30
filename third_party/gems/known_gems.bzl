# If gem is missing here, we'll still be able to build it.
# the known downsides of not including gems here are:
#  - bazel will print the "sha" for unknown gems when running(that's how these were collected)
#  - bazel will sometimes redownload them(as it uses those sha's for invalidation)
KNOWN_GEM_SHA265 = {
    "diff-lcs-1.3": "ea7bf591567e391ef262a7c29edaf87c6205204afb5bb39dfa8f08f2e51282a3",
    "rspec-3.8.0": "83f519611bb674d456e87397fea7c5b15b1af8bdc77ce929673ae3b4b656f796",
    "rspec-core-3.8.0": "97d0b30c5687075417ac6f837c44a95e4a825007d0017fccec7a5cbcec2a3adc",
    "rspec-expectations-3.8.3": "de192f5c3f9f2916857eaee42964b062bcfdd1b822d5063506751eb77cd18fb0",
    "rspec-mocks-3.8.0": "d73b926d641676025ba086b4854f70b8a70d6cb763d50e9d278b792c1902c51b",
    "rspec-support-3.8.0": "0918cc4165bb7626e518cef41046ddab90d0435868b0fb85dc90e61e733b755c",
    "cantor-1.2.1": "f9c2c3d2ff23f07908990a891d4d4d53e6ad157f3fe8194ce06332fa4037d8bb",
    "stupidedi-1.4.0": "be7a05ecdff462598d10950dde3994e3688ef18ec31fd0c5dbf8ae504f3f42e0",
    "term-ansicolor-1.7.1": "92339ffec77c4bddc786a29385c91601dd52fc68feda23609bba0491229b05f7",
    "tins-1.20.2": "b2f6e9247e590228bc39d7a1fe713c284058a99d6db5c9fcf9f99b206c2112df",
    "addressable-2.8.0": "f76d29d2d1f54b6c6a49aec58f9583b08d97e088c227a3fcba92f6c6531d5908",
    "crack-0.4.5": "798416fb29b8c9f655d139d5559169b39c4a0a3b8f8f39b7f670eec1af9b21b3",
    "hashdiff-1.0.1": "2cd4d04f5080314ecc8403c4e2e00dbaa282dff395e2d031bc16c8d501bdd6db",
    "public_suffix-4.0.6": "a99967c7b2d1d2eb00e1142e60de06a1a6471e82af574b330e9af375e87c0cf7",
    "rexml-3.2.5": "a33c3bf95fda7983ec7f05054f3a985af41dbc25a0339843bd2479e93cabb123",
    "webmock-3.14.0": "b317a4f6fa7ebcc1e6e2cba29f01fccc12a5e9aa4365b037c34ca4072e821bd1",
    "sorbet-runtime-0.5.6353": "edd643460c7c16ddd5b3df227fb90ff8f6fe536535cd2e9f9e1e6626421c0b49",
}

def get_known_gem_sha256(gem_name, default):
    return KNOWN_GEM_SHA265.get(gem_name, default)
