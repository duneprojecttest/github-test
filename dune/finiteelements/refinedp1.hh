// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_REFINED_P1_LOCALFINITEELEMENT_HH
#define DUNE_REFINED_P1_LOCALFINITEELEMENT_HH

#include <dune/common/geometrytype.hh>

#include "common/localfiniteelement.hh"
#include "refinedp1/refinedp1localbasis.hh"
#include "pk2d/pk2dlocalcoefficients.hh"
#include "pk2d/pk2dlocalinterpolation.hh"
#include "pk2d/pk2dlocalbasis.hh"

namespace Dune
{

  /** \todo Please doc me !
   */
  template<class D, class R>
  class RefinedP1LocalFiniteElement : LocalFiniteElementInterface<
                                          LocalFiniteElementTraits<RefinedP1LocalBasis<D,R>,
                                              Pk2DLocalCoefficients<2>,
                                              Pk2DLocalInterpolation<Pk2DLocalBasis<D,R,2> > >
#ifndef DUNE_VIRTUAL_SHAPEFUNCTIONS
                                          , RefinedP1LocalFiniteElement<D,R>
#endif
                                          >
  {
  public:
    /** \todo Please doc me !
     */
    typedef LocalFiniteElementTraits<RefinedP1LocalBasis<D,R>,
        Pk2DLocalCoefficients<2>,
        Pk2DLocalInterpolation<Pk2DLocalBasis<D,R,2> > > Traits;

    /** \todo Please doc me !
     */
    RefinedP1LocalFiniteElement ()
    {
      gt.makeTriangle();
    }

    /** \todo Please doc me !
     */
    const typename Traits::LocalBasisType& localBasis () const
    {
      return basis;
    }

    /** \todo Please doc me !
     */
    const typename Traits::LocalCoefficientsType& localCoefficients () const
    {
      return coefficients;
    }

    /** \todo Please doc me !
     */
    const typename Traits::LocalInterpolationType& localInterpolation () const
    {
      return interpolation;
    }

    /** \todo Please doc me !
     */
    GeometryType type () const
    {
      return gt;
    }

  private:
    RefinedP1LocalBasis<D,R> basis;
    Pk2DLocalCoefficients<2> coefficients;
    Pk2DLocalInterpolation<Pk2DLocalBasis<D,R,2> > interpolation;
    GeometryType gt;
  };

}

#endif
