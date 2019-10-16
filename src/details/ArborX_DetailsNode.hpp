/****************************************************************************
 * Copyright (c) 2012-2019 by the ArborX authors                            *
 * All rights reserved.                                                     *
 *                                                                          *
 * This file is part of the ArborX library. ArborX is                       *
 * distributed under a BSD 3-clause license. For the licensing terms see    *
 * the LICENSE file in the top-level directory.                             *
 *                                                                          *
 * SPDX-License-Identifier: BSD-3-Clause                                    *
 ****************************************************************************/

#ifndef ARBORX_NODE_HPP
#define ARBORX_NODE_HPP

#include <ArborX_Box.hpp>

#include <Kokkos_Pair.hpp>

namespace ArborX
{
namespace Details
{
struct Node
{
  KOKKOS_FUNCTION
  constexpr Node() = default;

  KOKKOS_FUNCTION constexpr bool isLeaf() const noexcept
  {
    return children.first == nullptr;
  }

  KOKKOS_FUNCTION std::size_t getLeafPermutationIndex() const noexcept
  {
    return reinterpret_cast<std::size_t>(children.second);
  }

  Kokkos::pair<Node *, Node *> children = {nullptr, nullptr};
  Box bounding_box;
};

KOKKOS_INLINE_FUNCTION Node makeLeafNode(std::size_t index, Box box) noexcept
{
  return {{nullptr, reinterpret_cast<Node *>(index)}, std::move(box)};
}
} // namespace Details
} // namespace ArborX

#endif
