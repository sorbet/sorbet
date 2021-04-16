# If gem is missing here, we'll still be able to build it.
# the known downsides of not including gems here are:
#  - bazel will print the "sha" for unknown gems when running(that's how these were collected)
#  - bazel will sometimes redownload them(as it uses those sha's for invalidation)
KNOWN_GEM_SHA265 = {
    "colorize-0.8.1": "0ba0c2a58232f9b706dc30621ea6aa6468eeea120eb6f1ccc400105b90c4798c",
    "docile-1.3.2": "2e3eb481209795084eaf3a8b55e35991048abc02cba9363afd88fefe948f3c63",
    "json-2.3.1": "3f9ebb42fcd46ec3ecad16c89c7b174dc539bdd353610c39c15aecca1d570e95",
    "simplecov-0.19.0": "b9228393465c2b69468d30f949331aa36cd99cbd3f0fc64a39d3603a731708ac",
    "simplecov-html-0.12.2": "c9ae10b01a765452c9a50bedd3e8a20ee0f1793d9850f8c89e884b7b5c25ebec",
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
    "addressable-2.6.0": "d490ad06dfc421503e659a12597d6bb0273b5cd7ff2789a1ec27210b1914952d",
    "crack-0.4.3": "5318ba8cd9cf7e0b5feb38948048503ba4b1fdc1b6ff30a39f0a00feb6036b29",
    "hashdiff-0.3.9": "9d788399acc8e0a9937c9647f1c099f1c44e42e92718cd3f486c44eb542251e2",
    "public_suffix-3.0.3": "d4f4addffbd1ad3e7b5bb2e258a761ccef5670c23c29b0476b2299bcca220623",
    "safe_yaml-1.0.5": "a6ac2d64b7eb027bdeeca1851fe7e7af0d668e133e8a88066a0c6f7087d9f848",
    "webmock-3.5.1": "b73fca6fbc3622a2a63e6a17075b07a95876ee8187f0911688361fdcb560e03d",
    "sorbet-runtime-0.5.6353": "edd643460c7c16ddd5b3df227fb90ff8f6fe536535cd2e9f9e1e6626421c0b49",
}

def get_known_gem_sha256(gem_name, default):
    return KNOWN_GEM_SHA265.get(gem_name, default)
