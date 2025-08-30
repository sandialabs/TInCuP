"""
" autoload/cpo.vim
"""
function! cpo#Generate(args, doxygen)
  " Debug: show what we received
  echo "Raw args received: " . string(a:args)
  
  let l:arg_list = []
  let l:remaining_args = a:args
  " This regex pattern matches either a single-quoted string or a sequence of non-whitespace characters.
  let l:pattern = '\v\s*(''[^'']*''|\S+)'

  while !empty(l:remaining_args)
      let l:match_text = matchstr(l:remaining_args, l:pattern)
      if empty(l:match_text)
          break " Safeguard against infinite loops
      endif
      
      " Get the actual content, removing leading/trailing whitespace and the outer quotes.
      let l:cleaned = trim(l:match_text)
      " Remove outer single quotes if they exist
      if l:cleaned =~# "^'.*'$"
        let l:arg = l:cleaned[1:-2]  " Remove first and last character
      else
        let l:arg = l:cleaned
      endif
      call add(l:arg_list, l:arg)
      
      " Advance the string past the matched argument.
      let l:remaining_args = strpart(l:remaining_args, len(l:match_text))
  endwhile

  if empty(l:arg_list)
    echohl ErrorMsg
    echo "Error: No arguments provided."
    echohl None
    return
  endif

  let l:cpo_name = l:arg_list[0]
  let l:cpo_args = l:arg_list[1:]

  " Debug: show parsed arguments
  echo "Parsed cpo_name: " . l:cpo_name
  echo "Parsed args: " . string(l:cpo_args)

  let l:json_dict = {
        \ "cpo_name": l:cpo_name,
        \ "args": l:cpo_args
        \ }

  let l:json_string = json_encode(l:json_dict)

  let l:command = '/home/greg/venv/bin/cpo-generator '
  if a:doxygen
    let l:command .= '--doxygen '
  endif
  let l:command .= shellescape(l:json_string)

  echo "Executing command:" l:command
  let l:output = system(l:command)

  if v:shell_error != 0
    echohl ErrorMsg
    echo "Error generating CPO:"
    echo l:output
    echohl None
    return
  endif

  call append(line('.'), split(l:output, "\n"))
endfunction