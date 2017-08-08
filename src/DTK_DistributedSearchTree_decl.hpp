/****************************************************************************
 * Copyright (c) 2012-2017 by the DataTransferKit authors                   *
 * All rights reserved.                                                     *
 *                                                                          *
 * This file is part of the DataTransferKit library. DataTransferKit is     *
 * distributed under a BSD 3-clause license. For the licensing terms see    *
 * the LICENSE file in the top-level directory.                             *
 ****************************************************************************/

#ifndef DTK_DISTRIBUTED_SEARCH_TREE_DECL_HPP
#define DTK_DISTRIBUTED_SEARCH_TREE_DECL_HPP

#include <Kokkos_View.hpp>

#include <Teuchos_Comm.hpp>
#include <Teuchos_RCP.hpp>

#include <DTK_DBC.hpp>
#include <DTK_LinearBVH.hpp>
#include <details/DTK_DetailsDistributedSearchTreeImpl.hpp>

#include "DTK_ConfigDefs.hpp"

namespace DataTransferKit
{

template <typename DeviceType>
class DistributedSearchTree
{
  public:
    DistributedSearchTree(
        Teuchos::RCP<Teuchos::Comm<int> const> comm,
        Kokkos::View<Box const *, DeviceType> bounding_boxes );

    template <typename Query>
    void query( Kokkos::View<Query *, DeviceType> queries,
                Kokkos::View<int *, DeviceType> &indices,
                Kokkos::View<int *, DeviceType> &offset,
                Kokkos::View<int *, DeviceType> &ranks ) const;

    template <typename Query>
    typename std::enable_if<
        std::is_same<typename Query::Tag, Details::NearestPredicateTag>::value,
        void>::type
    query( Kokkos::View<Query *, DeviceType> queries,
           Kokkos::View<int *, DeviceType> &indices,
           Kokkos::View<int *, DeviceType> &offset,
           Kokkos::View<int *, DeviceType> &ranks,
           Kokkos::View<double *, DeviceType> &distances ) const;

  private:
    Teuchos::RCP<Teuchos::Comm<int> const> _comm;
    BVH<DeviceType> _local_tree;
    std::shared_ptr<BVH<DeviceType>> _distributed_tree;
};

template <typename DeviceType>
template <typename Query>
void DistributedSearchTree<DeviceType>::query(
    Kokkos::View<Query *, DeviceType> queries,
    Kokkos::View<int *, DeviceType> &indices,
    Kokkos::View<int *, DeviceType> &offset,
    Kokkos::View<int *, DeviceType> &ranks ) const
{
    using Tag = typename Query::Tag;
    DistributedSearchTreeImpl<DeviceType>::query_dispatch(
        _comm, *_distributed_tree, _local_tree, queries, indices, offset, ranks,
        Tag{} );
}

template <typename DeviceType>
template <typename Query>
typename std::enable_if<
    std::is_same<typename Query::Tag, Details::NearestPredicateTag>::value,
    void>::type
DistributedSearchTree<DeviceType>::query(
    Kokkos::View<Query *, DeviceType> queries,
    Kokkos::View<int *, DeviceType> &indices,
    Kokkos::View<int *, DeviceType> &offset,
    Kokkos::View<int *, DeviceType> &ranks,
    Kokkos::View<double *, DeviceType> &distances ) const
{
    using Tag = typename Query::Tag;
    DistributedSearchTreeImpl<DeviceType>::query_dispatch(
        _comm, *_distributed_tree, _local_tree, queries, indices, offset, ranks,
        Tag{}, &distances );
}

} // end namespace DataTransferKit

#endif
