/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once
#include <tincup/tincup.hpp>

${NAMESPACE_OPEN}

/**
 * @brief ${DESCRIPTION}
 * 
 * ${DETAILED_DESCRIPTION}
 */
inline constexpr struct ${CPO_NAME}_ftor final : tincup::cpo_base<${CPO_NAME}_ftor> {
    TINCUP_CPO_TAG("${CPO_NAME}")

    template<${TEMPLATE_PARAMS}>
        requires tag_invocable_c<${CPO_NAME}_ftor, ${REQUIRES_PARAMS}>
        constexpr auto operator()(${FUNCTION_PARAMS}) const
        noexcept(nothrow_tag_invocable_c<${CPO_NAME}_ftor, ${REQUIRES_PARAMS}>) 
        -> tag_invocable_t<${CPO_NAME}_ftor, ${REQUIRES_PARAMS}> {
            return tag_invoke(*this, ${FORWARD_ARGS});
        }

    template<${TEMPLATE_PARAMS}>
        requires (!tag_invocable_c<${CPO_NAME}_ftor, ${REQUIRES_PARAMS}>)
        constexpr void operator()(${FUNCTION_PARAMS}) const {
            this->fail(${FORWARD_ARGS});
        }
} ${CPO_NAME};

${NAMESPACE_CLOSE}
