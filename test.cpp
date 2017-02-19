#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "FunctionalUtilities.h"
#include "BTree.h"
#include "BlackScholes.h"
/*Black Scholes drift and Put payoffs */
double alpha(double t, double x, double sig, double alph){
  return alph/sig; //alpha/sigma
}
double sigma(double t, double x, double sig){
  return sig; //sigma'(x)
}
double payoff(double x, double k){
  if(k>x){
    return k-x;
  }
  else{
    return 0;
  }
}
double discount(double t, double x, double dt, double r){
  return exp(-r*dt);
}
double finv(double x, double sig){
  return exp(sig*x);
}
/*End Black Scholes */

/*Begin interest rate "option" (bond pricing) assuming CIR underlying*/
double alphaR(double t, double x, double a, double b, double sig){
  return (a*(b-x))/(sig*sqrt(x));
}
double sigmaR(double t, double x, double sig){
  return (.5*sig)/sqrt(x);
}
double payoffR(double x){
  return 1;
}
double discountR(double t, double x, double dt){
  return exp(-x*dt);
}
double finvR(double x, double sig){
  return sig*sig*x*x*.25;
}

TEST_CASE("Test BlackScholes", "[BTree]"){
    const double r=.03;
    const double sig=.3;
    const double S0=50;
    const double T=1;
    const double k=50;
    REQUIRE(btree::computePrice(
       [&](double t, double underlying, double dt, int width ){return alpha(t, underlying, sig, r);},
       [&](double t, double underlying, double dt, int width){return sigma(t, underlying, sig);}, 
       [&](double t, double x, double dt, int width){return finv(x, sig);}, 
       [&](double t, double underlying, double dt, int width){return payoff(underlying, k);}, 
       [&](double t, double underlying, double dt, int width){return discount(t, underlying, dt, r);},
       log(S0)/sig,
       5000,
       T,
       false
    )==Approx(
        BSPut(S0, exp(-r*T), k, sig*sqrt(T))
    ).epsilon(.0001));
}
TEST_CASE("Test CIR", "[BTree]"){
    const double r=.03;
    const double sig=.3;
    const double initialR=2.0*sqrt(r)/sig;
    const double T=1.0;
    const double a=1;
    const double b=.05;
    auto bondcir=[](double r, double a, double b, double sigma, double tau){
        double h=sqrt(a*a+2*sigma*sigma);
        double expt=exp(tau*h)-1;
        double den=2*h+(a+h)*expt;
        double AtT=(2*a*b)/(sigma*sigma);
        AtT=AtT*log(2*h*exp((a+h)*tau*.5)/den);
        double BtT=2*expt/den;
        return exp(AtT-r*BtT);
    };
    REQUIRE(btree::computePrice(
       [&](double t, double underlying, double dt, int width ){return alphaR(t, underlying, a, b, sig);},
       [&](double t, double underlying, double dt, int width){return sigmaR(t, underlying, sig);}, 
       [&](double t, double x, double dt, int width){return finvR(x, sig);}, 
       [&](double t, double underlying, double dt, int width){return payoffR(underlying);}, 
       [&](double t, double underlying, double dt, int width){return discountR(t, underlying, dt);},
       initialR,
       5000,
       T,
       false
    )==Approx(
        bondcir(r, a, b,sig, T)
    ).epsilon(.0001));
}

