// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_Pk2DLOCALFINITEELEMENT_HH
#define DUNE_Pk2DLOCALFINITEELEMENT_HH

#include <dune/common/geometrytype.hh>

#include "common/localfiniteelement.hh"
#include "pk2d/pk2dlocalbasis.hh"
#include "pk2d/pk2dlocalcoefficients.hh"
#include "pk2d/pk2dlocalinterpolation.hh"

namespace Dune
{

  /** \todo Please doc me !
   */
  template<class D, class R, unsigned int k>
  class Pk2DLocalFiniteElement
    :
#if DUNE_VIRTUAL_SHAPEFUNCTIONS
      public
#endif
      LocalFiniteElementInterface<
          LocalFiniteElementTraits<Pk2DLocalBasis<D,R,k>,
              Pk2DLocalCoefficients<k>,
              Pk2DLocalInterpolation<Pk2DLocalBasis<D,R,k> > >
#ifndef DUNE_VIRTUAL_SHAPEFUNCTIONS
          ,Pk2DLocalFiniteElement<D,R,k>
#endif
          >
  {
  public:
    /** \todo Please doc me !
     */
    typedef LocalFiniteElementTraits<Pk2DLocalBasis<D,R,k>,
        Pk2DLocalCoefficients<k>,
        Pk2DLocalInterpolation<Pk2DLocalBasis<D,R,k> > > Traits;

    /** \todo Please doc me !
     */
    Pk2DLocalFiniteElement ()
    {
      gt.makeTriangle();
    }

    /** \todo Please doc me !
     */
    Pk2DLocalFiniteElement (int variant) : coefficients(variant)
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
    Pk2DLocalBasis<D,R,k> basis;
    Pk2DLocalCoefficients<k> coefficients;
    Pk2DLocalInterpolation<Pk2DLocalBasis<D,R,k> > interpolation;
    GeometryType gt;
  };

}

#endif
