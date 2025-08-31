# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

from jinja2 import Environment
from .trait_impl import _analyze_target


def render_adl_shim(env: Environment, cpo_name: str, target: str, shim_namespace: str | None) -> str:
    tmpl_params, specialized_target = _analyze_target(target)
    template = env.get_template('trait_adl_shim.hpp.jinja2')
    return template.render({
        'cpo_name': cpo_name,
        'tmpl_params': tmpl_params,
        'target_type': specialized_target,
        'shim_namespace': shim_namespace,
    })

