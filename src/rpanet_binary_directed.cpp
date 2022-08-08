#include<iostream>
#include<queue>
#include<deque>
#include<algorithm>
#include<R.h>
#include "funcPtrD.h"
#include<RcppArmadillo.h>
// [[Rcpp::depends(RcppArmadillo)]]

using namespace std;

/**
 * Node structure in directed networks.
 * id: node id
 * outs, ins: out- and in-strength
 * sourcep: preference of being chosen as a source node
 * targetp: preference of being chosed as a target node
 * total_sourcep: sum of sourcep of current node and its children
 * total_targetp: sum of targetp of current node and its children
 * *left, *right, *parent: pointers to its left, right and parent
 */
struct node_d {
  int id, group;
  double outs, ins;
  double sourcep, targetp, total_sourcep, total_targetp;
  node_d *left, *right, *parent;
};

/**
 * Default source preference function.
 * 
 * @param outs Node out-strength.
 * @param ins Node in-strength.
 * @param sparams Parameters passed to the source preference function.
 * 
 * @return Source preference of a node.
 */
double sourcePrefFuncDefault(double outs, double ins, Rcpp::NumericVector sparams) {
  return sparams[0] * pow(outs, sparams[1]) + 
    sparams[2] * pow(ins, sparams[3]) + sparams[4];
}

/**
 * Default target preference function.
 * 
 * @param outs Node out-strength.
 * @param ins Node in-strength.
 * @param tparams Parameters passed to the target preference function.
 * @return Target preference of a node.
 */
double targetPrefFuncDefault(double outs, double ins, Rcpp::NumericVector tparams) {
  return tparams[0] * pow(outs, tparams[1]) + 
    tparams[2] * pow(ins, tparams[3]) + tparams[4];
}

/**
 * Update total source preference from current node to root.
 * 
 * @param current_node The current node.
 */
void updateTotalSourcep(node_d *current_node) {
  if (current_node->left == NULL) {
    current_node->total_sourcep = current_node->sourcep;
  }
  else if (current_node->right == NULL) {
    current_node->total_sourcep = current_node->sourcep + current_node->left->total_sourcep;
  }
  else {
    current_node->total_sourcep = current_node->sourcep + current_node->left->total_sourcep + current_node->right->total_sourcep;
  }
  while(current_node->id > 0) {
    return updateTotalSourcep(current_node->parent);
  }
}

/**
 * Update total target preference from current node to root.
 * 
 * @param current_node The current node.
 */
void updateTotalTargetp(node_d *current_node) {
  if (current_node->left == NULL) {
    current_node->total_targetp = current_node->targetp;
  }
  else if (current_node->right == NULL) {
    current_node->total_targetp = current_node->targetp + current_node->left->total_targetp;
  }
  else {
    current_node->total_targetp = current_node->targetp + current_node->left->total_targetp + current_node->right->total_targetp;
  }
  while(current_node->id > 0) {
    return updateTotalTargetp(current_node->parent);
  }
}

/**
 * Update node preference and total preference from the sampled node to root.
 * 
 * @param temp_node The sampled node.
 * @param func_type Default or customized preference function.
 * @param sparams Parameters passed to the default source preference function.
 * @param tparams Parameters passed to the default target preference function.
 * @param sourcePrefFuncCpp Pointer of customized source preference function.
 * @param targetPrefFuncCpp Pointer of customized target preference function.
 */
void updatePrefD(node_d *temp_node, int func_type, 
                 Rcpp::NumericVector sparams, Rcpp::NumericVector tparams,
                 funcPtrD sourcePrefFuncCpp, 
                 funcPtrD targetPrefFuncCpp) {
  double temp_sourcep = temp_node->sourcep, temp_targetp = temp_node->targetp;
  if (func_type == 1) {
    temp_node->sourcep = sourcePrefFuncDefault(temp_node->outs, temp_node->ins, sparams);
    temp_node->targetp = targetPrefFuncDefault(temp_node->outs, temp_node->ins, tparams);
  }
  else {
    temp_node->sourcep = sourcePrefFuncCpp(temp_node->outs, temp_node->ins);
    temp_node->targetp = targetPrefFuncCpp(temp_node->outs, temp_node->ins);
  }
  
  if (temp_node->sourcep != temp_sourcep) {
    updateTotalSourcep(temp_node);
  }
  if (temp_node->targetp != temp_targetp) {
    updateTotalTargetp(temp_node);
  }
}

/**
 * Create a new node.
 * 
 * @param id Node ID.
 * 
 * @return The new node.
 */
node_d *createNodeD(int id) {
  node_d *new_node = new node_d();
  new_node->id = id;
  new_node->group = -1;
  new_node->outs = new_node->ins = 0;
  new_node->sourcep = new_node->total_sourcep = 0;
  new_node->targetp = new_node->total_targetp = 0;
  new_node->left = new_node->right = new_node->parent = NULL;
  return new_node;
}

/**
 * Insert a new node to the tree.
 * 
 * @param q Sequence of nodes that have less than 2 children.
 * @param new_node_id New node ID.
 * 
 * @return The new node.
 */
node_d *insertNodeD(queue<node_d*> &q, int new_node_id) {
  node_d *new_node = createNodeD(new_node_id);
  node_d *temp_node = q.front();
  if(temp_node->left == NULL) {
    temp_node->left = new_node;
  }
  else if (temp_node->right == NULL) {
    temp_node->right = new_node;
    q.pop();
  }
  new_node->parent = temp_node;
  q.push(new_node);
  return new_node;
}

/**
 * Find a source node with a given cutoff point w.
 * 
 * @param root Root node of the tree.
 * @param w A cutoff point.
 * 
 * @return Sampled source/target node.
 */
node_d *findSourceNode(node_d *root, double w) {
  if (w > root->total_sourcep) {
    // numerical error
    // Rprintf("Numerical error. Diff %f.\n", (w - root->total_sourcep) * pow(10, 10));
    w = root->total_sourcep;
  }
  w -= root->sourcep;
  if (w <= 0) {
    return root;
  } 
  else {
    if (w > root->left->total_sourcep) {
      return findSourceNode(root->right, w - root->left->total_sourcep);
    }
    else {
      return findSourceNode(root->left, w);
    }
  }
}

/**
 * Find a target node with a given cutoff point w.
 * 
 * @param root Root node of the tree.
 * @param w A cutoff point.
 * 
 * @return Sampled source/target node.
 */
node_d *findTargetNode(node_d *root, double w) {
  if (w > root->total_targetp) {
    // numerical error
    // Rprintf("Numerical error. Diff %f.\n", (w - root->total_targetp) * pow(10, 10));
    w = root->total_targetp;
  }
  w -= root->targetp;
  if (w <= 0) {
    return root;
  }
  else {
    if (w > root->left->total_targetp) {
      return findTargetNode(root->right, w - root->left->total_targetp);
    }
    else {
      return findTargetNode(root->left, w);
    }
  }
}

/**
 * Sample a source/target node from the tree.
 *
 * @param root Root node of the tree.
 * @param type Represent source node or target node.
 * @param qm Nodes to be excluded from the sampling process.
 *
 * @return Sampled source/target node.
 */
node_d* sampleNodeD(node_d *root, char type, deque<node_d*> &qm) {
  double w;
  node_d *temp_node;
  while (true) {
    w = 1;
    while (w == 1) {
      w = unif_rand();
    }
    if (type == 's') {
      w *= root->total_sourcep;
      temp_node = findSourceNode(root, w);
    }
    else {
      w *= root->total_targetp;
      temp_node = findTargetNode(root, w);
    }
    if (find(qm.begin(), qm.end(), temp_node) == qm.end()) {
      // if temp_node not in qm
      return temp_node;
    }
  }
}

/**
 * Sample a node group.
 * 
 * @param group_prob Probability weights for sampling the group of new nodes.
 * 
 * @return Sampled group for the new node.
 */
int sampleGroup(Rcpp::NumericVector group_prob) {
  double g = 0;
  int i = 0;
  while ((g == 0) || (g == 1)) {
    g = unif_rand();
  }
  while (g > 0) {
    g -= group_prob[i];
    i++;
  }
  return i - 1;
}

//' Preferential attachment algorithm.
//'
//' @param nstep Number of steps.
//' @param m Number of new edges in each step.
//' @param new_node_id New node ID.
//' @param new_edge_id New edge ID.
//' @param source_node Sequence of source nodes.
//' @param target_node Sequence of target nodes.
//' @param outs Sequence of out-strength.
//' @param ins Sequence of in-strength.
//' @param edgeweight Weight of existing and new edges.
//' @param scenario Scenario of existing and new edges.
//' @param sample_recip Logical, whether reciprocal edges will be added.
//' @param node_group Sequence of node group.
//' @param source_pref Sequence of node source preference.
//' @param target_pref Sequence of node target preference.
//' @param control List of controlling arguments.
//' @return Sampled network.
//'
// [[Rcpp::export]]
Rcpp::List rpanet_binary_directed(
    int nstep, 
    Rcpp::IntegerVector m, 
    int new_node_id, 
    int new_edge_id, 
    Rcpp::IntegerVector source_node, 
    Rcpp::IntegerVector target_node, 
    Rcpp::NumericVector outs, 
    Rcpp::NumericVector ins, 
    Rcpp::NumericVector edgeweight, 
    Rcpp::IntegerVector scenario,
    bool sample_recip, 
    Rcpp::IntegerVector node_group, 
    Rcpp::NumericVector source_pref, 
    Rcpp::NumericVector target_pref, 
    Rcpp::List control) {
  Rcpp::List scenario_ctl = control["scenario"];
  double alpha = scenario_ctl["alpha"];
  double beta = scenario_ctl["beta"];
  double gamma = scenario_ctl["gamma"];
  double xi = scenario_ctl["xi"];
  bool beta_loop = scenario_ctl["beta.loop"];
  bool source_first = scenario_ctl["source.first"];
  Rcpp::List newedge_ctl = control["newedge"];
  bool node_unique = ! newedge_ctl["node.replace"];
  bool snode_unique = ! newedge_ctl["snode.replace"];
  bool tnode_unique = ! newedge_ctl["tnode.replace"];
  Rcpp::List reciprocal_ctl = control["reciprocal"];
  bool selfloop_recip = reciprocal_ctl["selfloop.recip"];
  Rcpp::NumericVector group_prob = reciprocal_ctl["group.prob"];
  Rcpp::NumericMatrix recip_prob = reciprocal_ctl["recip.prob"];
  Rcpp::List preference_ctl = control["preference"];
  Rcpp::NumericVector sparams(5);
  Rcpp::NumericVector tparams(5);
  funcPtrD sourcePrefFuncCpp;
  funcPtrD targetPrefFuncCpp;
  // different types of preference functions
  int func_type = preference_ctl["ftype.temp"];
  switch (func_type) {
  case 1: 
    sparams = preference_ctl["sparams"];
    tparams = preference_ctl["tparams"];
    break;
  case 2: {
      SEXP source_pref_func_ptr = preference_ctl["spref.pointer"];
      Rcpp::XPtr<funcPtrD> xpfunSource(source_pref_func_ptr);
      sourcePrefFuncCpp = *xpfunSource;
      SEXP target_pref_func_ptr = preference_ctl["tpref.pointer"];
      Rcpp::XPtr<funcPtrD> xpfunTarget(target_pref_func_ptr);
      targetPrefFuncCpp = *xpfunTarget;
      break;
    }
  }
  
  double u, p;
  bool check_unique = node_unique || snode_unique || tnode_unique;
  bool m_error;
  int i, j, ks, kt, n_existing, current_scenario;
  node_d *node1, *node2;
  // initialize a tree from the seed graph
  node_d *root = createNodeD(0);
  root->outs = outs[0];
  root->ins = ins[0];
  root->group = node_group[0];
  updatePrefD(root, func_type, sparams, tparams, sourcePrefFuncCpp, targetPrefFuncCpp);
  queue<node_d*> q, q1;
  deque<node_d*> qm_source, qm_target;
  q.push(root);
  for (int i = 1; i < new_node_id; i++) {
    node1 = insertNodeD(q, i);
    node1->outs = outs[i];
    node1->ins = ins[i];
    node1->group = node_group[i];
    updatePrefD(node1, func_type, sparams, tparams, sourcePrefFuncCpp, targetPrefFuncCpp);
  }
  // sample edges
  GetRNGstate();
  for (i = 0; i < nstep; i++) {
    m_error = false;
    n_existing = new_node_id;
    for (j = 0; j < m[i]; j++) {
      u = unif_rand();
      ks = qm_source.size();
      kt = qm_target.size();
      if (u <= alpha) {
        current_scenario = 1;
      }
      else if (u <= alpha + beta) {
        current_scenario = 2;
      }
      else if (u <= alpha + beta + gamma) {
        current_scenario = 3;
      }
      else if (u <= alpha + beta + gamma + xi) {
        current_scenario = 4;
      }
      else {
        current_scenario = 5;
      }
      if (check_unique) {
        switch (current_scenario) {
        case 1:
          if (kt + 1 > n_existing) {
            m_error = true;
          }
          break;
        case 2:
          if (node_unique) {
            if (ks + 2 - int(beta_loop) > n_existing) {
              m_error = true;
            }
          }
          else {
            if (snode_unique) {
              if (ks + 1 > n_existing) {
                m_error = true;
              }
            }
            if (tnode_unique) {
              if (kt + 1 > n_existing) {
                m_error = true;
              }
            }
          }
          break;
        case 3:
          if (ks + 1 > n_existing) {
            m_error = true;
          }
          break;
        }
      }
      if (m_error) {
        break;
      }
      switch (current_scenario) {
      case 1:
        node1 = insertNodeD(q, new_node_id);
        if (sample_recip) {
          node1->group = sampleGroup(group_prob);
        }
        new_node_id++;
        node2 = sampleNodeD(root, 't', qm_target);
        break;
      case 2:
        if (source_first) {
          node1 = sampleNodeD(root, 's', qm_source);
          if (beta_loop) {
            node2 = sampleNodeD(root, 't', qm_target);
          }
          else {
            if (find(qm_target.begin(), qm_target.end(), node1) != qm_target.end()) {
              node2 = sampleNodeD(root, 't', qm_target);
            }
            else {
              if (kt + 2 > n_existing) {
                m_error = true;
                break;
              }
              qm_target.push_back(node1);
              node2 = sampleNodeD(root, 't', qm_target);
              qm_target.pop_back();
            }
          }
        }
        else {
          node2 = sampleNodeD(root, 't', qm_target);
          if (beta_loop) {
            node1 = sampleNodeD(root, 's', qm_source);
          }
          else {
            if (find(qm_source.begin(), qm_source.end(), node2) != qm_source.end()) {
              node1 = sampleNodeD(root, 's', qm_source);
            }
            else {
              if (ks + 2 > n_existing) {
                m_error = true;
                break;
              }
              qm_source.push_back(node2);
              node1 = sampleNodeD(root, 's', qm_source);
              qm_source.pop_back();
            }
          }
        }
        break;
      case 3:
        node1 = sampleNodeD(root, 's', qm_source);
        node2 = insertNodeD(q, new_node_id);
        if (sample_recip) {
          node2->group = sampleGroup(group_prob);
        }
        new_node_id++;
        break;
      case 4:
        node1 = insertNodeD(q, new_node_id);
        new_node_id++;
        node2 = insertNodeD(q, new_node_id);
        new_node_id++;
        if (sample_recip) {
          node1->group = sampleGroup(group_prob);
          node2->group = sampleGroup(group_prob);
        }
        break;
      case 5:
        node1 = node2 = insertNodeD(q, new_node_id);
        if (sample_recip) {
          node1->group = sampleGroup(group_prob);
        }
        new_node_id++;
        break;
      }
      if (m_error) {
        break;
      }
      // handle duplicate nodes
      if (node_unique) {
        if (node1->id < n_existing) {
          qm_source.push_back(node1);
          qm_target.push_back(node1);
        }
        if ((node2->id < n_existing) && (node1 != node2)) {
          qm_source.push_back(node2);
          qm_target.push_back(node2);
        }
      }
      else {
        if (snode_unique && (node1->id < n_existing)) {
          qm_source.push_back(node1);
        }
        if (tnode_unique && (node2->id < n_existing)) {
          qm_target.push_back(node2);
        }
      }
      node1->outs += edgeweight[new_edge_id];
      node2->ins += edgeweight[new_edge_id];
      source_node[new_edge_id] = node1->id;
      target_node[new_edge_id] = node2->id;
      scenario[new_edge_id] = current_scenario;
      q1.push(node1);
      q1.push(node2);
      // handle reciprocal
      if (sample_recip) {
        if ((node1->id != node2->id) || selfloop_recip) {
          p = unif_rand();
          if (p <= recip_prob(node2->group, node1->group)) {
            new_edge_id++;
            node2->outs += edgeweight[new_edge_id];
            node1->ins += edgeweight[new_edge_id];
            source_node[new_edge_id] = node2->id;
            target_node[new_edge_id] = node1->id;
            scenario[new_edge_id] = 6;
          }
        }
      }
      new_edge_id++;
    }
    if (m_error) {
      m[i] = j;
      Rprintf("Unique nodes exhausted at step %u. Set the value of m at current step to %u.\n", i + 1, j);
    }
    while(! q1.empty()) {
      updatePrefD(q1.front(), func_type, sparams, tparams, sourcePrefFuncCpp, targetPrefFuncCpp);
      q1.pop();
    }
    qm_source.clear();
    qm_target.clear();
  }
  PutRNGstate();
  // free memory (queue)
  queue<node_d*>().swap(q);
  queue<node_d*>().swap(q1);
  // save strength and preference
  q.push(root);
  node_d *temp_node;
  j = 0;
  while (! q.empty()) {
    temp_node = q.front();
    q.pop();
    if (temp_node->right != NULL) {
      q.push(temp_node->left);
      q.push(temp_node->right);
    }
    else if (temp_node->left != NULL) {
      q.push(temp_node->left);
    }
    outs[j] = temp_node->outs;
    ins[j] = temp_node->ins;
    node_group[j] = temp_node->group;
    source_pref[j] = temp_node->sourcep;
    target_pref[j] = temp_node->targetp;
    // free memory (node and tree)
    delete temp_node;
    j++;
  }
  // free memory (queue)
  queue<node_d*>().swap(q);
  
  Rcpp::List ret;
  ret["m"] = m;
  ret["nnode"] = new_node_id;
  ret["nedge"] = new_edge_id;
  ret["node_vec1"] = source_node;
  ret["node_vec2"] = target_node;
  ret["outstrength"] = outs;
  ret["instrength"] = ins;
  ret["scenario"] = scenario;
  ret["nodegroup"] = node_group;
  ret["source_pref"] = source_pref;
  ret["target_pref"] = target_pref;
  return ret;
}