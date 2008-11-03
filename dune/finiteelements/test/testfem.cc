// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <iostream>

#include "../p0.hh"
#include "../p12d.hh"
#include "../pk2d.hh"

int main(int argc, char** argv)
{
  Dune::P0LocalFiniteElement<float,float,2> p0lfem(Dune::GeometryType::simplex);
  std::cout << p0lfem.type() << std::endl;

  int s=p0lfem.localCoefficients().size();

  Dune::P12DLocalFiniteElement<double,double> p12dlfem;
  Dune::Pk2DLocalFiniteElement<double,double,5> pk2dlfem;

  return 0;
}