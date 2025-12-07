#!/usr/bin/env python3
"""
Tests for custom function parser.
"""

import pytest
from cpo_tools.bijective.custom_parser import CustomFunctionParser, CustomFunction, Parameter


class TestParameterParsing:
    """Test Parameter class functionality."""

    def test_simple_parameter(self):
        """Test simple int parameter."""
        param = Parameter("int", "x")
        assert param.type_str == "int"
        assert param.name == "x"
        assert param.to_tincup_format() == "int: x"

    def test_generic_parameter(self):
        """Test generic template parameter."""
        param = Parameter("T", "value")
        assert param.to_tincup_format() == "$T: value"

    def test_const_ref_parameter(self):
        """Test const reference parameter."""
        param = Parameter("const T&", "obj")
        assert param.to_tincup_format() == "$const T&: obj"

    def test_templated_type_parameter(self):
        """Test parameter with template type."""
        param = Parameter("std::vector<T>", "vec")
        assert param.to_tincup_format() == "$std::vector<T>: vec"


class TestCustomFunctionParsing:
    """Test CustomFunctionParser."""

    @pytest.fixture
    def parser(self):
        return CustomFunctionParser()

    def test_simple_function(self, parser):
        """Test parsing simple custom function."""
        code = "custom void foo();"
        functions = parser.parse(code)

        assert len(functions) == 1
        func = functions[0]
        assert func.function_name == "foo"
        assert func.return_type == "void"
        assert len(func.parameters) == 0
        assert not func.is_noexcept
        assert len(func.template_params) == 0

    def test_function_with_parameters(self, parser):
        """Test parsing function with parameters."""
        code = "custom int add(int x, int y);"
        functions = parser.parse(code)

        assert len(functions) == 1
        func = functions[0]
        assert func.function_name == "add"
        assert func.return_type == "int"
        assert len(func.parameters) == 2

        assert func.parameters[0].type_str == "int"
        assert func.parameters[0].name == "x"
        assert func.parameters[1].type_str == "int"
        assert func.parameters[1].name == "y"

    def test_template_function(self, parser):
        """Test parsing template function."""
        code = "template<typename T> custom T identity(T x);"
        functions = parser.parse(code)

        assert len(functions) == 1
        func = functions[0]
        assert func.function_name == "identity"
        assert func.return_type == "T"
        assert len(func.template_params) == 1
        assert func.template_params[0] == "typename T"
        assert len(func.parameters) == 1
        assert func.parameters[0].type_str == "T"

    def test_noexcept_function(self, parser):
        """Test parsing noexcept function."""
        code = "custom int compute() noexcept;"
        functions = parser.parse(code)

        assert len(functions) == 1
        func = functions[0]
        assert func.function_name == "compute"
        assert func.is_noexcept

    def test_complex_types(self, parser):
        """Test parsing function with complex parameter types."""
        code = "custom void serialize(const MyType& obj, std::ostream& os);"
        functions = parser.parse(code)

        assert len(functions) == 1
        func = functions[0]
        assert len(func.parameters) == 2

        assert func.parameters[0].type_str == "const MyType&"
        assert func.parameters[0].name == "obj"

        assert func.parameters[1].type_str == "std::ostream&"
        assert func.parameters[1].name == "os"

    def test_template_parameters_in_types(self, parser):
        """Test parsing function with templated types."""
        code = "template<typename T> custom void process(std::vector<T>& vec);"
        functions = parser.parse(code)

        assert len(functions) == 1
        func = functions[0]
        assert len(func.parameters) == 1
        assert "std::vector<T>" in func.parameters[0].type_str

    def test_multiple_template_params(self, parser):
        """Test function with multiple template parameters."""
        code = "template<typename T, typename U> custom void foo(T x, U y);"
        functions = parser.parse(code)

        assert len(functions) == 1
        func = functions[0]
        assert len(func.template_params) == 2
        assert func.template_params[0] == "typename T"
        assert func.template_params[1] == "typename U"

    def test_function_with_body(self, parser):
        """Test parsing custom function with inline body."""
        code = """
        custom void foo() {
            // implementation
        }
        """
        functions = parser.parse(code)

        assert len(functions) == 1
        func = functions[0]
        assert func.function_name == "foo"

    def test_multiple_functions(self, parser):
        """Test parsing multiple custom functions."""
        code = """
        custom void foo();
        custom int bar(int x);
        template<typename T>
        custom T baz(T x) noexcept;
        """
        functions = parser.parse(code)

        assert len(functions) == 3
        assert functions[0].function_name == "foo"
        assert functions[1].function_name == "bar"
        assert functions[2].function_name == "baz"
        assert functions[2].is_noexcept


class TestTInCuPSpecGeneration:
    """Test conversion to TInCuP spec format."""

    def test_simple_spec(self):
        """Test generating TInCuP spec for simple function."""
        func = CustomFunction(
            function_name="foo",
            return_type="void",
            parameters=[]
        )

        spec = func.to_tincup_spec()
        assert spec == {
            "cpo_name": "foo",
            "args": []
        }

    def test_spec_with_parameters(self):
        """Test generating TInCuP spec with parameters."""
        func = CustomFunction(
            function_name="add",
            return_type="int",
            parameters=[
                Parameter("int", "x"),
                Parameter("int", "y")
            ]
        )

        spec = func.to_tincup_spec()
        assert spec["cpo_name"] == "add"
        assert len(spec["args"]) == 2
        assert spec["args"][0] == "int: x"
        assert spec["args"][1] == "int: y"

    def test_spec_with_generic_types(self):
        """Test generating TInCuP spec with generic types."""
        func = CustomFunction(
            function_name="serialize",
            return_type="void",
            parameters=[
                Parameter("const T&", "obj"),
                Parameter("std::ostream&", "os")
            ],
            template_params=["typename T"]
        )

        spec = func.to_tincup_spec()
        assert spec["cpo_name"] == "serialize"
        assert "$const T&: obj" in spec["args"]


class TestEdgeCases:
    """Test edge cases and error handling."""

    @pytest.fixture
    def parser(self):
        return CustomFunctionParser()

    def test_no_custom_functions(self, parser):
        """Test parsing file with no custom functions."""
        code = """
        void regular_function() {}
        int another_function(int x);
        """
        functions = parser.parse(code)
        assert len(functions) == 0

    def test_custom_as_identifier(self, parser):
        """Test that custom used as identifier doesn't match."""
        code = """
        int custom = 5;  // custom as variable name
        void foo(int custom);  // custom as parameter name
        """
        functions = parser.parse(code)
        assert len(functions) == 0

    def test_rvalue_references(self, parser):
        """Test parsing function with rvalue references."""
        code = "custom void move_into(std::vector<T>&& vec);"
        functions = parser.parse(code)

        assert len(functions) == 1
        func = functions[0]
        assert "&&" in func.parameters[0].type_str

    def test_variadic_template(self, parser):
        """Test parsing variadic template function."""
        code = "template<typename... Args> custom void foo(Args... args);"
        functions = parser.parse(code)

        assert len(functions) == 1
        func = functions[0]
        assert "typename... Args" in func.template_params[0]


if __name__ == '__main__':
    pytest.main([__file__, '-v'])
