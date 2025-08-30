# Doxygen Configuration for TInCuP CPO Tags

The Jinja2 templates for generating CPO headers use custom Doxygen tags like `@cpo`,
`@cpo_example`, and `@tag_invoke_impl`. To make these recognized by Doxygen,
you can add the following alias definitions to your `Doxyfile`:

```ini
# Customization Point Object (CPO) tags
ALIASES += cpo="Customization Point Object"
ALIASES += cpo_example="Example usage of CPO"
ALIASES += tag_invoke_impl="Default tag_invoke overload implementation"
```

With these aliases in place, your generated headers will render correctly in
the Doxygen output without requiring further modifications.

## Grouping CPOs in the Docs

To get an index page listing all CPOs, add a simple groups file to your Doxygen input
and tag generated CPOs with `@ingroup tincup_cpos` (already present in templates):

1) Ensure `docs/DOXYGEN_GROUPS.dox` is part of `INPUT` in your Doxyfile:
```
INPUT = include docs/DOXYGEN_GROUPS.dox
```

2) Regenerate docs — Doxygen will create a group “TInCuP Customization Points” and
   list all headers that include `@ingroup tincup_cpos`.

For more details on customizing Doxygen, see the official Doxygen manual:
https://www.doxygen.nl/manual/config.html#cfg_aliases
