# Doxygen Configuration for TInCuP CPO Tags

The Jinja2 templates for generating CPO headers use custom Doxygen tags like `@cpo`,
`@cpo_example`, and `@tag_invoke_impl`. For third‑party integrations, trait specializations
and ADL shims can optionally be documented with `@cpo_impl` and `@cpo_adl_shim`.
To make these recognized by Doxygen,
you can add the following alias definitions to your `Doxyfile`:

```ini
# Customization Point Object (CPO) tags
ALIASES += cpo="Customization Point Object"
ALIASES += cpo_example="Example usage of CPO"
ALIASES += tag_invoke_impl="Default tag_invoke overload implementation"
ALIASES += cpo_impl="CPO trait specialization (formatter-style)"
ALIASES += cpo_adl_shim="ADL shim forwarding to cpo_impl"
```

With these aliases in place, your generated headers will render correctly in
the Doxygen output without requiring further modifications.

## Grouping CPOs in the Docs

To get index pages for CPOs and trait specializations, add a simple groups file to your
Doxygen input and tag generated code accordingly:

1) Ensure `docs/DOXYGEN_GROUPS.dox` is part of `INPUT` in your Doxyfile:
```
INPUT = include docs/DOXYGEN_GROUPS.dox
```

2) Regenerate docs — Doxygen will create groups and list all headers that include
   `@ingroup tincup_cpos` (for CPOs) and `@ingroup tincup_cpo_integrations`
   (for `cpo_impl` specializations and ADL shims).

For more details on customizing Doxygen, see the official Doxygen manual:
https://www.doxygen.nl/manual/config.html#cfg_aliases
