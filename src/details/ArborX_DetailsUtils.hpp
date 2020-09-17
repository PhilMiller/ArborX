/****************************************************************************
 * Copyright (c) 2012-2020 by the ArborX authors                            *
 * All rights reserved.                                                     *
 *                                                                          *
 * This file is part of the ArborX library. ArborX is                       *
 * distributed under a BSD 3-clause license. For the licensing terms see    *
 * the LICENSE file in the top-level directory.                             *
 *                                                                          *
 * SPDX-License-Identifier: BSD-3-Clause                                    *
 ****************************************************************************/

#ifndef ARBORX_DETAILS_UTILS_HPP
#define ARBORX_DETAILS_UTILS_HPP

#include <ArborX_Exception.hpp>
#include <ArborX_Macros.hpp>

#include <Kokkos_Core.hpp>
#include <Kokkos_Sort.hpp> // min_max_functor

namespace ArborX
{

namespace Details
{

namespace internal
{
template <typename PointerType>
struct PointerDepth
{
  static int constexpr value = 0;
};

template <typename PointerType>
struct PointerDepth<PointerType *>
{
  static int constexpr value = PointerDepth<PointerType>::value + 1;
};

template <typename PointerType, std::size_t N>
struct PointerDepth<PointerType[N]>
{
  static int constexpr value = PointerDepth<PointerType>::value;
};
} // namespace internal

template <typename View, typename ExecutionSpace>
inline Kokkos::View<typename View::traits::data_type, Kokkos::LayoutRight,
                    typename ExecutionSpace::memory_space>
create_layout_right_mirror_view(
    ExecutionSpace const & /*execution_space*/, View const &src,
    typename std::enable_if<!(
        (std::is_same<typename View::traits::array_layout,
                      Kokkos::LayoutRight>::value ||
         (View::rank == 1 && !std::is_same<typename View::traits::array_layout,
                                           Kokkos::LayoutStride>::value)) &&
        std::is_same<typename View::traits::memory_space,
                     typename ExecutionSpace::memory_space>::value)>::type * =
        nullptr)
{
  constexpr int pointer_depth =
      internal::PointerDepth<typename View::traits::data_type>::value;
  return Kokkos::View<typename View::traits::data_type, Kokkos::LayoutRight,
                      typename ExecutionSpace::memory_space>(
      std::string(src.label()).append("_layout_right_mirror"), src.extent(0),
      pointer_depth > 1 ? src.extent(1) : KOKKOS_INVALID_INDEX,
      pointer_depth > 2 ? src.extent(2) : KOKKOS_INVALID_INDEX,
      pointer_depth > 3 ? src.extent(3) : KOKKOS_INVALID_INDEX,
      pointer_depth > 4 ? src.extent(4) : KOKKOS_INVALID_INDEX,
      pointer_depth > 5 ? src.extent(5) : KOKKOS_INVALID_INDEX,
      pointer_depth > 6 ? src.extent(6) : KOKKOS_INVALID_INDEX,
      pointer_depth > 7 ? src.extent(7) : KOKKOS_INVALID_INDEX);
}

template <typename View, typename ExecutionSpace>
inline auto create_layout_right_mirror_view(
    ExecutionSpace const & /*execution_space*/, View const &src,
    typename std::enable_if<
        ((std::is_same<typename View::traits::array_layout,
                       Kokkos::LayoutRight>::value ||
          (View::rank == 1 && !std::is_same<typename View::traits::array_layout,
                                            Kokkos::LayoutStride>::value)) &&
         std::is_same<typename View::traits::memory_space,
                      typename ExecutionSpace::memory_space>::value)>::type * =
        nullptr)
{
  return src;
}

template <typename View>
inline auto create_layout_right_mirror_view(View const &src)
{
  return create_layout_right_mirror_view(
      typename View::traits::host_mirror_space{}, src);
}

template <typename View, typename ExecutionSpace>
inline auto create_layout_right_mirror_view_and_copy(
    ExecutionSpace const &execution_space, View const &src,
    typename std::enable_if<!(
        (std::is_same<typename View::traits::array_layout,
                      Kokkos::LayoutRight>::value ||
         (View::rank == 1 && !std::is_same<typename View::traits::array_layout,
                                           Kokkos::LayoutStride>::value)) &&
        std::is_same<typename View::traits::memory_space,
                     typename ExecutionSpace::memory_space>::value)>::type * =
        0)
{
  constexpr int pointer_depth =
      internal::PointerDepth<typename View::traits::data_type>::value;
  Kokkos::View<typename View::traits::data_type, Kokkos::LayoutRight,
               typename ExecutionSpace::memory_space>
      layout_right_view(
          Kokkos::ViewAllocateWithoutInitializing(
              std::string(src.label()).append("_layout_right_mirror")),
          src.extent(0),
          pointer_depth > 1 ? src.extent(1) : KOKKOS_INVALID_INDEX,
          pointer_depth > 2 ? src.extent(2) : KOKKOS_INVALID_INDEX,
          pointer_depth > 3 ? src.extent(3) : KOKKOS_INVALID_INDEX,
          pointer_depth > 4 ? src.extent(4) : KOKKOS_INVALID_INDEX,
          pointer_depth > 5 ? src.extent(5) : KOKKOS_INVALID_INDEX,
          pointer_depth > 6 ? src.extent(6) : KOKKOS_INVALID_INDEX,
          pointer_depth > 7 ? src.extent(7) : KOKKOS_INVALID_INDEX);
  auto tmp_view = Kokkos::create_mirror_view_and_copy(execution_space, src);
  // TODO not quite sure wy this can't be execution_space
  Kokkos::deep_copy(/*execution_space, */ layout_right_view, tmp_view);
  return layout_right_view;
}

template <typename View, typename ExecutionSpace>
inline auto create_layout_right_mirror_view_and_copy(
    ExecutionSpace const & /*execution_space*/, View const &src,
    typename std::enable_if<
        ((std::is_same<typename View::traits::array_layout,
                       Kokkos::LayoutRight>::value ||
          (View::rank == 1 && !std::is_same<typename View::traits::array_layout,
                                            Kokkos::LayoutStride>::value)) &&
         std::is_same<typename View::traits::memory_space,
                      typename ExecutionSpace::memory_space>::value)>::type * =
        nullptr)
{
  return src;
}

// NOTE: This functor is used in exclusivePrefixSum( src, dst ).  We were
// getting a compile error on CUDA when using a KOKKOS_LAMBDA.
template <typename T, typename DeviceType>
class ExclusiveScanFunctor
{
public:
  ExclusiveScanFunctor(Kokkos::View<T *, DeviceType> const &in,
                       Kokkos::View<T *, DeviceType> const &out)
      : _in(in)
      , _out(out)
  {
  }
  KOKKOS_INLINE_FUNCTION void operator()(int i, T &update,
                                         bool final_pass) const
  {
    T const in_i = _in(i);
    if (final_pass)
      _out(i) = update;
    update += in_i;
  }

private:
  Kokkos::View<T *, DeviceType> _in;
  Kokkos::View<T *, DeviceType> _out;
};
} // namespace Details

/** \brief Computes an exclusive scan.
 *
 *  \param[in] space Execution space
 *  \param[in] src Input view with range of elements to sum
 *  \param[out] dst Output view; may be equal to \p src
 *
 *  When \p dst is not provided or if \p src and \p dst are the same view, the
 *  scan is performed in-place.  "Exclusive" means that the i-th input element
 *  is not included in the i-th sum.
 *
 *  \pre \p src and \p dst must be of rank 1 and have the same size.
 */
template <typename ExecutionSpace, typename ST, typename... SP, typename DT,
          typename... DP>
void exclusivePrefixSum(ExecutionSpace &&space,
                        Kokkos::View<ST, SP...> const &src,
                        Kokkos::View<DT, DP...> const &dst)
{
  static_assert(
      std::is_same<
          typename Kokkos::ViewTraits<DT, DP...>::value_type,
          typename Kokkos::ViewTraits<DT, DP...>::non_const_value_type>::value,
      "exclusivePrefixSum requires non-const destination type");

  static_assert(
      (unsigned(Kokkos::ViewTraits<DT, DP...>::rank) ==
       unsigned(Kokkos::ViewTraits<ST, SP...>::rank)) &&
          (unsigned(Kokkos::ViewTraits<DT, DP...>::rank) == unsigned(1)),
      "exclusivePrefixSum requires Views of rank 1");

  using ValueType = typename Kokkos::ViewTraits<DT, DP...>::value_type;
  using DeviceType = typename Kokkos::ViewTraits<DT, DP...>::device_type;

  auto const n = src.extent(0);
  ARBORX_ASSERT(n == dst.extent(0));
  Kokkos::RangePolicy<std::decay_t<ExecutionSpace>> policy(
      std::forward<ExecutionSpace>(space), 0, n);
  Kokkos::parallel_scan(
      ARBORX_MARK_REGION("exclusive_scan"), policy,
      Details::ExclusiveScanFunctor<ValueType, DeviceType>(src, dst));
}

/** \brief In-place exclusive scan.
 *
 *  \param[in] space Execution space
 *  \param[in,out] v View with range of elements to sum
 *
 *  Calls \c exclusivePrefixSum(v, v)
 */
template <typename ExecutionSpace, typename T, typename... P>
inline std::enable_if_t<
    Kokkos::is_execution_space<std::remove_reference_t<ExecutionSpace>>::value>
exclusivePrefixSum(ExecutionSpace &&space, Kokkos::View<T, P...> const &v)
{
  exclusivePrefixSum(std::forward<ExecutionSpace>(space), v, v);
}

template <typename ST, typename... SP, typename DT, typename... DP>
[[deprecated]] inline void
exclusivePrefixSum(Kokkos::View<ST, SP...> const &src,
                   Kokkos::View<DT, DP...> const &dst)
{
  using ExecutionSpace = typename Kokkos::View<DT, DP...>::execution_space;
  exclusivePrefixSum(ExecutionSpace{}, src, dst);
}

template <typename T, typename... P>
[[deprecated]] inline void exclusivePrefixSum(Kokkos::View<T, P...> const &v)
{
  using ExecutionSpace = typename Kokkos::View<T, P...>::execution_space;
  exclusivePrefixSum(ExecutionSpace{}, v);
}

/** \brief Get a copy of the last element.
 *
 *  Returns a copy of the last element in the view on the host.  Note that it
 *  may require communication between host and device (e.g. if the view passed
 *  as an argument lives on the device).
 *
 *  \pre \c v is of rank 1 and not empty.
 */
template <typename T, typename... P>
typename Kokkos::ViewTraits<T, P...>::non_const_value_type
lastElement(Kokkos::View<T, P...> const &v)
{
  static_assert((unsigned(Kokkos::ViewTraits<T, P...>::rank) == unsigned(1)),
                "lastElement requires Views of rank 1");
  auto const n = v.extent(0);
  ARBORX_ASSERT(n > 0);
  auto v_subview = Kokkos::subview(v, n - 1);
  auto v_host = Kokkos::create_mirror_view(v_subview);
  Kokkos::deep_copy(v_host, v_subview);
  return v_host();
}

/** \brief Fills the view with a sequence of numbers
 *
 *  \param[in] space Execution space
 *  \param[out] v Output view
 *  \param[in] value (optional) Initial value
 *
 *  \note Similar to \c std::iota() but differs in that it directly assigns
 *  <code>v(i) = value + i</code> instead of repetitively evaluating
 *  <code>++value</code> which would be difficult to achieve in a performant
 *  manner while still guaranteeing the order of execution.
 */
template <typename ExecutionSpace, typename T, typename... P>
void iota(ExecutionSpace &&space, Kokkos::View<T, P...> const &v,
          typename Kokkos::ViewTraits<T, P...>::value_type value = 0)
{
  using ValueType = typename Kokkos::ViewTraits<T, P...>::value_type;
  static_assert((unsigned(Kokkos::ViewTraits<T, P...>::rank) == unsigned(1)),
                "iota requires a View of rank 1");
  static_assert(std::is_arithmetic<ValueType>::value,
                "iota requires a View with an arithmetic value type");
  static_assert(
      std::is_same<ValueType, typename Kokkos::ViewTraits<
                                  T, P...>::non_const_value_type>::value,
      "iota requires a View with non-const value type");
  auto const n = v.extent(0);
  Kokkos::RangePolicy<std::decay_t<ExecutionSpace>> policy(
      std::forward<ExecutionSpace>(space), 0, n);
  Kokkos::parallel_for(ARBORX_MARK_REGION("iota"), policy,
                       KOKKOS_LAMBDA(int i) { v(i) = value + (ValueType)i; });
}

template <typename T, typename... P>
[[deprecated]] inline void
iota(Kokkos::View<T, P...> const &v,
     typename Kokkos::ViewTraits<T, P...>::value_type value = 0)
{
  using ExecutionSpace = typename Kokkos::ViewTraits<T, P...>::execution_space;
  iota(ExecutionSpace{}, v, value);
}

/** \brief Returns the smallest and the greatest element in the view
 *
 *  \param[in] space Execution space
 *  \param[in] v Input view
 *
 *  Returns a pair on the host with the smallest value in the view as the first
 *  element and the greatest as the second.
 */
template <typename ExecutionSpace, typename ViewType>
std::pair<typename ViewType::non_const_value_type,
          typename ViewType::non_const_value_type>
minMax(ExecutionSpace &&space, ViewType const &v)
{
  static_assert(ViewType::rank == 1, "minMax requires a View of rank 1");
  auto const n = v.extent(0);
  ARBORX_ASSERT(n > 0);
  Kokkos::MinMaxScalar<typename ViewType::non_const_value_type> result;
  Kokkos::MinMax<typename ViewType::non_const_value_type> reducer(result);
  Kokkos::RangePolicy<std::decay_t<ExecutionSpace>> policy(
      std::forward<ExecutionSpace>(space), 0, n);
  Kokkos::parallel_reduce(ARBORX_MARK_REGION("min_max"), policy,
                          Kokkos::Impl::min_max_functor<ViewType>(v), reducer);
  return std::make_pair(result.min_val, result.max_val);
}

template <typename ViewType>
[[deprecated]] inline std::pair<typename ViewType::non_const_value_type,
                                typename ViewType::non_const_value_type>
minMax(ViewType const &v)
{
  using ExecutionSpace = typename ViewType::execution_space;
  return minMax(ExecutionSpace{}, v);
}

/** \brief Returns the smallest element in the view
 *
 *  \param[in] space Execution space
 *  \param[in] v Input view
 */
template <typename ExecutionSpace, typename ViewType>
typename ViewType::non_const_value_type min(ExecutionSpace &&space,
                                            ViewType const &v)
{
  static_assert(ViewType::rank == 1, "min requires a View of rank 1");
  auto const n = v.extent(0);
  ARBORX_ASSERT(n > 0);
  typename ViewType::non_const_value_type result;
  Kokkos::Min<typename ViewType::non_const_value_type> reducer(result);
  Kokkos::RangePolicy<std::decay_t<ExecutionSpace>> policy(
      std::forward<ExecutionSpace>(space), 0, n);
  Kokkos::parallel_reduce(ARBORX_MARK_REGION("min"), policy,
                          KOKKOS_LAMBDA(int i, int &update) {
                            if (v(i) < update)
                              update = v(i);
                          },
                          reducer);
  return result;
}

template <typename ViewType>
[[deprecated]] inline typename ViewType::non_const_value_type
min(ViewType const &v)
{
  using ExecutionSpace = typename ViewType::execution_space;
  return min(ExecutionSpace{}, v);
}

/** \brief Returns the greatest element in the view
 *
 *  \param[in] space Execution space
 *  \param[in] v Input view
 */
template <typename ExecutionSpace, typename ViewType>
typename ViewType::non_const_value_type max(ExecutionSpace &&space,
                                            ViewType const &v)
{
  static_assert(ViewType::rank == 1, "max requires a View of rank 1");
  auto const n = v.extent(0);
  ARBORX_ASSERT(n > 0);
  typename ViewType::non_const_value_type result;
  Kokkos::Max<typename ViewType::non_const_value_type> reducer(result);
  Kokkos::RangePolicy<std::decay_t<ExecutionSpace>> policy(
      std::forward<ExecutionSpace>(space), 0, n);
  Kokkos::parallel_reduce(ARBORX_MARK_REGION("max"), policy,
                          KOKKOS_LAMBDA(int i, int &update) {
                            if (v(i) > update)
                              update = v(i);
                          },
                          reducer);
  return result;
}

template <typename ViewType>
[[deprecated]] inline typename ViewType::non_const_value_type
max(ViewType const &v)
{
  using ExecutionSpace = typename ViewType::execution_space;
  return max(ExecutionSpace{}, v);
}

/** \brief Accumulate values in a view
 *
 *  \param[in] space Execution space
 *  \param[in] v Input view
 *  \param[in] init Initial value of the sum
 *
 *  Returns the sum of the given \p init value and elements in the given view \p
 *  v.  Uses operator+ to sum up the elements.
 */
template <typename ExecutionSpace, typename ViewType>
typename ViewType::non_const_value_type
accumulate(ExecutionSpace &&space, ViewType const &v,
           typename ViewType::non_const_value_type init)
{
  static_assert(ViewType::rank == 1, "accumulate requires a View of rank 1");
  auto const n = v.extent(0);
  // NOTE: Passing the argument init directly to the parallel_reduce() while
  // using a lambda does not yield the expected result because Kokkos will
  // supply a default init method that sets the reduction result to zero.
  // Rather than going through the hassle of defining a custom functor for
  // the reduction, introduce here a temporary variable and add it to init
  // before returning.
  typename ViewType::non_const_value_type tmp = 0;
  Kokkos::RangePolicy<std::decay_t<ExecutionSpace>> policy(
      std::forward<ExecutionSpace>(space), 0, n);
  Kokkos::parallel_reduce(
      ARBORX_MARK_REGION("accumulate"), policy,
      KOKKOS_LAMBDA(int i, typename ViewType::non_const_value_type &update) {
        update += v(i);
      },
      tmp);
  init += tmp;
  return init;
}

template <typename ViewType>
[[deprecated]] inline typename ViewType::non_const_value_type
accumulate(ViewType const &v, typename ViewType::non_const_value_type init)
{
  using ExecutionSpace = typename ViewType::execution_space;
  return accumulate(ExecutionSpace{}, v, init);
}

// FIXME shameless forward declaration
template <typename View>
typename View::non_const_type clone(View &v);

/** \brief Computes the adjacent difference.
 *
 *  \param[in] space Execution space
 *  \param[in] src Input view
 *  \param[out] dst Output view; may not be equal to \p dst
 *
 *  Assigns to every element in the \p dst view the difference between its
 *  corresponding element and the one preceding it in the \p src view, except
 *  for the first element \c dst[0] which is assigned \c src[0]
 *
 *  \warning Undefined behavior if \p src and \p dst arrays overlap in any way.
 */
template <typename ExecutionSpace, typename SrcViewType, typename DstViewType>
void adjacentDifference(ExecutionSpace &&space, SrcViewType const &src,
                        DstViewType const &dst)
{
  static_assert(SrcViewType::rank == 1 && DstViewType::rank == 1,
                "adjacentDifference operates on rank-1 views");
  static_assert(std::is_same<typename DstViewType::value_type,
                             typename DstViewType::non_const_value_type>::value,
                "adjacentDifference requires non-const destination value type");
  static_assert(std::is_same<typename SrcViewType::non_const_value_type,
                             typename DstViewType::value_type>::value,
                "adjacentDifference requires same value type for source and "
                "destination");
  // QUESTION Should we assert anything about the memory spaces?
  auto const n = src.extent(0);
  ARBORX_ASSERT(n == dst.extent(0));
  ARBORX_ASSERT(src != dst);
  Kokkos::RangePolicy<std::decay_t<ExecutionSpace>> policy(
      std::forward<ExecutionSpace>(space), 0, n);
  Kokkos::parallel_for(ARBORX_MARK_REGION("adjacent_difference"), policy,
                       KOKKOS_LAMBDA(int i) {
                         if (i > 0)
                           dst(i) = src(i) - src(i - 1);
                         else
                           dst(i) = src(i);
                       });
}

template <typename SrcViewType, typename DstViewType>
[[deprecated]] inline void adjacentDifference(SrcViewType const &src,
                                              DstViewType const &dst)
{
  using ExecutionSpace = typename DstViewType::execution_space;
  adjacentDifference(ExecutionSpace{}, src, dst);
}

// FIXME split this into one for STL-like algorithms and another one for view
// utility helpers

// FIXME get rid of this when Trilinos/Kokkos version is updated
// clang-format off
#ifndef KOKKOS_IMPL_CTOR_DEFAULT_ARG
#  ifdef KOKKOS_ENABLE_DEPRECATED_CODE
#    define KOKKOS_IMPL_CTOR_DEFAULT_ARG 0
#  else
#    define KOKKOS_IMPL_CTOR_DEFAULT_ARG (~std::size_t(0))
#  endif
#endif
// clang-format on

// NOTE: not possible to avoid initialization with Kokkos::realloc()
template <typename View>
void reallocWithoutInitializing(View &v,
                                size_t n0 = KOKKOS_IMPL_CTOR_DEFAULT_ARG,
                                size_t n1 = KOKKOS_IMPL_CTOR_DEFAULT_ARG,
                                size_t n2 = KOKKOS_IMPL_CTOR_DEFAULT_ARG,
                                size_t n3 = KOKKOS_IMPL_CTOR_DEFAULT_ARG,
                                size_t n4 = KOKKOS_IMPL_CTOR_DEFAULT_ARG,
                                size_t n5 = KOKKOS_IMPL_CTOR_DEFAULT_ARG,
                                size_t n6 = KOKKOS_IMPL_CTOR_DEFAULT_ARG,
                                size_t n7 = KOKKOS_IMPL_CTOR_DEFAULT_ARG)
{
  static_assert(View::is_managed, "Can only realloc managed views");
  v = View(Kokkos::ViewAllocateWithoutInitializing(v.label()), n0, n1, n2, n3,
           n4, n5, n6, n7);
}

template <typename View>
void reallocWithoutInitializing(View &v,
                                const typename View::array_layout &layout)
{
  static_assert(View::is_managed, "Can only realloc managed views");
  v = View(Kokkos::ViewAllocateWithoutInitializing(v.label()), layout);
}

template <typename View>
typename View::non_const_type cloneWithoutInitializingNorCopying(View &v)
{
  return typename View::non_const_type(
      Kokkos::ViewAllocateWithoutInitializing(v.label()), v.layout());
}

template <typename ExecutionSpace, typename View>
typename View::non_const_type clone(ExecutionSpace &&space, View &v)
{
  typename View::non_const_type w(
      Kokkos::ViewAllocateWithoutInitializing(v.label()), v.layout());
  Kokkos::deep_copy(std::forward<ExecutionSpace>(space), w, v);
  return w;
}

template <typename View>
[[deprecated]] inline typename View::non_const_type clone(View &v)
{
  using ExecutionSpace = typename View::execution_space;
  return clone(ExecutionSpace{}, v);
}

} // namespace ArborX

#endif
