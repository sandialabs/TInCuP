# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

"""
Thin wrapper to run the enhanced CPO verifier with CLI compatibility.
"""
from .cpo_verification_enhanced import main as _enhanced_main


def main():
    _enhanced_main()


if __name__ == "__main__":
    main()
