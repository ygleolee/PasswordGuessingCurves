#include <iostream>

#include "bounds.hpp"
#include "helpers.hpp"

// LP paper

double freq_UB(dist_t& dist, int64_t G, double err) { // Coro 4
  int64_t top_G_freq = most_frequent(dist, G);
  double eps = sqrtl(-log(err) / (2.0 * dist.N));
  return std::min(((double) top_G_freq) / ((double) dist.N) + eps, 1.0);
}

double samp_LB(dist_t& dist, int64_t G, double err) { // Thm 5
  if (dist.D2_idx.empty()) {
    std::cerr << "must partition before calculating sampling LB" << std::endl;
    return -1.0;
  }

  // TODO: There might be more efficient implementation, now its O(distinct), might be some O(log(N)*d)

  std::vector<int64_t> cnt_D1(dist.distinct, 0);
  std::vector<int64_t> cnt_D2(dist.distinct, 0);
  for (int i=0; i<dist.freqcount.size(); ++i) {
    int64_t freq = dist.freqcount[i].first;
    int64_t count = dist.freqcount[i].second;
    int64_t offset = (i==0) ? 0 : dist.prefcount[i-1];
    for (int j=0; j<count; ++j) {
      cnt_D1[offset+j] = freq;
    }
  }

  std::vector<int64_t> pref_D(dist.distinct, 0);
  pref_D[0] = cnt_D1[0];
  for (int i=1; i<pref_D.size(); ++i) {
    pref_D[i] = pref_D[i-1] + cnt_D1[i];
  }

  for (auto x:dist.D2_idx) {
    auto it = std::lower_bound(pref_D.begin(), pref_D.end(), x);
    if (it == pref_D.end()) {
      std::cerr << "Something went wrong here" << std::endl;
      return -1.0;
    }
    int64_t id = it - pref_D.begin();
    cnt_D2[id]++;
    cnt_D1[id]--;
  }

  std::vector<std::pair<int64_t, int64_t>> cnt_D1_D2(dist.distinct);
  for (int i=0; i<dist.distinct; ++i) {
    cnt_D1_D2[i] = std::make_pair(cnt_D1[i], cnt_D2[i]);
  }

  sort(cnt_D1_D2.begin(), cnt_D1_D2.end(), [&](std::pair<int64_t, int64_t>& a, std::pair<int64_t, int64_t>& b) {
    if (a.first == b.first) {
      return a.second > b.second;
    }
    return a.first > b.first;
  });

  double h_D1_D2_G = 0.0;
  for (int i=0; i<G; ++i) {
    h_D1_D2_G += cnt_D1_D2[i].second;
  }

  double t = sqrtl(-log(err) / (2.0 * dist.D2_idx.size()));
  return (h_D1_D2_G - t) / dist.D2_idx.size();
}

double prior_LB(dist_t& dist, int64_t G, int64_t j, double err1, double err2) { // Thm 9
  if (G <= dist.N) {
    std::cerr << "No L value satisfy the constraints on the parameters" << std::endl;
    return -1.0;
  }

  if (j < 2) {
    std::cerr << "Invalid j value. j must be greater than or equal to 2." << std::endl;
    return -1.0;
  }

  double L = ((double) G) / ((double) dist.N);
  double temp = (double) dist.N;
  for (int i=0; i<j-1; ++i) {
    temp /= ((j-1) * L);
  }

  int lo = 0, hi = dist.freqcount.size() - 1, mid;
  while (lo != hi) {
    mid = (lo + hi + 1) >> 1;
    if (dist.freqcount[mid].first >= j) {
      lo = mid;
    }
    else {
      hi = mid - 1;
    }
  }

  double f_SL = ((double) dist.preftotal[lo]) / ((double) dist.N) - temp;
  double t = sqrtl(-dist.N * j * j * log(err1) / 2);
  double eps = sqrtl(-log(err2) / (2.0 * dist.N));

  return std::max(f_SL - t/dist.N - eps, 0.0);
}

double extended_LB() { // Coro 7
}

// double best_prior_LB(dist_t& dist, int64_t G, double err1, double err2) { // Thm 9 with different j, return best
// }


// PIN paper

double new_LB_model() { // Thm 3
}

double new_LB_samp() { // Coro 4
}

double new_UB(dist_t& dist, int64_t G, double err) { // Thm 2
  int64_t F = most_frequent(dist, G);

  if (F == dist.N) {
    return 1.0;
  }

  int64_t iterations = 50;
  double lo=0.0, hi=1.0, mid;

  while (iterations--) {
    mid = (lo + hi) / 2.0;
    double cdf = bcdf(F, dist.N, mid);
    if (cdf > err) {
      lo = mid;
    }
    else {
      hi = mid;
    }
  }

  return mid;
}
