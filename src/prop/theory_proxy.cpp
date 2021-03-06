/*********************                                                        */
/*! \file theory_proxy.cpp
 ** \verbatim
 ** Original author: cconway
 ** Major contributors: kshitij, dejan, mdeters
 ** Minor contributors (to current version): barrett, taking, lianah
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009-2012  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief [[ Add one-line brief description here ]]
 **
 ** [[ Add lengthier description here ]]
 ** \todo document this file
 **/

#include "prop/cnf_stream.h"
#include "prop/prop_engine.h"
#include "prop/theory_proxy.h"
#include "context/context.h"
#include "theory/theory_engine.h"
#include "theory/rewriter.h"
#include "expr/expr_stream.h"
#include "decision/decision_engine.h"
#include "decision/options.h"
#include "util/lemma_input_channel.h"
#include "util/lemma_output_channel.h"
#include "util/statistics_registry.h"

namespace CVC4 {
namespace prop {

void TheoryProxy::variableNotify(SatVariable var) {
  d_theoryEngine->preRegister(getNode(SatLiteral(var)));
}

void TheoryProxy::theoryCheck(theory::Theory::Effort effort) {
  while (!d_queue.empty()) {
    TNode assertion = d_queue.front();
    d_queue.pop();
    d_theoryEngine->assertFact(assertion);
  }
  d_theoryEngine->check(effort);
}

void TheoryProxy::theoryPropagate(std::vector<SatLiteral>& output) {
  // Get the propagated literals
  std::vector<TNode> outputNodes;
  d_theoryEngine->getPropagatedLiterals(outputNodes);
  for (unsigned i = 0, i_end = outputNodes.size(); i < i_end; ++ i) {
    Debug("prop-explain") << "theoryPropagate() => " << outputNodes[i] << std::endl;
    output.push_back(d_cnfStream->getLiteral(outputNodes[i]));
  }
}

void TheoryProxy::explainPropagation(SatLiteral l, SatClause& explanation) {
  TNode lNode = d_cnfStream->getNode(l);
  Debug("prop-explain") << "explainPropagation(" << lNode << ")" << std::endl;
  Node theoryExplanation = d_theoryEngine->getExplanation(lNode);
  Debug("prop-explain") << "explainPropagation() => " <<  theoryExplanation << std::endl;
  if (theoryExplanation.getKind() == kind::AND) {
    Node::const_iterator it = theoryExplanation.begin();
    Node::const_iterator it_end = theoryExplanation.end();
    explanation.push_back(l);
    for (; it != it_end; ++ it) {
      explanation.push_back(~d_cnfStream->getLiteral(*it));
    }
  } else {
    explanation.push_back(l);
    explanation.push_back(~d_cnfStream->getLiteral(theoryExplanation));
  }
}

void TheoryProxy::enqueueTheoryLiteral(const SatLiteral& l) {
  Node literalNode = d_cnfStream->getNode(l);
  Debug("prop") << "enqueueing theory literal " << l << " " << literalNode << std::endl;
  Assert(!literalNode.isNull());
  d_queue.push(literalNode);
}

SatLiteral TheoryProxy::getNextDecisionRequest(bool &stopSearch) {
  TNode n = d_theoryEngine->getNextDecisionRequest();
  if(not n.isNull()) {
    return d_cnfStream->getLiteral(n);
  }

  // If theory doesn't give us a deicsion ask the decision engine. It
  // may return in undefSatLiteral in which case the sat solver uses
  // whatever default heuristic it has.
  Assert(d_decisionEngine != NULL);
  Assert(stopSearch != true);
  SatLiteral ret = d_decisionEngine->getNext(stopSearch);
  if(stopSearch) {
    Trace("decision") << "  ***  Decision Engine stopped search *** " << std::endl;
  }
  return options::decisionStopOnly() ? undefSatLiteral : ret;
}

bool TheoryProxy::theoryNeedCheck() const {
  return d_theoryEngine->needCheck();
}

TNode TheoryProxy::getNode(SatLiteral lit) {
  return d_cnfStream->getNode(lit);
}

void TheoryProxy::notifyRestart() {
  d_propEngine->checkTime();
  d_theoryEngine->notifyRestart();

  static uint32_t lemmaCount = 0;

  if(options::lemmaInputChannel() != NULL) {
    while(options::lemmaInputChannel()->hasNewLemma()) {
      Debug("shared") << "shared" << std::endl;
      Expr lemma = options::lemmaInputChannel()->getNewLemma();
      Node asNode = lemma.getNode();
      asNode = theory::Rewriter::rewrite(asNode);

      if(d_shared.find(asNode) == d_shared.end()) {
        d_shared.insert(asNode);
        if(asNode.getKind() == kind::OR) {
          ++lemmaCount;
          if(lemmaCount % 1 == 0) {
            Debug("shared") << "=) " << asNode << std::endl;
          }
          d_propEngine->assertLemma(d_theoryEngine->preprocess(asNode), false, true);
        } else {
          Debug("shared") << "=(" << asNode << std::endl;
        }
      } else {
        Debug("shared") <<"drop shared " << asNode << std::endl;
      }
    }
  }
}

void TheoryProxy::notifyNewLemma(SatClause& lemma) {
  Assert(lemma.size() > 0);
  if(options::lemmaOutputChannel() != NULL) {
    if(lemma.size() == 1) {
      // cannot share units yet
      //options::lemmaOutputChannel()->notifyNewLemma(d_cnfStream->getNode(lemma[0]).toExpr());
    } else {
      NodeBuilder<> b(kind::OR);
      for(unsigned i = 0, i_end = lemma.size(); i < i_end; ++i) {
        b << d_cnfStream->getNode(lemma[i]);
      }
      Node n = b;

      if(d_shared.find(n) == d_shared.end()) {
        d_shared.insert(n);
        options::lemmaOutputChannel()->notifyNewLemma(n.toExpr());
      } else {
        Debug("shared") <<"drop new " << n << std::endl;
      }
    }
  }
}

SatLiteral TheoryProxy::getNextReplayDecision() {
#ifdef CVC4_REPLAY
  if(options::replayStream() != NULL) {
    Expr e = options::replayStream()->nextExpr();
    if(!e.isNull()) { // we get null node when out of decisions to replay
      // convert & return
      ++d_replayedDecisions;
      return d_cnfStream->getLiteral(e);
    }
  }
#endif /* CVC4_REPLAY */
  return undefSatLiteral;
}

void TheoryProxy::logDecision(SatLiteral lit) {
#ifdef CVC4_REPLAY
  if(options::replayLog() != NULL) {
    Assert(lit != undefSatLiteral, "logging an `undef' decision ?!");
    *options::replayLog() << d_cnfStream->getNode(lit) << std::endl;
  }
#endif /* CVC4_REPLAY */
}

void TheoryProxy::checkTime() {
  d_propEngine->checkTime();
}

bool TheoryProxy::isDecisionRelevant(SatVariable var) {
  return d_decisionEngine->isRelevant(var);
}

bool TheoryProxy::isDecisionEngineDone() {
  return d_decisionEngine->isDone();
}

SatValue TheoryProxy::getDecisionPolarity(SatVariable var) {
  return d_decisionEngine->getPolarity(var);
}

}/* CVC4::prop namespace */
}/* CVC4 namespace */
