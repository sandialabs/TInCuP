#!/usr/bin/env python3
"""
Tests for custom → CPO transformer.
"""

import pytest
import tempfile
from pathlib import Path

from cpo_tools.bijective.transformer import CustomToCPOTransformer, transform_custom_to_cpo
from cpo_tools.bijective.custom_parser import CustomFunction, Parameter


class TestTransformerBasics:
    """Test basic transformer functionality."""

    @pytest.fixture
    def transformer(self):
        return CustomToCPOTransformer(enable_metadata=True)

    def test_simple_transformation(self, transformer):
        """Test transforming a simple custom function."""
        func = CustomFunction(
            function_name="foo",
            return_type="void",
            parameters=[]
        )

        result = transformer.transform([func])

        # Check for key components
        assert "foo_ftor" in result
        assert "TINCUP_CPO_TAG" in result
        assert 'TINCUP_CPO_TAG("foo")' in result
        assert "tag_invoke" in result

    def test_metadata_generation(self, transformer):
        """Test that metadata comments are generated."""
        func = CustomFunction(
            function_name="bar",
            return_type="int",
            parameters=[Parameter("int", "x")]
        )

        result = transformer.transform([func])

        # Check for metadata
        assert "TINCUP_GENERATED_FROM_CUSTOM" in result
        assert "TINCUP_FUNCTION_NAME: bar" in result

    def test_no_metadata_option(self):
        """Test disabling metadata generation."""
        transformer = CustomToCPOTransformer(enable_metadata=False)

        func = CustomFunction(
            function_name="baz",
            return_type="void",
            parameters=[]
        )

        result = transformer.transform([func])

        # Metadata should not be present
        assert "TINCUP_GENERATED_FROM_CUSTOM" not in result

    def test_noexcept_preservation(self, transformer):
        """Test that noexcept specification is preserved."""
        func = CustomFunction(
            function_name="safe_func",
            return_type="int",
            parameters=[],
            is_noexcept=True
        )

        result = transformer.transform([func])

        # Should have noexcept in tag_invoke stub
        assert "noexcept" in result


class TestFileTransformation:
    """Test file-based transformation."""

    def test_transform_file(self):
        """Test transforming a complete file."""
        # Create temporary input file
        custom_code = """
template<typename T>
custom void serialize(const T& obj, std::ostream& os);

custom int compute(double x) noexcept;
"""

        with tempfile.NamedTemporaryFile(mode='w', suffix='.hpp', delete=False) as f:
            f.write(custom_code)
            input_path = Path(f.name)

        try:
            # Create output path
            output_path = input_path.with_suffix('.generated.hpp')

            # Transform
            result = transform_custom_to_cpo(
                str(input_path),
                str(output_path),
                enable_metadata=True
            )

            # Check output file was created
            assert output_path.exists()

            # Read and verify content
            content = output_path.read_text()
            assert "serialize_ftor" in content
            assert "compute_ftor" in content
            assert "tag_invoke" in content

            # Check metadata
            assert "TINCUP_GENERATED_FROM_CUSTOM" in content

            # Clean up
            output_path.unlink()

        finally:
            input_path.unlink()

    def test_empty_file(self):
        """Test transforming file with no custom functions."""
        with tempfile.NamedTemporaryFile(mode='w', suffix='.hpp', delete=False) as f:
            f.write("// No custom functions here\nvoid regular_func();")
            input_path = Path(f.name)

        try:
            with pytest.raises(ValueError, match="No custom functions found"):
                transform_custom_to_cpo(str(input_path))
        finally:
            input_path.unlink()


class TestComplexScenarios:
    """Test complex transformation scenarios."""

    @pytest.fixture
    def transformer(self):
        return CustomToCPOTransformer(enable_metadata=True)

    def test_template_function_transformation(self, transformer):
        """Test transforming template function."""
        func = CustomFunction(
            function_name="identity",
            return_type="T",
            parameters=[Parameter("T", "x")],
            template_params=["typename T"]
        )

        result = transformer.transform([func])

        # Should generate CPO with template
        assert "identity_ftor" in result
        assert "tag_invoke" in result

    def test_multiple_functions(self, transformer):
        """Test transforming multiple custom functions."""
        functions = [
            CustomFunction("foo", "void", []),
            CustomFunction("bar", "int", [Parameter("int", "x")]),
            CustomFunction("baz", "double", [Parameter("double", "y")], is_noexcept=True)
        ]

        result = transformer.transform(functions)

        # All functions should be present
        assert "foo_ftor" in result
        assert "bar_ftor" in result
        assert "baz_ftor" in result

        # Check for multiple tag_invoke stubs
        assert result.count("tag_invoke") >= 3


class TestOutputFormatting:
    """Test output formatting and structure."""

    @pytest.fixture
    def transformer(self):
        return CustomToCPOTransformer(enable_metadata=True)

    def test_header_generation(self, transformer):
        """Test that proper header is generated."""
        func = CustomFunction("test", "void", [])
        result = transformer.transform([func])

        # Should have header comments
        assert "Generated by TInCuP" in result

        # Should have include guard
        assert "#pragma once" in result

        # Should include tincup header
        assert "#include <tincup/tincup.hpp>" in result

    def test_tag_invoke_stub_format(self, transformer):
        """Test tag_invoke stub formatting."""
        func = CustomFunction(
            function_name="process",
            return_type="void",
            parameters=[Parameter("int", "x"), Parameter("double", "y")]
        )

        result = transformer.transform([func])

        # Should have TODO comment
        assert "// TODO: Implement" in result

        # Should have proper signature
        assert "tag_invoke(process_ftor" in result


if __name__ == '__main__':
    pytest.main([__file__, '-v'])
