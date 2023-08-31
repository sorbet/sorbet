-- This config is hacky and only meant as a quick and dirty way to test Sorbet in Neovim
-- If you actually want to integrate Sorbet LSP in Neovim, see the nvim-lspconfig project.

vim.cmd.colorscheme('slate')
vim.g.mapleader = vim.api.nvim_replace_termcodes('<Space>', false, false, true)
vim.o.number = true
vim.o.signcolumn = 'yes'

vim.api.nvim_create_autocmd('LspAttach', {
  group = vim.api.nvim_create_augroup('UserLspConfig', {}),
  callback = function(ev)
    -- Enable completion triggered by <c-x><c-o>
    vim.bo[ev.buf].omnifunc = 'v:lua.vim.lsp.omnifunc'

    -- Buffer local mappings.
    -- See `:help vim.lsp.*` for documentation on any of the below functions
    local opts = { buffer = ev.buf }
    vim.keymap.set('n', 'gD', vim.lsp.buf.declaration, opts)
    vim.keymap.set('n', 'gd', vim.lsp.buf.definition, opts)
    vim.keymap.set('n', 'K', vim.lsp.buf.hover, opts)
    vim.keymap.set('n', '<leader>t', vim.lsp.buf.type_definition, opts)
    vim.keymap.set('n', '<leader>rn', vim.lsp.buf.rename, opts)
    vim.keymap.set({ 'n', 'v' }, '<leader>.', vim.lsp.buf.code_action, opts)
    vim.keymap.set('n', '<leader>r', vim.lsp.buf.references, opts)
    vim.keymap.set('n', '<leader>e', vim.diagnostic.open_float, opts)
  end,
})

vim.lsp.start_client({
  name = 'sorbet',
  cmd = {
    '../../../bazel-bin/main/sorbet',
    '--lsp',
    '--debug-log-file=sorbet-nvim.log',
    '--disable-watchman',
    '--dir=.',
  },
  root_dir = vim.fn.expand('%:p:h'),
})

vim.api.nvim_create_autocmd('FileType', {
  pattern = {'ruby'},
  callback = function()
    vim.lsp.buf_attach_client(0, 1)
  end
})
