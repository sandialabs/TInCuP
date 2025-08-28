#!/usr/bin/env python3
# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

"""
Comprehensive test suite for CPO pattern verification.

Tests all supported type patterns and validates that our verification
system correctly identifies valid and invalid CPO structures.
"""

import unittest
import tempfile
import os
import subprocess
import sys
from io import StringIO
import json

# Import our modules
try:
    from .cpo_generator import main as cpo_main
except ImportError:
    # Fallback for direct execution
    try:
        from cpo_generator import main as cpo_main
    except ImportError:
        from cpo_tools.cpo_generator import main as cpo_main

class CPOPatternTests(unittest.TestCase):
    """Test all supported CPO patterns for compliance."""
    
    def setUp(self):
        """Set up test environment."""
        self.maxDiff = None
        
    def generate_cpo(self, cpo_spec):
        """Generate a CPO using our generator and return the output."""
        # Capture stdout from cpo_generator
        old_stdout = sys.stdout
        sys.stdout = captured_output = StringIO()
        
        # Simulate command line args
        old_argv = sys.argv
        sys.argv = ['cpo_generator', json.dumps(cpo_spec)]
        
        try:
            cpo_main()
            result = captured_output.getvalue()
        except SystemExit:
            result = captured_output.getvalue()
        finally:
            sys.stdout = old_stdout
            sys.argv = old_argv
            
        return result

class ConcreteTypePatternTests(CPOPatternTests):
    """Test concrete type patterns."""
    

    def test_concrete_by_value(self):
        """Test concrete types passed by value."""
        spec = {"cpo_name": "concrete_by_value", "args": ["int: value", "double: scalar"]}
        result = self.generate_cpo(spec)
  
        # Verify key structural elements (new CRTP architecture)
        self.assertIn('struct concrete_by_value_ftor final : tincup::cpo_base<concrete_by_value_ftor> {', result)
        self.assertIn('TINCUP_CPO_TAG("concrete_by_value")', result)
        self.assertIn('} concrete_by_value;', result)  # Proper struct closing
        self.assertIn('// Note: operator() methods are provided by cpo_base', result)
        # Verify usage guidance comments
        self.assertIn('// Usage: tincup::is_invocable_v<concrete_by_value_ftor, int, double>', result)

    def test_concrete_lvalue_references(self):
        """Test concrete types with lvalue references."""
        spec = {"cpo_name": "concrete_lvalue_ref", "args": ["int&: target", "const double&: source"]}
        result = self.generate_cpo(spec)
        
        # Verify usage guidance comments
        self.assertIn('// Usage: tincup::is_invocable_v<concrete_lvalue_ref_ftor, int&, const double&>', result)
        self.assertIn('// Note: operator() methods are provided by cpo_base', result)
        
    def test_concrete_rvalue_reference(self):
        """Test concrete types with rvalue references."""
        spec = {"cpo_name": "concrete_rvalue_ref", "args": ["std::string&&: data"]}
        result = self.generate_cpo(spec)
        
        # Verify usage guidance comments  
        self.assertIn('// Usage: tincup::is_invocable_v<concrete_rvalue_ref_ftor, std::string&&>', result)
        self.assertIn('// Note: operator() methods are provided by cpo_base', result)

class GenericTypePatternTests(CPOPatternTests):
    """Test generic type patterns."""
    
    def test_generic_by_value(self):
        """Test generic types passed by value."""
        spec = {"cpo_name": "generic_by_value", "args": ["$T: value", "$U: other"]}
        result = self.generate_cpo(spec)
        
        # Verify usage guidance comments (generic version)
        self.assertIn('// Usage: tincup::is_invocable_v<generic_by_value_ftor, T, U>', result)
        self.assertIn('// Note: operator() methods are provided by cpo_base', result)
        
    def test_generic_lvalue_references(self):
        """Test generic types with lvalue references."""
        spec = {"cpo_name": "generic_lvalue_ref", "args": ["$T&: target", "$const U&: source"]}
        result = self.generate_cpo(spec)
        
        # Verify usage guidance comments (generic lvalue refs)
        self.assertIn('// Usage: tincup::is_invocable_v<generic_lvalue_ref_ftor, T&, const U&>', result)
        self.assertIn('// Note: operator() methods are provided by cpo_base', result)
        
    def test_generic_forwarding_reference(self):
        """Test generic types with forwarding references."""
        spec = {"cpo_name": "generic_forwarding_ref", "args": ["$T&&: fwd_ref"]}
        result = self.generate_cpo(spec)
        
        # Verify usage guidance comments (forwarding ref)
        self.assertIn('// Usage: tincup::is_invocable_v<generic_forwarding_ref_ftor, T>', result)
        self.assertIn('// Note: operator() methods are provided by cpo_base', result)
        
    def test_generic_variadic_by_value(self):
        """Test variadic generic types by value."""
        spec = {"cpo_name": "generic_variadic_value", "args": ["$T...: args"]}
        result = self.generate_cpo(spec)
        
        # Verify usage guidance comments (variadic)
        self.assertIn('// Usage: tincup::is_invocable_v<generic_variadic_value_ftor, T...>', result)
        self.assertIn('// Note: operator() methods are provided by cpo_base', result)
        
    def test_generic_variadic_lvalue_refs(self):
        """Test variadic generic types with lvalue references."""
        spec = {"cpo_name": "generic_variadic_lvalue", "args": ["$T&...: targets"]}
        result = self.generate_cpo(spec)
        
        # Verify usage guidance comments (variadic lvalue refs)
        self.assertIn('// Usage: tincup::is_invocable_v<generic_variadic_lvalue_ftor, T&...>', result)
        self.assertIn('// Note: operator() methods are provided by cpo_base', result)
        
    def test_generic_variadic_forwarding_refs(self):
        """Test variadic generic types with forwarding references."""
        spec = {"cpo_name": "generic_variadic_forwarding", "args": ["$T&&...: fwd_args"]}
        result = self.generate_cpo(spec)
        
        # Verify usage guidance comments (variadic forwarding refs)
        self.assertIn('// Usage: tincup::is_invocable_v<generic_variadic_forwarding_ftor, T...>', result)
        self.assertIn('// Note: operator() methods are provided by cpo_base', result)
        # Note: The generator should handle variadic forwarding correctly

class LLMModePatternTests(CPOPatternTests):
    """Test LLM mode patterns."""
    
    def test_llm_mode_tokens_present(self):
        """Verify LLM tokens appear in LLM mode."""
        spec = {"cpo_name": "llm_test", "operation_type": "mutating_binary"}
        result = self.generate_cpo(spec)
        
        self.assertIn('LLM_TODO:', result)
        self.assertIn('LLM_HINT:', result)
        
    def test_vim_mode_no_llm_tokens(self):
        """Verify LLM tokens don't appear in VIM mode."""
        spec = {"cpo_name": "vim_test", "args": ["$T&: target", "$const U&: source"]}
        result = self.generate_cpo(spec)
        
        self.assertNotIn('LLM_TODO:', result)
        self.assertNotIn('LLM_HINT:', result)

class CorruptedPatternTests(unittest.TestCase):
    """Test corrupted CPO patterns to verify our verification catches them."""
    
    def setUp(self):
        """Set up corrupted test cases."""
        self.valid_cpo = '''
inline constexpr struct test_cpo_ftor final : tincup::cpo_base<test_cpo_ftor> {
  TINCUP_CPO_TAG("test_cpo")
  // Note: operator() methods are provided by cpo_base<test_cpo_ftor> via CRTP
} test_cpo;

// Usage: tincup::is_invocable_v<test_cpo_ftor, T&>
// Usage: tincup::invocable_t<test_cpo_ftor, T&>  
// Usage: tincup::cpo_traits<test_cpo_ftor, T&>
'''
    
    def test_missing_declare_cpo_tag(self):
        """Test detection of missing DECLARE_CPO_TAG."""
        corrupted = self.valid_cpo.replace('TINCUP_CPO_TAG("test_cpo")', '// Missing tag')
        # TODO: Use CPOVerifier to validate this fails
        
    def test_missing_noexcept(self):
        """Test detection of missing noexcept specification."""
        # With new CRTP architecture, noexcept is handled in cpo_base
        # This test would need to corrupt the base class definition
        corrupted = self.valid_cpo.replace(
            'tincup::cpo_base<test_cpo_ftor>', 
            'tincup::broken_cpo_base<test_cpo_ftor>'
        )
        # TODO: Use CPOVerifier to validate this fails
        
    def test_missing_trailing_return_type(self):
        """Test detection of missing trailing return type."""
        # With new CRTP architecture, return types are handled in cpo_base
        # This test would need to corrupt the concept alias generation
        corrupted = self.valid_cpo.replace(
            'TINCUP_GENERATE_CPO_CONCEPT_ALIASES(test_cpo, typename T, T&)', 
            '// Missing concept aliases'
        )
        # TODO: Use CPOVerifier to validate this fails
        
    def test_inconsistent_template_parameters(self):
        """Test detection of inconsistent template parameters."""
        # With new predicate-based approach, inconsistent parameter usage is less common
        corrupted = self.valid_cpo.replace(
            '// Usage: tincup::is_invocable_v<test_cpo_ftor, T&>',
            '// Usage: tincup::is_invocable_v<test_cpo_ftor, U&>'  # Inconsistent types
        )
        # TODO: Use CPOVerifier to validate this fails
        
    def test_missing_usage_guidance(self):
        """Test detection of missing usage guidance."""
        corrupted = self.valid_cpo.split('// Usage: tincup::is_invocable_v')[0]
        # TODO: Use CPOVerifier to validate this fails
        
    def test_wrong_inheritance(self):
        """Test detection of wrong base class."""
        corrupted = self.valid_cpo.replace(
            'tincup::cpo_base<test_cpo_ftor>', 
            'tincup::some_other_base<test_cpo_ftor>'
        )
        # TODO: Use CPOVerifier to validate this fails
        
    def test_wrong_naming_convention(self):
        """Test detection of naming convention violations."""
        corrupted = self.valid_cpo.replace('test_cpo_ftor', 'TestCpoFtor')  # Wrong case
        # TODO: Use CPOVerifier to validate this fails

class IntegrationTests(unittest.TestCase):
    """Integration tests for the complete system."""
    
    def test_generate_and_verify_roundtrip(self):
        """Test that generated CPOs pass verification."""
        # Generate various CPOs and verify they all pass validation
        test_specs = [
            {"cpo_name": "test1", "args": ["int: value"]},
            {"cpo_name": "test2", "args": ["$T&: target", "$const U&: source"]},
            {"cpo_name": "test3", "args": ["$T&&: fwd_ref"]},
            {"cpo_name": "test4", "operation_type": "mutating_binary"}
        ]
        
        for spec in test_specs:
            with self.subTest(spec=spec):
                # TODO: Generate CPO, write to temp file, verify with CPOVerifier
                pass


class VariadicFlagVerificationTests(unittest.TestCase):
    """Tests for is_variadic flag presence and correctness."""

    def _verify_file(self, content: str):
        from cpo_tools.cpo_verification_enhanced import CPOVerifier
        with tempfile.NamedTemporaryFile(mode='w+', suffix='.hpp', delete=False) as tmp:
            tmp.write(content)
            tmp.flush()
            path = tmp.name
        try:
            ver = CPOVerifier(path)
            cpos = ver.find_cpo_definitions()
            self.assertTrue(cpos, "No CPO definitions found in test content")
            errs = ver.verify_variadic_flag(cpos[0])
            return errs
        finally:
            try:
                os.remove(path)
            except OSError:
                pass

    def test_missing_is_variadic_flag_detected(self):
        """CPO with parameter pack but missing is_variadic should be flagged."""
        content = r'''
inline constexpr struct missing_flag_ftor final : cpo_base<missing_flag_ftor> {
  TINCUP_CPO_TAG("missing_flag")
  template<typename... T>
  constexpr auto operator()(T... xs) const { return 0; }
} missing_flag;
'''
        errs = self._verify_file(content)
        self.assertTrue(any('missing inline static constexpr bool is_variadic' in e for e in errs), errs)

    def test_is_variadic_false_but_pack_present(self):
        """CPO with is_variadic=false and pack in signature should be flagged."""
        content = r'''
inline constexpr struct false_flag_ftor final : cpo_base<false_flag_ftor> {
  TINCUP_CPO_TAG("false_flag")
  inline static constexpr bool is_variadic = false;
  template<typename... T>
  constexpr auto operator()(T... xs) const { return 0; }
} false_flag;
'''
        errs = self._verify_file(content)
        self.assertTrue(any("is_variadic=false but parameter pack ('...')" in e for e in errs), errs)

    def test_is_variadic_true_but_no_pack(self):
        """CPO with is_variadic=true but no pack in signature should be flagged."""
        content = r'''
inline constexpr struct true_flag_no_pack_ftor final : cpo_base<true_flag_no_pack_ftor> {
  TINCUP_CPO_TAG("true_flag_no_pack")
  inline static constexpr bool is_variadic = true;
  template<typename T>
  constexpr auto operator()(T x) const { return x; }
} true_flag_no_pack;
'''
        errs = self._verify_file(content)
        self.assertTrue(any("is_variadic=true but no parameter pack ('...')" in e for e in errs), errs)


class GeneratedVariadicFlagTests(CPOPatternTests):
    """Ensure generated CPOs include correct is_variadic and pass verifier."""

    def _verify_generated(self, spec, expect_true: bool):
        from cpo_tools.cpo_verification_enhanced import CPOVerifier
        code = self.generate_cpo(spec)
        # Check the flag presence in generated code
        flag_line = f"inline static constexpr bool is_variadic = {'true' if expect_true else 'false'};"
        self.assertIn(flag_line, code)

        # Run verifier on the generated code
        with tempfile.NamedTemporaryFile(mode='w+', suffix='.hpp', delete=False) as tmp:
            tmp.write(code)
            tmp.flush()
            path = tmp.name
        try:
            ver = CPOVerifier(path)
            cpos = ver.find_cpo_definitions()
            self.assertTrue(cpos, "No CPO definitions found in generated code")
            errs = ver.verify_variadic_flag(cpos[0])
            self.assertEqual(errs, [], f"Unexpected variadic flag errors: {errs}")
        finally:
            try:
                os.remove(path)
            except OSError:
                pass

    def test_generated_non_variadic(self):
        spec = {"cpo_name": "gen_non_variadic", "args": ["$T&: x", "$const U&: y"]}
        self._verify_generated(spec, expect_true=False)

    def test_generated_variadic(self):
        spec = {"cpo_name": "gen_variadic", "args": ["$T...: xs"]}
        self._verify_generated(spec, expect_true=True)

def create_test_suite():
    """Create a comprehensive test suite."""
    suite = unittest.TestSuite()
    
    # Add all test classes
    test_classes = [
        ConcreteTypePatternTests,
        GenericTypePatternTests, 
        LLMModePatternTests,
        CorruptedPatternTests,
        IntegrationTests,
        VariadicFlagVerificationTests,
        GeneratedVariadicFlagTests,
    ]
    
    for test_class in test_classes:
        tests = unittest.TestLoader().loadTestsFromTestCase(test_class)
        suite.addTests(tests)
        
    return suite

if __name__ == '__main__':
    # Run with detailed output
    runner = unittest.TextTestRunner(verbosity=2)
    suite = create_test_suite()
    result = runner.run(suite)
    
    # Exit with error code if tests failed
    sys.exit(0 if result.wasSuccessful() else 1)
