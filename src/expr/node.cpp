/*********************                                           -*- C++ -*-  */
/** node.cpp
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009 The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.
 **
 ** Reference-counted encapsulation of a pointer to an expression.
 **/

#include "expr/node.h"
#include "expr/node_value.h"
#include "expr/node_builder.h"
#include "util/Assert.h"

#include <sstream>

using namespace CVC4::expr;
using namespace std;

namespace CVC4 {

NodeValue NodeValue::s_null;

Node Node::s_null(&NodeValue::s_null);

Node Node::null() {
  return s_null;
}

bool Node::isNull() const {
  return d_ev == &NodeValue::s_null;
}

Node::Node() :
  d_ev(&NodeValue::s_null) {
  // No refcount needed
}

// FIXME: escape from type system convenient but is there a better
// way?  Nodes conceptually don't change their expr values but of
// course they do modify the refcount.  But it's nice to be able to
// support node_iterators over const NodeValue*.  So.... hm.
Node::Node(const NodeValue* ev)
  : d_ev(const_cast<NodeValue*>(ev)) {
  Assert(d_ev != NULL, "Expecting a non-NULL expression value!");
  d_ev->inc();
}

Node::Node(const Node& e) {
  Assert(e.d_ev != NULL, "Expecting a non-NULL expression value!");
  d_ev = e.d_ev;
  d_ev->inc();
}

Node::~Node() {
  Assert(d_ev != NULL, "Expecting a non-NULL expression value!");
  d_ev->dec();
}

void Node::assignNodeValue(NodeValue* ev) {
  d_ev = ev;
  d_ev->inc();
}

Node& Node::operator=(const Node& e) {
  Assert(d_ev != NULL, "Expecting a non-NULL expression value!");
  if(this != &e && d_ev != e.d_ev) {
    d_ev->dec();
    d_ev = e.d_ev;
    d_ev->inc();
  }
  return *this;
}

NodeValue const* Node::operator->() const {
  Assert(d_ev != NULL, "Expecting a non-NULL expression value!");
  return d_ev;
}

uint64_t Node::hash() const {
  Assert(d_ev != NULL, "Expecting a non-NULL expression value!");
  return d_ev->hash();
}

Node Node::eqExpr(const Node& right) const {
  return NodeManager::currentNM()->mkExpr(EQUAL, *this, right);
}

Node Node::notExpr() const {
  return NodeManager::currentNM()->mkExpr(NOT, *this);
}

Node Node::andExpr(const Node& right) const {
  return NodeManager::currentNM()->mkExpr(AND, *this, right);
}

Node Node::orExpr(const Node& right) const {
  return NodeManager::currentNM()->mkExpr(OR, *this, right);
}

Node Node::iteExpr(const Node& thenpart, const Node& elsepart) const {
  return NodeManager::currentNM()->mkExpr(ITE, *this, thenpart, elsepart);
}

Node Node::iffExpr(const Node& right) const {
  return NodeManager::currentNM()->mkExpr(IFF, *this, right);
}

Node Node::impExpr(const Node& right) const {
  return NodeManager::currentNM()->mkExpr(IMPLIES, *this, right);
}

Node Node::xorExpr(const Node& right) const {
  return NodeManager::currentNM()->mkExpr(XOR, *this, right);
}

}/* CVC4 namespace */