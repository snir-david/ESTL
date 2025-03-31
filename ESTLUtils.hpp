//
// Created by snirn on 3/13/25.
//

#ifndef ESTLUTILS_HPP
#define ESTLUTILS_HPP

#define WARN_IF(condition, message) \
do { \
if (condition) { \
__attribute__((warning(message))) int dummy; \
} \
} while (0)

#endif //ESTLUTILS_HPP
