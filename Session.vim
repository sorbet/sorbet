let SessionLoad = 1
let s:so_save = &g:so | let s:siso_save = &g:siso | setg so=0 siso=0 | setl so=-1 siso=-1
let v:this_session=expand("<sfile>:p")
silent only
silent tabonly
cd ~/stripe/sorbet
if expand('%') == '' && !&modified && line('$') <= 1 && getline(1) == ''
  let s:wipebuf = bufnr('%')
endif
set shortmess=aoO
badd +1 test/testdata/lsp/code_actions/loop_type_change.A.rbedited
badd +321 test/helpers/position_assertions.h
badd +250 test/lsp_test_runner.cc
badd +8 test/testdata/lsp/missing_typed_sigil.rb
badd +10038 term://~/stripe/sorbet//35888:/bin/zsh
badd +220 /private/var/tmp/_bazel/db4fb26c5e76a24b25e87547ff408b1e/execroot/com_stripe_ruby_typer/bazel-out/darwin-dbg/bin/main/lsp/lsp_messages_enums_gen.h
badd +1290 /private/var/tmp/_bazel/db4fb26c5e76a24b25e87547ff408b1e/execroot/com_stripe_ruby_typer/bazel-out/darwin-dbg/bin/main/lsp/lsp_messages_gen.h
badd +2457 /private/var/tmp/_bazel/db4fb26c5e76a24b25e87547ff408b1e/execroot/com_stripe_ruby_typer/bazel-out/darwin-dbg/bin/external/doctest/_virtual_includes/doctest/doctest.h
badd +1 ~/stripe/sorbet
badd +122 main/lsp/LSPMessage.h
badd +9 test/testdata/lsp/code_actions/extract_class_func.rb
badd +8 test/testdata/lsp/code_actions/extract_class_func.A.rbedited
badd +1 /private/tmp/a.rb
badd +1 /private/tmp/b.rb
badd +1520 test/helpers/position_assertions.cc
badd +88 test/helpers/lsp.h
badd +224 test/helpers/lsp.cc
badd +342 main/lsp/requests/code_action.cc
badd +28 main/lsp/AbstractRenamer.h
badd +2 main/lsp/lsp.cc
badd +24 common/common.h
badd +443 /private/var/tmp/_bazel/db4fb26c5e76a24b25e87547ff408b1e/external/com_google_absl/absl/container/flat_hash_map.h
badd +1207 /private/var/tmp/_bazel/db4fb26c5e76a24b25e87547ff408b1e/external/llvm_toolchain_12_0_0/include/c++/v1/string
badd +320 main/minimize/minimize.cc
badd +10 common/FileOps.h
badd +15 test/testdata/lsp/code_actions/extract_func_naming.rb
badd +44 NERD_tree_2
badd +13 test/testdata/lsp/code_actions/extract_func_naming.A.rbedited
badd +1 test/testdata/lsp/code_actions
badd +2 test/testdata/lsp/code_actions/loop_type_change.rb
badd +2 test/testdata/lsp/code_actions/private.A.rbedited
badd +2 test/testdata/lsp/code_actions/private.B.rbedited
badd +2 test/testdata/lsp/code_actions/private.rb
badd +2 test/testdata/lsp/code_actions/sig_missing__child.A.rbedited
badd +2 test/testdata/lsp/code_actions/sig_missing__child.rb
badd +2 test/testdata/lsp/code_actions/sig_missing__parent.A.rbedited
badd +2 test/testdata/lsp/code_actions/sig_missing__parent.rb
badd +4 test/testdata/lsp/code_actions/typo.A.rbedited
badd +4 test/testdata/lsp/code_actions/typo.B.rbedited
badd +4 test/testdata/lsp/code_actions/typo.C.rbedited
badd +4 test/testdata/lsp/code_actions/typo.D.rbedited
badd +4 test/testdata/lsp/code_actions/typo.rb
badd +8 test/testdata/lsp/missing_typed_sigil.A.rbedited
badd +0 test/testdata/compiler/Regexp.rb
badd +0 test/testdata/compiler/disabled/validate_exp_tests/test.opt.ll.exp
badd +1 test/testdata/compiler/disabled/validate_exp_tests/test.rb
badd +79 ~/stripe/pay-server/.vscode/settings.json
badd +57 ~/stripe/pay-server/scripts/bin/typecheck
badd +1 ~/stripe/pay-server/scripts/bin/typecheck_service_wrapper
badd +21 main/lsp/requests/code_action.h
badd +235 main/options/options.h
badd +174 main/lsp/LSPTypechecker.h
badd +2 test/lsp/alias-incremental/alias-incremental.rec
badd +2 test/lsp/incremental-lsp-changes/incremental-lsp-changes.rec
badd +2 test/lsp/no-trailing-newline/no-trailing-newline.rec
badd +2 test/lsp/workspaceSymbol/workspaceSymbol.rec
badd +1 main/lsp/AbstractRenamer.cc
badd +1453 main/lsp/tools/make_lsp_types.cc
badd +39 main/lsp/requests/initialize.cc
badd +7 test/lsp/update_one.sh
badd +120 gems/sorbet/test/snapshot/snapshot.bzl
badd +815 README.md
badd +19 sorbet_version/sorbet_version.h
badd +8 experimental/rubyfmt/test/rubyfmt_test.cc
badd +1 /private/var/tmp/_bazel/db4fb26c5e76a24b25e87547ff408b1e/execroot/com_stripe_ruby_typer/bazel-out/darwin-dbg/testlogs/test/lsp/test_no-trailing-newline/test.log
badd +13 test/lsp/BUILD
badd +316 main/lsp/LSPTask.cc
badd +64 main/lsp/requests/prepare_rename.cc
badd +1 vscode_extension/.vscode/tasks.json
badd +7 main/lsp/LSPPreprocessor.cc
badd +9 main/lsp/requests/command.h
badd +1 main/lsp/requests/definition.cc
badd +1 main/lsp/requests/definition.h
badd +15 main/lsp/requests/command.cc
badd +1483 /private/var/tmp/_bazel/db4fb26c5e76a24b25e87547ff408b1e/external/llvm_toolchain_12_0_0/include/c++/v1/variant
badd +7 main/lsp/json_types.h
badd +25 main/lsp/requests/requests.h
badd +1612 /private/var/tmp/_bazel/db4fb26c5e76a24b25e87547ff408b1e/external/llvm_toolchain_12_0_0/include/c++/v1/type_traits
badd +24 main/lsp/json_enums.h
badd +730 main/options/options.cc
badd +165 main/lsp/wrapper.cc
badd +1 main/lsp/requests/rename.cc
badd +55 main/lsp/wrapper.h
badd +13 test/testdata/lsp/code_actions/move_method/no_sig.A.rbedited
badd +1 test/testdata/lsp/code_actions/move_method/func_naming.rb
badd +1 test/testdata/lsp/code_actions/move_method/no_sig.rb
badd +12 main/sig_finder/sig_finder.h
badd +15 main/sig_finder/sig_finder.cc
badd +1 test/testdata/lsp/code_actions/move_method
badd +329 /private/var/tmp/_bazel/db4fb26c5e76a24b25e87547ff408b1e/external/llvm_toolchain_12_0_0/include/c++/v1/string_view
badd +104 core/Loc.h
badd +6 core/Loc.cc
badd +54 core/Files.h
badd +204 core/Files.cc
badd +0 test/testdata/lsp/code_actions/move_method/single_line_def.A.rbedited
badd +0 test/testdata/lsp/code_actions/move_method/single_line_def.rb
badd +14 test/testdata/lsp/code_actions/move_method/class_func.rb
badd +42 test/testdata/lsp/code_actions/move_method/class_func.A.rbedited
badd +9 test/testdata/lsp/code_actions/move_method/call_location.rb
badd +1 test/testdata/lsp/code_actions/move_method/func_naming.A.rbedited
badd +20 test/testdata/lsp/code_actions/move_method/call_location.A.rbedited
badd +20 test/testdata/lsp/code_actions/move_method/call_location.B.rbedited
badd +28 test/testdata/lsp/code_actions/move_method/call_location.C.rbedited
badd +27 test/testdata/lsp/code_actions/move_method/call_location.D.rbedited
badd +1630 /private/var/tmp/_bazel/db4fb26c5e76a24b25e87547ff408b1e/external/llvm_toolchain_12_0_0/include/c++/v1/memory
badd +42 test/BUILD
badd +17 test/testdata/lsp/code_actions/move_method/comments.rb
badd +1 test/testdata/lsp/code_actions/move_method/comments.A.rbedited
badd +1 test/testdata/lsp/code_actions/move_method/comments.B.rbedited
badd +152 core/lsp/QueryResponse.h
badd +2 core/lsp/QueryResponse.cc
argglobal
%argdel
$argadd ~/stripe/sorbet
edit main/lsp/requests/code_action.cc
argglobal
balt term://~/stripe/sorbet//35888:/bin/zsh
setlocal fdm=expr
setlocal fde=nvim_treesitter#foldexpr()
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=99
setlocal fml=1
setlocal fdn=20
setlocal fen
10
normal! zo
255
normal! zo
335
normal! zo
340
normal! zo
341
normal! zo
342
normal! zo
let s:l = 342 - ((28 * winheight(0) + 25) / 50)
if s:l < 1 | let s:l = 1 | endif
keepjumps exe s:l
normal! zt
keepjumps 342
normal! 042|
tabnext 1
if exists('s:wipebuf') && len(win_findbuf(s:wipebuf)) == 0&& getbufvar(s:wipebuf, '&buftype') isnot# 'terminal'
  silent exe 'bwipe ' . s:wipebuf
endif
unlet! s:wipebuf
set winheight=1 winwidth=20 shortmess=filnxtToOFc
let s:sx = expand("<sfile>:p:r")."x.vim"
if filereadable(s:sx)
  exe "source " . fnameescape(s:sx)
endif
let &g:so = s:so_save | let &g:siso = s:siso_save
set hlsearch
doautoall SessionLoadPost
unlet SessionLoad
" vim: set ft=vim :
