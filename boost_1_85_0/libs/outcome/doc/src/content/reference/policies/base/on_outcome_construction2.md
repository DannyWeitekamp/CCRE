+++
title = "`static void on_outcome_construction(T *, U &&, V &&) noexcept`"
description = "(>= Outcome v2.2.0) Hook invoked by the implicit constructors of `basic_outcome`."
categories = ["observer-policies"]
weight = 450
+++

One of the constructor hooks for {{% api "basic_outcome<T, EC, EP, NoValuePolicy>" %}}, generally invoked by the implicit constructors of `basic_outcome` which consume two arguments. See each constructor's documentation to see which specific hook it invokes.

*Requires*: Always available.

*Guarantees*: Never throws an exception.
