/*********************                                                        */
/*! \file node_value.h
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: dejan
 ** Minor contributors (to current version): cconway, taking
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief An expression node.
 **
 ** An expression node.
 **
 ** Instances of this class are generally referenced through
 ** cvc4::Node rather than by pointer; cvc4::Node maintains the
 ** reference count on NodeValue instances and
 **/

#include "cvc4_private.h"

// circular dependency
#include "expr/metakind.h"

#ifndef __CVC4__EXPR__NODE_VALUE_H
#define __CVC4__EXPR__NODE_VALUE_H

#include "expr/kind.h"

#include <stdint.h>
#include <string>
#include <iterator>

namespace CVC4 {

template <bool ref_count> class NodeTemplate;
class TypeNode;
template <unsigned N> class NodeBuilder;
class AndNodeBuilder;
class OrNodeBuilder;
class PlusNodeBuilder;
class MultNodeBuilder;
class NodeManager;

namespace kind {
  namespace metakind {
    template < ::CVC4::Kind k, bool pool >
    struct NodeValueConstCompare;

    struct NodeValueCompare;
    struct NodeValueConstPrinter;
  }/* CVC4::kind::metakind namespace */
}/* CVC4::kind namespace */

namespace expr {

#if __CVC4__EXPR__NODE_VALUE__NBITS__REFCOUNT + \
    __CVC4__EXPR__NODE_VALUE__NBITS__KIND + \
    __CVC4__EXPR__NODE_VALUE__NBITS__ID + \
    __CVC4__EXPR__NODE_VALUE__NBITS__NCHILDREN != 64
#  error NodeValue header bit assignment does not sum to 64 !
#endif /* sum != 64 */

/**
 * This is an NodeValue.
 */
class NodeValue {

  /** A convenient null-valued expression value */
  static NodeValue s_null;

  static const unsigned NBITS_REFCOUNT = __CVC4__EXPR__NODE_VALUE__NBITS__REFCOUNT;
  static const unsigned NBITS_KIND = __CVC4__EXPR__NODE_VALUE__NBITS__KIND;
  static const unsigned NBITS_ID = __CVC4__EXPR__NODE_VALUE__NBITS__ID;
  static const unsigned NBITS_NCHILDREN = __CVC4__EXPR__NODE_VALUE__NBITS__NCHILDREN;

  /** Maximum reference count possible.  Used for sticky
   *  reference-counting.  Should be (1 << num_bits(d_rc)) - 1 */
  static const unsigned MAX_RC = (1u << NBITS_REFCOUNT) - 1;

  /** A mask for d_kind */
  static const unsigned kindMask = (1u << NBITS_KIND) - 1;

  // this header fits into one 64-bit word

  /** The ID (0 is reserved for the null value) */
  unsigned d_id        : NBITS_ID;

  /** The expression's reference count.  @see cvc4::Node. */
  unsigned d_rc        : NBITS_REFCOUNT;

  /** Kind of the expression */
  unsigned d_kind      : NBITS_KIND;

  /** Number of children */
  unsigned d_nchildren : NBITS_NCHILDREN;

  /** Variable number of child nodes */
  NodeValue* d_children[0];

  // todo add exprMgr ref in debug case

  template <bool> friend class ::CVC4::NodeTemplate;
  friend class ::CVC4::TypeNode;
  template <unsigned nchild_thresh> friend class ::CVC4::NodeBuilder;
  friend class ::CVC4::NodeManager;

  template <Kind k, bool pool>
  friend struct ::CVC4::kind::metakind::NodeValueConstCompare; 

  friend struct ::CVC4::kind::metakind::NodeValueCompare;
  friend struct ::CVC4::kind::metakind::NodeValueConstPrinter;

  void inc();
  void dec();

  static size_t next_id;

public:
  /**
   * Uninitializing constructor for NodeBuilder's use.  This is
   * somewhat dangerous, but must also be public for the
   * makeStackNodeBuilder() macro to work.
   */
  NodeValue() { /* do not initialize! */ }

private:
  /** Private constructor for the null value. */
  NodeValue(int);

  typedef NodeValue** nv_iterator;
  typedef NodeValue const* const* const_nv_iterator;

  nv_iterator nv_begin();
  nv_iterator nv_end();

  const_nv_iterator nv_begin() const;
  const_nv_iterator nv_end() const;

  template <typename T>
  class iterator {
    const_nv_iterator d_i;
  public:
    typedef T value_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef T& reference;

    iterator() : d_i(NULL) {}
    explicit iterator(const_nv_iterator i) : d_i(i) {}

    inline T operator*();

    bool operator==(const iterator& i) {
      return d_i == i.d_i;
    }

    bool operator!=(const iterator& i) {
      return d_i != i.d_i;
    }

    iterator operator++() {
      ++d_i;
      return *this;
    }

    iterator operator++(int) {
      return iterator(d_i++);
    }

    typedef std::input_iterator_tag iterator_category;
  };

  /** Decrement ref counts of children */
  inline void decrRefCounts();

  bool isBeingDeleted() const;

public:

  template <typename T>
  inline iterator<T> begin() const;

  template <typename T>
  inline iterator<T> end() const;

  /**
   * Hash this NodeValue.  For hash_maps, hash_sets, etc.. but this is
   * for expr package internal use only at present!  This is likely to
   * be POORLY PERFORMING for other uses!  For example, this gives
   * collisions for all VARIABLEs.
   * @return the hash value of this expression.
   */
  size_t poolHash() const {
    if(getMetaKind() == kind::metakind::CONSTANT) {
      return kind::metakind::NodeValueCompare::constHash(this);
    }

    size_t hash = d_kind;
    const_nv_iterator i = nv_begin();
    const_nv_iterator i_end = nv_end();
    while(i != i_end) {
      hash ^= (*i)->d_id + 0x9e3779b9 + (hash << 6) + (hash >> 2);
      ++i;
    }
    return hash;
  }

  unsigned getId() const { return d_id; }
  Kind getKind() const { return dKindToKind(d_kind); }
  kind::MetaKind getMetaKind() const { return kind::metaKindOf(getKind()); }
  unsigned getNumChildren() const {
    return (getMetaKind() == kind::metakind::PARAMETERIZED)
      ? d_nchildren - 1
      : d_nchildren;
  }

  std::string toString() const;
  void toStream(std::ostream& out, int toDepth = -1, bool types = false) const;

  static inline unsigned kindToDKind(Kind k) {
    return ((unsigned) k) & kindMask;
  }

  static inline Kind dKindToKind(unsigned d) {
    return (d == kindMask) ? kind::UNDEFINED_KIND : Kind(d);
  }

  static inline const NodeValue& null() {
    return s_null;
  }

  /**
   * If this is a CONST_* Node, extract the constant from it.
   */
  template <class T>
  inline const T& getConst() const;

  NodeValue* getChild(int i) const;

  void printAst(std::ostream& out, int indent = 0) const;

private:

  /**
   * Indents the given stream a given amount of spaces.
   * @param out the stream to indent
   * @param indent the numer of spaces
   */
  static inline void indent(std::ostream& out, int indent) {
    for(int i = 0; i < indent; i++) {
      out << ' ';
    }
  }

};/* class NodeValue */

/**
 * For hash_maps, hash_sets, etc.. but this is for expr package
 * internal use only at present!  This is likely to be POORLY
 * PERFORMING for other uses!  NodeValue::poolHash() will lead to
 * collisions for all VARIABLEs.
 */
struct NodeValuePoolHashFcn {
  inline size_t operator()(const NodeValue* nv) const {
    return (size_t) nv->poolHash();
  }
};/* struct NodeValuePoolHashFcn */

/**
 * For hash_maps, hash_sets, etc.
 */
struct NodeValueIDHashFunction {
  inline size_t operator()(const NodeValue* nv) const {
    return (size_t) nv->getId();
  }
};/* struct NodeValueIDHashFcn */

inline std::ostream& operator<<(std::ostream& out, const NodeValue& nv);

}/* CVC4::expr namespace */
}/* CVC4 namespace */

#include "expr/node_manager.h"
#include "expr/type_node.h"

namespace CVC4 {
namespace expr {

inline NodeValue::NodeValue(int) :
  d_id(0),
  d_rc(MAX_RC),
  d_kind(kind::NULL_EXPR),
  d_nchildren(0) {
}

inline void NodeValue::decrRefCounts() {
  for(nv_iterator i = nv_begin(); i != nv_end(); ++i) {
    (*i)->dec();
  }
}

inline void NodeValue::inc() {
  Assert(!isBeingDeleted(),
         "NodeValue is currently being deleted "
         "and increment is being called on it. Don't Do That!");
  // FIXME multithreading
  if(EXPECT_TRUE( d_rc < MAX_RC )) {
    ++d_rc;
  }
}

inline void NodeValue::dec() {
  // FIXME multithreading
  if(EXPECT_TRUE( d_rc < MAX_RC )) {
    --d_rc;
    if(EXPECT_FALSE( d_rc == 0 )) {
      Assert(NodeManager::currentNM() != NULL,
             "No current NodeManager on destruction of NodeValue: "
             "maybe a public CVC4 interface function is missing a NodeManagerScope ?");
      NodeManager::currentNM()->markForDeletion(this);
    }
  }
}

inline NodeValue::nv_iterator NodeValue::nv_begin() {
  return d_children;
}

inline NodeValue::nv_iterator NodeValue::nv_end() {
  return d_children + d_nchildren;
}

inline NodeValue::const_nv_iterator NodeValue::nv_begin() const {
  return d_children;
}

inline NodeValue::const_nv_iterator NodeValue::nv_end() const {
  return d_children + d_nchildren;
}

template <typename T>
inline NodeValue::iterator<T> NodeValue::begin() const {
  NodeValue* const* firstChild = d_children;
  if(getMetaKind() == kind::metakind::PARAMETERIZED) {
    ++firstChild;
  }
  return iterator<T>(firstChild);
}

template <typename T>
inline NodeValue::iterator<T> NodeValue::end() const {
  return iterator<T>(d_children + d_nchildren);
}

inline bool NodeValue::isBeingDeleted() const {
  return NodeManager::currentNM() != NULL &&
    NodeManager::currentNM()->isCurrentlyDeleting(this);
}

inline NodeValue* NodeValue::getChild(int i) const {
  if(getMetaKind() == kind::metakind::PARAMETERIZED) {
    ++i;
  }

  Assert(i >= 0 && unsigned(i) < d_nchildren);
  return d_children[i];
}

}/* CVC4::expr namespace */
}/* CVC4 namespace */

#include "expr/node.h"
#include "expr/type_node.h"

namespace CVC4 {
namespace expr {

template <typename T>
inline T NodeValue::iterator<T>::operator*() {
  return T(*d_i);
}

inline std::ostream& operator<<(std::ostream& out, const NodeValue& nv) {
  nv.toStream(out,
              Node::setdepth::getDepth(out),
              Node::printtypes::getPrintTypes(out));
  return out;
}

}/* CVC4::expr namespace */

#ifdef CVC4_DEBUG
/**
 * Pretty printer for use within gdb.  This is not intended to be used
 * outside of gdb.  This writes to the Warning() stream and immediately
 * flushes the stream.
 */
static void __attribute__((used)) debugPrintNodeValue(const expr::NodeValue* nv) {
  Warning() << Node::setdepth(-1) << *nv << std::endl;
  Warning().flush();
}

static void __attribute__((used)) debugPrintRawNodeValue(const expr::NodeValue* nv) {
  nv->printAst(Warning(), 0);
  Warning().flush();
}
#endif /* CVC4_DEBUG */

}/* CVC4 namespace */

#endif /* __CVC4__EXPR__NODE_VALUE_H */
