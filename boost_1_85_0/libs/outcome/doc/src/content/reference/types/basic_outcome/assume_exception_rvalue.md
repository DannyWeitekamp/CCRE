+++
title = "`exception_type &&assume_exception() && noexcept`"
description = "Narrow contract rvalue reference observer of the stored exception. Constexpr propagating, never throws."
categories = ["observers"]
weight = 782
+++

Narrow contract rvalue reference observer of the stored exception. `NoValuePolicy::narrow_exception_check()` is first invoked, then the reference to the exception is returned. As a valid default constructed exception is always present, no undefined behaviour occurs unless `NoValuePolicy::narrow_exception_check()` does that.

Note that if `exception_type` is `void`, only a `const` overload returning `void` is present.

*Requires*: Always available.

*Complexity*: Depends on `NoValuePolicy::narrow_exception_check()`.

*Guarantees*: An exception is never thrown.
