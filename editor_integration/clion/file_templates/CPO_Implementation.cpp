/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#include "${HEADER_NAME}.hpp"

${NAMESPACE_OPEN}

// Example tag_invoke implementation for ${CPO_NAME}
// Customize this implementation for your specific use case
constexpr auto tag_invoke(${CPO_NAME}_ftor, ${EXAMPLE_PARAMS}) noexcept -> ${RETURN_TYPE} {
    // TODO: Implement your CPO logic here
    ${IMPLEMENTATION_BODY}
}

${NAMESPACE_CLOSE}