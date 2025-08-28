# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

from jinja2 import Environment


def _analyze_target(target: str):
    """Parse target type string for template pack placeholder '...'.
    Returns (template_params, specialized_target).
    Example: 'Kokkos::View<...>' -> ('template<class... P>', 'Kokkos::View<P...>')
             'MyType' -> ('', 'MyType')
    """
    target = target.strip()
    if '...' in target:
        # Single pack variable name 'P'
        tmpl = 'template<class... P>'
        specialized = target.replace('...', 'P...')
        return tmpl, specialized
    return '', target


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
