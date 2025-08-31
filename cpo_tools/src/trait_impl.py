# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

from jinja2 import Environment
import re


def _analyze_target(target: str):
    """Parse target type string for generics and packs.
    Returns (template_params, specialized_target).

    Rules for declaring generics in --impl-target:
      - Use '$Name' to declare a type parameter 'Name' (e.g., 'Kokkos::View<$T>')
      - Use '$Name...' to declare a type parameter pack (e.g., '$Rest...')
      - Use '...' to declare an anonymous pack 'P...' (legacy)
      - Bare identifiers without '$' are treated as concrete tokens (e.g., 'double')

    This mirrors the shorthand used for CPO argument generics and prevents
    accidentally turning concrete types like 'double' into template parameters.
    """
    s = target.strip()
    m = re.search(r"<(.+?)>", s)
    if not m:
        # No angle-bracket args; still allow anonymous '...'
        if '...' in s:
            return 'template<class... P>', s.replace('...', 'P...')
        return '', s

    args_str = m.group(1)
    tokens = [tok.strip() for tok in args_str.split(',')]

    template_params: list[str] = []
    new_parts: list[str] = []

    def add_param(name: str, is_pack: bool):
        decl = f"typename{'...' if is_pack else ''} {name}"
        if decl not in template_params:
            template_params.append(decl)

    for tok in tokens:
        if tok == '':
            continue
        # Anonymous pack
        if tok == '...':
            add_param('P', True)
            new_parts.append('P...')
            continue
        # Named generics require '$' prefix
        if tok.startswith('$'):
            base = tok[1:]
            if base.endswith('...'):
                name = base[:-3]
                if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name):
                    # Fallback: treat as concrete if not an identifier
                    new_parts.append(base)
                else:
                    add_param(name, True)
                    new_parts.append(f"{name}...")
            else:
                name = base
                if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name):
                    new_parts.append(base)
                else:
                    add_param(name, False)
                    new_parts.append(name)
            continue
        # Named pack without '$' is ambiguous; force explicit '$'
        if re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*\s*\.\.\.", tok):
            raise ValueError(
                f"Invalid impl-target token '{tok}': use '${tok}' to declare a template parameter pack"
            )
        # Otherwise, treat as concrete token (could be nested or builtin)
        new_parts.append(tok)

    tmpl = ''
    if template_params:
        tmpl = 'template<' + ', '.join(template_params) + '>'

    # Rebuild with rewritten angle content
    def _sub_once(match):
        return '<' + ', '.join(new_parts) + '>'

    specialized = re.sub(r"<(.+?)>", _sub_once, s, count=1)
    return tmpl, specialized


def render_trait_impl(env: Environment, cpo_name: str, target: str, ctx: dict) -> str:
    """Render a tincup::cpo_impl<CPO, Target> specialization skeleton.

    Emits a template specialization that forwards to a static call(...) with
    Args pack, so users don't need perfect argument reconstruction.
    """
    tmpl_params, specialized_target = _analyze_target(target)

    template = env.get_template('trait_impl.hpp.jinja2')
    return template.render({
        'cpo_name': cpo_name,
        'tmpl_params': tmpl_params,
        'target_type': specialized_target,
    })
