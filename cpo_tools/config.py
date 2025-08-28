# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

from pathlib import Path
import yaml


def load_config():
    """Load configuration from multiple sources with proper precedence."""
    config = {}

    # Default configuration embedded in package
    default_config = {
        "style": {"indent": 2, "brace_style": "allman", "namespace": "cpo"},
        "verification": {"strict": False, "allowed_deviations": []},
    }
    config.update(default_config)

    # User configuration from home directory
    user_config_path = Path.home() / ".cpo-tools" / "config.yaml"
    if user_config_path.exists():
        with open(user_config_path) as f:
            config.update(yaml.safe_load(f))

    # Project configuration from current directory
    project_config_path = Path.cwd() / ".cpo-tools.yaml"
    if project_config_path.exists():
        with open(project_config_path) as f:
            config.update(yaml.safe_load(f))

    return config
