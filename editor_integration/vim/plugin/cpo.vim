"""
" plugin/cpo.vim
"""

if exists('g:loaded_cpo_plugin')
  finish
endif
let g:loaded_cpo_plugin = 1

command! -nargs=+ -complete=customlist,cpo#Complete CPO call cpo#Generate(<q-args>, 0)
command! -nargs=+ -complete=customlist,cpo#Complete CPOD call cpo#Generate(<q-args>, 1)

