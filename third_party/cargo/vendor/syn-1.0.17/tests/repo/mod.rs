mod progress;

use self::progress::Progress;
use crate::common;
use anyhow::Result;
use flate2::read::GzDecoder;
use std::fs;
use std::path::Path;
use tar::Archive;
use walkdir::DirEntry;

const REVISION: &str = "2c462a2f776b899d46743b1b44eda976e846e61d";

pub fn base_dir_filter(entry: &DirEntry) -> bool {
    let path = entry.path();
    if path.is_dir() {
        return true; // otherwise walkdir does not visit the files
    }
    if path.extension().map(|e| e != "rs").unwrap_or(true) {
        return false;
    }

    let mut path_string = path.to_string_lossy();
    if cfg!(windows) {
        path_string = path_string.replace('\\', "/").into();
    }
    assert!(path_string.starts_with("tests/rust/src/"));
    let path = &path_string["tests/rust/src/".len()..];

    // TODO assert that parsing fails on the parse-fail cases
    if path.starts_with("test/parse-fail")
        || path.starts_with("test/compile-fail")
        || path.starts_with("test/rustfix")
    {
        return false;
    }

    if path.starts_with("test/ui") {
        let stderr_path = entry.path().with_extension("stderr");
        if stderr_path.exists() {
            // Expected to fail in some way
            return false;
        }
    }

    match path {
        // TODO: or-patterns patterns: `Some(1 | 8)`
        // https://github.com/dtolnay/syn/issues/758
        "test/mir-opt/exponential-or.rs" |
        "test/ui/or-patterns/basic-switch.rs" |
        "test/ui/or-patterns/basic-switchint.rs" |
        "test/ui/or-patterns/bindings-runpass-1.rs" |
        "test/ui/or-patterns/bindings-runpass-2.rs" |
        "test/ui/or-patterns/consistent-bindings.rs" |
        "test/ui/or-patterns/exhaustiveness-pass.rs" |
        "test/ui/or-patterns/for-loop.rs" |
        "test/ui/or-patterns/if-let-while-let.rs" |
        "test/ui/or-patterns/issue-67514-irrefutable-param.rs" |
        "test/ui/or-patterns/issue-68785-irrefutable-param-with-at.rs" |
        "test/ui/or-patterns/let-pattern.rs" |
        "test/ui/or-patterns/mix-with-wild.rs" |
        "test/ui/or-patterns/or-patterns-default-binding-modes.rs" |
        "test/ui/or-patterns/or-patterns-syntactic-pass.rs" |
        "test/ui/or-patterns/search-via-bindings.rs" |
        "test/ui/or-patterns/struct-like.rs" |

        // TODO: inner attr in traits: `trait Foo { #![...] }`
        // https://github.com/dtolnay/syn/issues/759
        "test/pretty/trait-inner-attr.rs" |
        "test/ui/parser/inner-attr-in-trait-def.rs" |

        // TODO: const underscore in traits: `trait A { const _: (); }`
        // https://github.com/dtolnay/syn/issues/760
        "test/ui/parser/assoc-const-underscore-syntactic-pass.rs" |

        // TODO: top level fn without body: `fn f();`
        // https://github.com/dtolnay/syn/issues/761
        "test/ui/parser/fn-body-optional-syntactic-pass.rs" |
        "test/ui/parser/fn-header-syntactic-pass.rs" |

        // TODO: extern static with value: `extern { static X: u8 = 0; }`
        // https://github.com/dtolnay/syn/issues/762
        "test/ui/parser/foreign-static-syntactic-pass.rs" |

        // TODO: extern type with bound: `extern { type A: Ord; }`
        // https://github.com/dtolnay/syn/issues/763
        "test/ui/parser/foreign-ty-syntactic-pass.rs" |

        // TODO: top level const/static without value: `const X: u8;`
        // https://github.com/dtolnay/syn/issues/764
        "test/ui/parser/item-free-const-no-body-syntactic-pass.rs" |
        "test/ui/parser/item-free-static-no-body-syntactic-pass.rs" |

        // TODO: mut receiver in fn pointer type: `fn(mut self)`
        // https://github.com/dtolnay/syn/issues/765
        "test/ui/parser/self-param-syntactic-pass.rs" |

        // TODO: const trait impls and bounds
        // https://github.com/dtolnay/syn/issues/766
        // https://github.com/dtolnay/syn/issues/767
        "test/ui/rfc-2632-const-trait-impl/assoc-type.rs" |
        "test/ui/rfc-2632-const-trait-impl/call-const-trait-method-pass.rs" |
        "test/ui/rfc-2632-const-trait-impl/const-trait-bound-opt-out/feature-gate.rs" |
        "test/ui/rfc-2632-const-trait-impl/const-trait-bound-opt-out/syntax.rs" |
        "test/ui/rfc-2632-const-trait-impl/feature-gate.rs" |
        "test/ui/rfc-2632-const-trait-impl/generic-bound.rs" |
        "test/ui/rfc-2632-const-trait-impl/syntax.rs" |

        // Deprecated placement syntax
        "test/ui/obsolete-in-place/bad.rs" |

        // Deprecated anonymous parameter syntax in traits
        "test/ui/error-codes/e0119/auxiliary/issue-23563-a.rs" |
        "test/ui/issues/issue-13105.rs" |
        "test/ui/issues/issue-13775.rs" |
        "test/ui/issues/issue-34074.rs" |

        // 2015-style dyn that libsyntax rejects
        "test/ui/dyn-keyword/dyn-2015-no-warnings-without-lints.rs" |

        // not actually test cases
        "test/rustdoc-ui/test-compile-fail2.rs" |
        "test/rustdoc-ui/test-compile-fail3.rs" |
        "test/ui/include-single-expr-helper.rs" |
        "test/ui/include-single-expr-helper-1.rs" |
        "test/ui/issues/auxiliary/issue-21146-inc.rs" |
        "test/ui/json-bom-plus-crlf-multifile-aux.rs" |
        "test/ui/macros/auxiliary/macro-comma-support.rs" |
        "test/ui/macros/auxiliary/macro-include-items-expr.rs" => false,

        _ => true,
    }
}

pub fn clone_rust() {
    let needs_clone = match fs::read_to_string("tests/rust/COMMIT") {
        Err(_) => true,
        Ok(contents) => contents.trim() != REVISION,
    };
    if needs_clone {
        download_and_unpack().unwrap();
    }
}

fn download_and_unpack() -> Result<()> {
    let url = format!(
        "https://github.com/rust-lang/rust/archive/{}.tar.gz",
        REVISION
    );
    let response = reqwest::blocking::get(&url)?.error_for_status()?;
    let progress = Progress::new(response);
    let decoder = GzDecoder::new(progress);
    let mut archive = Archive::new(decoder);
    let prefix = format!("rust-{}", REVISION);

    let tests_rust = Path::new("tests/rust");
    if tests_rust.exists() {
        fs::remove_dir_all(tests_rust)?;
    }

    for entry in archive.entries()? {
        let mut entry = entry?;
        let path = entry.path()?;
        if path == Path::new("pax_global_header") {
            continue;
        }
        let relative = path.strip_prefix(&prefix)?;
        let out = tests_rust.join(relative);
        entry.unpack(&out)?;
        if common::travis_ci() {
            // Something about this makes the travis build not deadlock...
            errorf!(".");
        }
    }

    fs::write("tests/rust/COMMIT", REVISION)?;
    Ok(())
}
