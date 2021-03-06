// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_P2_3DLOCALFINITEELEMENT_HH
#define DUNE_P2_3DLOCALFINITEELEMENT_HH

#include <dune/common/geometrytype.hh>

#include "common/localfiniteelement.hh"
#include "p23d/p23dlocalbasis.hh"
#include "p23d/p23dlocalcoefficients.hh"
#include "p23d/p23dlocalinterpolation.hh"

namespace Dune
{

  /** \todo Please doc me !
   */
  template<class D, class R>
  class P23DLocalFiniteElement : LocalFiniteElementInterface<
                                     LocalFiniteElementTraits<P23DLocalBasis<D,R>,P23DLocalCoefficients,
                                         P23DLocalInterpolation<P23DLocalBasis<D,R> > >
#ifndef DUNE_VIRTUAL_SHAPEFUNCTIONS
                                     , P23DLocalFiniteElement<D,R>
#endif
                                     >
  {
  public:
    /** \todo Please doc me !
     */
    typedef LocalFiniteElementTraits<P23DLocalBasis<D,R>,
        P23DLocalCoefficients,
        P23DLocalInterpolation<P23DLocalBasis<D,R> > > Traits;

    /** \todo Please doc me !
     */
    P23DLocalFiniteElement ()
    {
      gt.makeTetrahedron();
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
    P23DLocalBasis<D,R> basis;
    P23DLocalCoefficients coefficients;
    P23DLocalInterpolation<P23DLocalBasis<D,R> > interpolation;
    GeometryType gt;
  };

}

#endif
