#include <RcppArmadillo.h>
// [[Rcpp::depends(RcppArmadillo)]]
//' Degree preserving rewiring process for directed networks.
//'
//' @param iteration Integer, number of iterations for nattemtps rewiring attempts.
//' @param nattempts Integer, number of rewiring attempts per iteration.
//' @param targetNode Vector, target node sequence - 1.
//' @param sourceOut Vector, sequence of source nodes' out-degree.
//' @param sourceIn Vector, sequence of ource nodes' in-degree.
//' @param targetOut Vector, sequence of target nodes' out-degree.
//' @param targetIn Vector, sequence of target nodes' in-degree.
//' @param r_sourceOut Vector, sequence of rank of source nodes' out-degree.
//' @param r_sourceIn Vector, sequence of rank of source nodes' in-degree.
//' @param r_targetOut Vector, sequence of rank of target nodes' out-degree.
//' @param r_targetIn Vector, sequence of rank of target nodes' in-degree.
//' @param index_s Index sequence of source nodes' out- and in-degree. 
//'   index_s/index_t bridges the indices of source/target nodes and the 
//'   target structure eta.
//' @param index_t Index sequence of target nodes' out- and in-degree. 
//' @param eta Matrix, target structure eta generated by
//'   wdnet::directed_joint_dist().
//' @param rewireTrack Logical, whether the rewiring history should be returned.
//' @return Target node sequence, four directed assortativity coefficients after
//'   each iteration, result of rewiring attempts, edge scenarios of accepted rewiring 
//'   attempts, and percentage of accepted rewiring attempts.
//'
// [[Rcpp::export]]
Rcpp::List directed_rewire_cpp(
    int iteration, 
    int nattempts,
    arma::uvec targetNode,
    arma::vec sourceOut,
    arma::vec sourceIn, 
    arma::vec targetOut, 
    arma::vec targetIn,
    arma::vec r_sourceOut,
    arma::vec r_sourceIn,
    arma::vec r_targetOut,
    arma::vec r_targetIn,
    arma::uvec index_s,
    arma::uvec index_t,
    arma::mat eta, 
    bool rewireTrack) {
  GetRNGstate();
  arma::vec out_out(iteration, arma::fill::zeros);
  arma::vec out_in(iteration, arma::fill::zeros);
  arma::vec in_out(iteration, arma::fill::zeros);
  arma::vec in_in(iteration, arma::fill::zeros);
  arma::vec r_out_out(iteration, arma::fill::zeros);
  arma::vec r_out_in(iteration, arma::fill::zeros);
  arma::vec r_in_out(iteration, arma::fill::zeros);
  arma::vec r_in_in(iteration, arma::fill::zeros);
  // arma::mat rewireHistory(iteration * nattempts, 4, arma::fill::zeros);
  // arma::vec sourceOut = outd.elem(sourceNode);
  // arma::vec sourceIn = ind.elem(sourceNode);
  // arma::vec targetOut = outd.elem(targetNode);
  // arma::vec targetIn = ind.elem(targetNode);
  int nedge = targetNode.size();
  int e1, e2, count = 0;
  int s1, s2, t1, t2, hist_row;
  double u, ratio, temp;
  if (rewireTrack) {
    hist_row = iteration * nattempts;
  } else {
    hist_row = 1;
  }
  arma::mat rewireHistory(hist_row, 4, arma::fill::zeros);
  for (int n = 0; n < iteration; n++) {
    for (int i = 0; i < nattempts; i++) {
      e1 = floor(unif_rand() * nedge);
      e2 = floor(unif_rand() * nedge);
      while (e1 == e2) {
        e2 = floor(unif_rand() * nedge);
      }
      if (rewireTrack) {
        rewireHistory(count, 0) = count;
        rewireHistory(count, 1) = e1;
        rewireHistory(count, 2) = e2;
      }
      s1 = index_s[e1];
      s2 = index_s[e2];
      t1 = index_t[e1];
      t2 = index_t[e2];
      if ((eta(s1, t2) * eta(s2, t1)) < (eta(s1, t1) * eta(s2, t2))) {
        ratio = eta(s1, t2) * eta(s2, t1) / 
          (eta(s1, t1) * eta(s2, t2));
      }
      else {
        ratio = 1;
      }
      u = unif_rand();
      if (u <= ratio) {
        temp = index_t[e1];
        index_t[e1] = index_t[e2];
        index_t[e2] = temp;
        temp = targetNode[e1];
        targetNode[e1] = targetNode[e2];
        targetNode[e2] = temp;
        temp = targetOut[e1];
        targetOut[e1] = targetOut[e2];
        targetOut[e2] = temp;
        temp = targetIn[e1];
        targetIn[e1] = targetIn[e2];
        targetIn[e2] = temp;
        temp = r_targetOut[e1];
        r_targetOut[e1] = r_targetOut[e2];
        r_targetOut[e2] = temp;
        temp = r_targetIn[e1];
        r_targetIn[e1] = r_targetIn[e2];
        r_targetIn[e2] = temp;
        if (rewireTrack) {
          rewireHistory(count, 3) = 1;
        }
      }
      count++;
    }
    out_out[n] = (arma::cor(sourceOut, targetOut)).eval()(0, 0);
    out_in[n] = (arma::cor(sourceOut, targetIn)).eval()(0, 0);
    in_out[n] = (arma::cor(sourceIn, targetOut)).eval()(0, 0);
    in_in[n] = (arma::cor(sourceIn, targetIn)).eval()(0, 0);
    r_out_out[n] = (arma::cor(r_sourceOut, r_targetOut)).eval()(0, 0);
    r_out_in[n] = (arma::cor(r_sourceOut, r_targetIn)).eval()(0, 0);
    r_in_out[n] = (arma::cor(r_sourceIn, r_targetOut)).eval()(0, 0);
    r_in_in[n] = (arma::cor(r_sourceIn, r_targetIn)).eval()(0, 0);
  }
  
  PutRNGstate();
  Rcpp::List ret;
  ret["targetNode"] = targetNode;
  if (rewireTrack) {
    ret["rewireHistory"] = rewireHistory;
  }
  ret["out_out"] = out_out;
  ret["out_in"] = out_in;
  ret["in_out"] = in_out;
  ret["in_in"] = in_in;
  ret["r_out_out"] = r_out_out;
  ret["r_out_in"] = r_out_in;
  ret["r_in_out"] = r_in_out;
  ret["r_in_in"] = r_in_in;
  return ret;
}

//' Degree preserving rewiring process for undirected networks.
//' 
//' @param iteration Integer, number of iterations for nattemtps rewiring attempts.
//' @param nattempts Integer, number of rewiring attempts for each iteration.
//' @param node1 Vector, node sequence of the first column of edgelist.
//' @param node2 Vector, node sequence of the second column of edgelist.
//' @param degree1 Vector, degree sequence of node1 and node2.
//' @param degree2 Vector, degree sequence of node2 and node1. degree1 
//'   and degree2 are used to calculate assortativity coefficient,
//'   i.e., degree correlation.
//' @param index1 Index sequence of the first column of edgelist. 
//'   index1 and index2 bridge the nodes' degree and the 
//'   structure e.
//' @param index2 Index sequence of the second column of edgelist..
//' @param e Matrix, target structure e generated by
//'   wdnet::undirected_joint_dist().
//' @param rewireTrack Logical, whether the rewiring history should be returned.
//' @return Node sequences, node indexes and rewiring history.
//'
// [[Rcpp::export]]
Rcpp::List undirected_rewire_cpp(
    int iteration,
    int nattempts,
    Rcpp::IntegerVector node1, 
    Rcpp::IntegerVector node2, 
    arma::vec degree1,
    arma::vec degree2,
    arma::vec index1,
    arma::vec index2,
    arma::mat e, 
    bool rewireTrack) {
  GetRNGstate();
  arma::vec rho(iteration, arma::fill::zeros);
  int nedge = index1.size();
  int e1, e2, temp, count = 0;
  int s1, s2, t1, t2, hist_row;
  double u, v, ratio;
  if (rewireTrack) {
    hist_row = iteration * nattempts;
  } else {
    hist_row = 1;
  }
  arma::mat rewireHistory(hist_row, 5, arma::fill::zeros);
  
  for (int n = 0; n < iteration; n++) {
    for (int i = 0; i < nattempts; i++) {
      e1 = floor(unif_rand() * nedge);
      e2 = floor(unif_rand() * nedge);
      while (e1 == e2) {
        e2 = floor(unif_rand() * nedge);
      }
      if (rewireTrack) {
        rewireHistory(count, 0) = count;
        rewireHistory(count, 1) = e1;
        rewireHistory(count, 2) = e2;
      }
      s1 = index1[e1];
      s2 = index1[e2];
      t1 = index2[e1];
      t2 = index2[e2];
      v = unif_rand();
      u = unif_rand();
      if (v < 0.5) {
        // if (rewireTrack) {
        //   rewireHistory(count, 3) = 0;
        // }
        if ((e(s1, t2) * e(s2, t1)) < (e(s1, t1) * e(s2, t2))) {
          ratio = e(s1, t2) * e(s2, t1) / 
            (e(s1, t1) * e(s2, t2));
        }
        else {
          ratio = 1;
        }
        if (u <= ratio) {
          if (rewireTrack) {
            rewireHistory(count, 4) = 1;
          }
          temp = index2[e1];
          index2[e1] = index2[e2];
          index2[e2] = temp;
          temp = node2[e1];
          node2[e1] = node2[e2];
          node2[e2] = temp;
          temp = degree2[e1];
          degree2[e1] = degree2[e2];
          degree2[e2] = temp;
          temp = degree1[e1 + nedge];
          degree1[e1 + nedge] = degree1[e2 + nedge];
          degree1[e2 + nedge] = temp;
        }
      } else {
        if (rewireTrack) {
          rewireHistory(count, 3) = 1;
        }
        if ((e(s1, s2) * e(t1, t2)) < (e(s1, t1) * e(s2, t2))) {
          ratio = e(s1, s2) * e(t1, t2) / 
            (e(s1, t1) * e(s2, t2));
        }
        else {
          ratio = 1;
        }
        if (u <= ratio) {
          if (rewireTrack) {
            rewireHistory(count, 4) = 1;
          }
          temp = index2[e1];
          index2[e1] = index1[e2];
          index1[e2] = temp;
          temp = node2[e1];
          node2[e1] = node1[e2];
          node1[e2] = temp;
          temp = degree2[e1];
          degree2[e1] = degree1[e2];
          degree1[e2] = temp;
          temp = degree1[e1 + nedge];
          degree1[e1 + nedge] = degree2[e2 + nedge];
          degree2[e2 + nedge] = temp;
        }
      }
      count++;
    }
    rho[n] = (arma::cor(degree1, degree2)).eval()(0, 0);
  }
  PutRNGstate();
  Rcpp::List ret;
  if (rewireTrack) {
    ret["rewireHistory"] = rewireHistory;
  }
  ret["node1"] = node1;
  ret["node2"] = node2;
  ret["rho"] = rho;
  // ret["degree1"] = degree1;
  // ret["degree2"] = degree2;
  return ret;
}