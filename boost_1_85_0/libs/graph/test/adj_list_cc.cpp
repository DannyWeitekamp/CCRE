//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_archetypes.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/concept/assert.hpp>

int main(int, char*[])
{
    using namespace boost;
    // Check adjacency_list with properties
    {
        typedef adjacency_list< vecS, vecS, directedS,
            property< vertex_color_t, int >, property< edge_weight_t, int > >
            Graph;
        typedef graph_traits< Graph >::vertex_descriptor Vertex;
        typedef graph_traits< Graph >::edge_descriptor Edge;
        BOOST_CONCEPT_ASSERT((VertexListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((AdjacencyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableIncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableEdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((VertexMutablePropertyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeMutablePropertyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT(
            (ReadablePropertyGraphConcept< Graph, Vertex, vertex_index_t >));
        BOOST_CONCEPT_ASSERT(
            (LvaluePropertyGraphConcept< Graph, Vertex, vertex_color_t >));
        BOOST_CONCEPT_ASSERT(
            (LvaluePropertyGraphConcept< Graph, Edge, edge_weight_t >));
    }
    {
        typedef adjacency_list< vecS, vecS, bidirectionalS,
            property< vertex_color_t, int >, property< edge_weight_t, int > >
            Graph;
        typedef graph_traits< Graph >::vertex_descriptor Vertex;
        typedef graph_traits< Graph >::edge_descriptor Edge;
        BOOST_CONCEPT_ASSERT((VertexListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((AdjacencyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((BidirectionalGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableBidirectionalGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableEdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((VertexMutablePropertyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeMutablePropertyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT(
            (ReadablePropertyGraphConcept< Graph, Vertex, vertex_index_t >));
        BOOST_CONCEPT_ASSERT(
            (LvaluePropertyGraphConcept< Graph, Vertex, vertex_color_t >));
        BOOST_CONCEPT_ASSERT(
            (LvaluePropertyGraphConcept< Graph, Edge, edge_weight_t >));
    }
    {
        typedef adjacency_list< listS, listS, directedS,
            property< vertex_color_t, int >, property< edge_weight_t, int > >
            Graph;
        typedef graph_traits< Graph >::vertex_descriptor Vertex;
        typedef graph_traits< Graph >::edge_descriptor Edge;
        BOOST_CONCEPT_ASSERT((VertexListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((AdjacencyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableIncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableEdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((VertexMutablePropertyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeMutablePropertyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT(
            (LvaluePropertyGraphConcept< Graph, Vertex, vertex_color_t >));
        BOOST_CONCEPT_ASSERT(
            (LvaluePropertyGraphConcept< Graph, Edge, edge_weight_t >));
    }
    {
        typedef adjacency_list< listS, listS, undirectedS,
            property< vertex_color_t, int >, property< edge_weight_t, int > >
            Graph;
        typedef graph_traits< Graph >::vertex_descriptor Vertex;
        typedef graph_traits< Graph >::edge_descriptor Edge;
        BOOST_CONCEPT_ASSERT((VertexListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((AdjacencyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableBidirectionalGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableEdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((VertexMutablePropertyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeMutablePropertyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT(
            (LvaluePropertyGraphConcept< Graph, Vertex, vertex_color_t >));
        BOOST_CONCEPT_ASSERT(
            (LvaluePropertyGraphConcept< Graph, Edge, edge_weight_t >));
    }
    // Checking adjacency_list with EdgeList=setS
    {
        typedef adjacency_list< setS, vecS, bidirectionalS,
            property< vertex_color_t, int >, property< edge_weight_t, int > >
            Graph;
        typedef graph_traits< Graph >::vertex_descriptor Vertex;
        typedef graph_traits< Graph >::edge_descriptor Edge;
        BOOST_CONCEPT_ASSERT((VertexListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((AdjacencyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((BidirectionalGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableBidirectionalGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableEdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((VertexMutablePropertyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeMutablePropertyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT(
            (ReadablePropertyGraphConcept< Graph, Vertex, vertex_index_t >));
        BOOST_CONCEPT_ASSERT(
            (LvaluePropertyGraphConcept< Graph, Vertex, vertex_color_t >));
        BOOST_CONCEPT_ASSERT(
            (LvaluePropertyGraphConcept< Graph, Edge, edge_weight_t >));
    }
    {
        typedef adjacency_list< setS, listS, directedS,
            property< vertex_color_t, int >, property< edge_weight_t, int > >
            Graph;
        typedef graph_traits< Graph >::vertex_descriptor Vertex;
        typedef graph_traits< Graph >::edge_descriptor Edge;
        BOOST_CONCEPT_ASSERT((VertexListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((AdjacencyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableIncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableEdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((VertexMutablePropertyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeMutablePropertyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT(
            (LvaluePropertyGraphConcept< Graph, Vertex, vertex_color_t >));
        BOOST_CONCEPT_ASSERT(
            (LvaluePropertyGraphConcept< Graph, Edge, edge_weight_t >));
    }
    {
        typedef adjacency_list< setS, listS, undirectedS,
            property< vertex_color_t, int >, property< edge_weight_t, int > >
            Graph;
        typedef graph_traits< Graph >::vertex_descriptor Vertex;
        typedef graph_traits< Graph >::edge_descriptor Edge;
        BOOST_CONCEPT_ASSERT((VertexListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((AdjacencyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableBidirectionalGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableEdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((VertexMutablePropertyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeMutablePropertyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT(
            (LvaluePropertyGraphConcept< Graph, Vertex, vertex_color_t >));
        BOOST_CONCEPT_ASSERT(
            (LvaluePropertyGraphConcept< Graph, Edge, edge_weight_t >));
    }
    // Check adjacency_list without any properties
    {
        typedef adjacency_list< vecS, vecS, directedS > Graph;
        typedef graph_traits< Graph >::vertex_descriptor Vertex;
        BOOST_CONCEPT_ASSERT((VertexListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((AdjacencyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableIncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableEdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((VertexMutablePropertyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeMutablePropertyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT(
            (ReadablePropertyGraphConcept< Graph, Vertex, vertex_index_t >));
    }
    {
        typedef adjacency_list< vecS, vecS, bidirectionalS > Graph;
        typedef graph_traits< Graph >::vertex_descriptor Vertex;
        BOOST_CONCEPT_ASSERT((VertexListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((AdjacencyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((BidirectionalGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableBidirectionalGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableEdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT(
            (ReadablePropertyGraphConcept< Graph, Vertex, vertex_index_t >));
    }
    {
        typedef adjacency_list< listS, listS, directedS > Graph;
        BOOST_CONCEPT_ASSERT((VertexListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((AdjacencyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableIncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableEdgeListGraphConcept< Graph >));
    }
    {
        typedef adjacency_list< listS, listS, undirectedS > Graph;
        BOOST_CONCEPT_ASSERT((VertexListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((AdjacencyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableBidirectionalGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableEdgeListGraphConcept< Graph >));
    }
    // Checking EdgeList=setS with no properties
    {
        typedef adjacency_list< setS, vecS, bidirectionalS > Graph;
        typedef graph_traits< Graph >::vertex_descriptor Vertex;
        BOOST_CONCEPT_ASSERT((VertexListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((AdjacencyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((BidirectionalGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableBidirectionalGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableEdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT(
            (ReadablePropertyGraphConcept< Graph, Vertex, vertex_index_t >));
    }
    {
        typedef adjacency_list< setS, listS, directedS > Graph;
        BOOST_CONCEPT_ASSERT((MutableIncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableEdgeListGraphConcept< Graph >));
    }
    {
        typedef adjacency_list< setS, listS, undirectedS > Graph;
        BOOST_CONCEPT_ASSERT((VertexListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((EdgeListGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((IncidenceGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((AdjacencyGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableBidirectionalGraphConcept< Graph >));
        BOOST_CONCEPT_ASSERT((MutableEdgeListGraphConcept< Graph >));
    }
    return 0;
}
