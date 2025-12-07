#!/usr/bin/env python3
# TInCuP - Bijective Transformation: CLI Interface
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.

"""
Command-line interface for bijective transformation.

Provides commands for:
- from-custom: Transform custom functions to CPO code
- to-custom: Transform CPO code back to custom (requires libclang)
- validate: Validate round-trip transformation
"""

import click
from pathlib import Path
import sys

from .transformer import transform_custom_to_cpo
from .custom_parser import parse_custom_file

# Lazy import for reverse transformation (requires libclang)
def _get_reverse_transformer():
    try:
        from .reverse_transformer import transform_cpo_to_custom
        return transform_cpo_to_custom
    except ImportError:
        return None


@click.group()
@click.version_option()
def cli():
    """
    TInCuP Bijective Transformer

    Transform between C++ `custom` functions and CPO+tag_invoke code.

    \b
    Examples:
        # Transform custom to CPO
        cpo-bidirectional from-custom input.hpp --output cpos.hpp

        # Parse and show what would be generated
        cpo-bidirectional from-custom input.hpp --dry-run

        # Transform without metadata
        cpo-bidirectional from-custom input.hpp --no-metadata
    """
    pass


@cli.command('from-custom')
@click.argument('input_file', type=click.Path(exists=True, path_type=Path))
@click.option('-o', '--output', type=click.Path(path_type=Path),
              help='Output file path. If not specified, prints to stdout.')
@click.option('--no-metadata', is_flag=True,
              help='Disable bijective transformation metadata')
@click.option('--dry-run', is_flag=True,
              help='Parse and show functions found, but do not generate')
@click.option('--verbose', '-v', is_flag=True,
              help='Verbose output')
def from_custom(input_file, output, no_metadata, dry_run, verbose):
    """
    Transform custom function declarations to CPO code.

    Reads INPUT_FILE containing `custom` function declarations and generates
    TInCuP CPO + tag_invoke boilerplate code.

    \b
    Example:
        cpo-bidirectional from-custom my_lib.hpp -o my_lib_cpo.hpp
    """
    try:
        if verbose or dry_run:
            click.echo(f"Parsing {input_file}...")

        # Parse custom functions
        functions = parse_custom_file(str(input_file))

        if not functions:
            click.echo(f"Warning: No custom functions found in {input_file}", err=True)
            sys.exit(1)

        if verbose or dry_run:
            click.echo(f"\nFound {len(functions)} custom function(s):")
            for i, func in enumerate(functions, 1):
                click.echo(f"  {i}. {func.function_name}")
                if verbose:
                    click.echo(f"     Return: {func.return_type}")
                    click.echo(f"     Params: {len(func.parameters)}")
                    if func.template_params:
                        click.echo(f"     Template: {func.template_params}")
                    if func.is_noexcept:
                        click.echo(f"     Noexcept: yes")

        if dry_run:
            click.echo("\n(Dry run - no output generated)")
            return

        # Transform
        if verbose:
            click.echo(f"\nGenerating CPO code...")

        result = transform_custom_to_cpo(
            str(input_file),
            str(output) if output else None,
            enable_metadata=not no_metadata
        )

        # Output
        if output:
            click.echo(f"✓ Generated CPO code: {output}", err=True)
        else:
            # Print to stdout
            click.echo(result)

    except Exception as e:
        click.echo(f"Error: {e}", err=True)
        if verbose:
            import traceback
            traceback.print_exc()
        sys.exit(1)


@cli.command('to-custom')
@click.argument('input_file', type=click.Path(exists=True, path_type=Path))
@click.option('-o', '--output', type=click.Path(path_type=Path),
              help='Output file path. If not specified, prints to stdout.')
@click.option('-I', '--include', 'include_paths', multiple=True,
              help='Include directories for parsing (e.g., -I./include)')
@click.option('-v', '--verbose', is_flag=True,
              help='Verbose output')
def to_custom(input_file, output, include_paths, verbose):
    """
    Transform CPO code back to custom declarations.

    Requires libclang: pip install TInCuP[reverse]

    \b
    Example:
        cpo-bidirectional to-custom my_lib_cpo.hpp -o recovered.hpp
        cpo-bidirectional to-custom my_lib_cpo.hpp -I./include -o recovered.hpp
    """
    try:
        # Check if libclang is available
        transform_func = _get_reverse_transformer()
        if transform_func is None:
            click.echo("Error: Reverse transformation requires libclang.", err=True)
            click.echo("Install with: pip install libclang", err=True)
            click.echo("Or: pip install TInCuP[reverse]", err=True)
            sys.exit(1)

        if verbose:
            click.echo(f"Parsing CPO file: {input_file}", err=True)
            if include_paths:
                click.echo(f"Include paths: {', '.join(include_paths)}", err=True)

        # Transform
        result = transform_func(
            str(input_file),
            str(output) if output else None,
            include_paths=list(include_paths)
        )

        # Output
        if output:
            click.echo(f"✓ Generated custom declarations: {output}", err=True)
        else:
            # Print to stdout
            click.echo(result)

    except Exception as e:
        click.echo(f"Error: {e}", err=True)
        if verbose:
            import traceback
            traceback.print_exc()
        sys.exit(1)


@cli.command('validate')
@click.argument('custom_file', type=click.Path(exists=True))
@click.argument('cpo_file', type=click.Path(exists=True))
def validate(custom_file, cpo_file):
    """
    Validate round-trip transformation (EXPERIMENTAL).

    Tests that: custom → CPO → custom produces identical result.

    \b
    Example:
        cpo-bidirectional validate original.hpp generated_cpo.hpp
    """
    click.echo("Error: Validation not yet implemented.", err=True)
    click.echo("Coming soon! Track progress at: https://github.com/sandialabs/TInCuP/issues", err=True)
    sys.exit(1)


@cli.command('parse')
@click.argument('input_file', type=click.Path(exists=True, path_type=Path))
@click.option('--json', is_flag=True, help='Output as JSON')
def parse(input_file, json):
    """
    Parse and display custom function declarations (debugging utility).

    Shows what the parser extracted from the input file.

    \b
    Example:
        cpo-bidirectional parse my_lib.hpp
    """
    try:
        functions = parse_custom_file(str(input_file))

        if not functions:
            click.echo(f"No custom functions found in {input_file}")
            return

        if json:
            import json as json_module
            specs = [func.to_tincup_spec() for func in functions]
            click.echo(json_module.dumps(specs, indent=2))
        else:
            click.echo(f"Found {len(functions)} custom function(s) in {input_file}:\n")

            for i, func in enumerate(functions, 1):
                click.echo(f"{i}. {func.function_name}")
                click.echo(f"   Line: {func.line_number}")
                click.echo(f"   Return type: {func.return_type}")

                if func.template_params:
                    click.echo(f"   Template params: {', '.join(func.template_params)}")

                if func.parameters:
                    click.echo(f"   Parameters:")
                    for p in func.parameters:
                        click.echo(f"     - {p.type_str} {p.name}")
                else:
                    click.echo(f"   Parameters: (none)")

                if func.is_noexcept:
                    click.echo(f"   Noexcept: yes")

                if func.namespace:
                    click.echo(f"   Namespace: {func.namespace}")

                click.echo(f"   TInCuP spec: {func.to_tincup_spec()}")
                click.echo()

    except Exception as e:
        click.echo(f"Error: {e}", err=True)
        sys.exit(1)


def main():
    """Entry point for cpo-bidirectional command."""
    cli()


if __name__ == '__main__':
    main()
