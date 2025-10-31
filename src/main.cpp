#include "bond.hpp"
#include "bond_pricer.hpp"
#include "bootstrap.hpp"
#include "curve.hpp"
#include <Eigen/Dense>
#include <cstddef>
#include <fmt/core.h>
#include <iomanip>
#include <iostream>
#include <range/v3/all.hpp>
#include <spdlog/spdlog.h>
#include <vector>
using namespace pricer;
int main() {
  std::vector<Bond> bonds = {{100.0, 0.02, 1.0, 2},
                             {100.0, 0.03, 2.0, 2},
                             {100.0, 0.04, 5.0, 2},
                             {100.0, 0.05, 10.0, 2}};
  std::vector<double> prices_mkt = {97.777, 96.89, 96.267, 96.075};
  auto br = boostrap_piecewise_forward(bonds, prices_mkt, 0.0, 0.2, 1e-10, 200);
  Curve c = make_curve_from_bootstrap(br);
  for (auto &p : br.pillars) {
    double y_cont = -std::log(p.df) / p.t;
    std::cout << "Tenor " << std::fixed << std::setprecision(1) << std::setw(1)
              << std::setw(4) << p.t << "Y |";
    std::cout << std::fixed << std ::setprecision(2) << std::setw(6)
              << y_cont * 100 << "%\n";
  }
  std::cout.unsetf(std::ios::fixed | std::ios::scientific);
  std::cout.precision(6);

  BondPricer pricer(c);
  for (std::size_t i = 0; i < bonds.size(); ++i) {
    double P_model = pricer.price(bonds[i]);
    double P_mkt = prices_mkt[i];
    double diff = std::abs(P_model - P_mkt);
    std::cout << "Bond" << i << " Maturity: " << bonds[i].maturity()
              << " P model: " << P_model << " P market :" << P_mkt
              << " diff : " << diff << "\n";
  }
  return 0;
}
