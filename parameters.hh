/*
 * This file is part of the Electron Orbital Explorer. The Electron
 * Orbital Explorer is distributed under the Simplified BSD License
 * (also called the "BSD 2-Clause License"), in hopes that these
 * rendering techniques might be used by other programmers in
 * applications such as scientific visualization, video gaming, and so
 * on. If you find value in this software and use its technologies for
 * another purpose, I would love to hear back from you at bjthinks (at)
 * gmail (dot) com. If you improve this software and agree to release
 * your modifications under the below license, I encourage you to fork
 * the development tree on github and push your modifications. The
 * Electron Orbital Explorer's development URL is:
 * https://github.com/bjthinks/orbital-explorer
 * (This paragraph is not part of the software license and may be
 * removed.)
 *
 * Copyright (c) 2013, Brian W. Johnson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * + Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * + Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PARAMETERS_HH
#define PARAMETERS_HH

#include "config.hh"
#include "util.hh"

class Parameters;

template <typename T>
class ParameterReader
{
public:
  ParameterReader(Parameters &p, T (Parameters::*g)() const)
    : params(p), getter(g)
  {}
  operator T() const
  {
    return (params.*getter)();
  }

protected:
  Parameters &params;

private:
  T (Parameters::*getter)() const;
};

template <typename T>
class ParameterController : public ParameterReader<T>
{
public:
  ParameterController(Parameters &p,
                      T (Parameters::*g)() const,
                      void (Parameters::*s)(T))
    : ParameterReader<T>(p, g), setter(s)
  {}
  void operator=(T x) { (params.*setter)(x); }

protected:
  using ParameterReader<T>::params;

private:
  void (Parameters::*setter)(T);
};

template <typename T>
class RangedParameterController : public ParameterController<T>
{
public:
  RangedParameterController(Parameters &p,
                            T (Parameters::*g)() const,
                            void (Parameters::*s)(T),
                            T (Parameters::*minv)() const,
                            T (Parameters::*maxv)() const)
    : ParameterController<T>(p, g, s),
      minval(minv), maxval(maxv)
  {}
  using ParameterController<T>::operator=;
  T min() const { return (params.*minval)(); }
  T max() const { return (params.*maxval)(); }

protected:
  using ParameterController<T>::params;

private:
  T (Parameters::*minval)() const;
  T (Parameters::*maxval)() const;
};

class Parameters
{
public:
  Parameters() : Z(1), N(1), L(0), M(0) {}
  RangedParameterController<int> ZController()
  {
    return RangedParameterController<int>(*this,
                                          &Parameters::getZ, &Parameters::setZ,
                                          &Parameters::minZ, &Parameters::maxZ);
  }
  RangedParameterController<int> NController()
  {
    return RangedParameterController<int>(*this,
                                          &Parameters::getN, &Parameters::setN,
                                          &Parameters::minN, &Parameters::maxN);
  }
  RangedParameterController<int> LController()
  {
    return RangedParameterController<int>(*this,
                                          &Parameters::getL, &Parameters::setL,
                                          &Parameters::minL, &Parameters::maxL);
  }
  RangedParameterController<int> MController()
  {
    return RangedParameterController<int>(*this,
                                          &Parameters::getM, &Parameters::setM,
                                          &Parameters::minM, &Parameters::maxM);
  }
  ParameterReader<double> EnergyReader()
  {
    return ParameterReader<double>(*this, &Parameters::getEnergy);
  }

private:
  int Z;
  int getZ() const { return Z; }
  int minZ() const { return 1; }
  int maxZ() const { return MAX_ATOMIC_NUMBER; }
  void setZ(int newZ)
  {
    clamp(newZ, minZ(), maxZ());
    Z = newZ;
    setEnergy();
  }

  int N;
  int getN() const { return N; }
  int minN() const { return 1; }
  int maxN() const { return MAX_ENERGY_LEVEL; }
  void setN(int newN)
  {
    clamp(newN, minN(), maxN());
    N = newN;
    setL(L);
    setEnergy();
  }

  int L;
  int getL() const { return L; }
  int minL() const { return 0; }
  int maxL() const { return N - 1; }
  void setL(int newL)
  {
    clamp(newL, minL(), maxL());
    L = newL;
    setM(M);
  }

  int M;
  int getM() const { return M; }
  int minM() const { return -L; }
  int maxM() const { return L; }
  void setM(int newM)
  {
    clamp(newM, minM(), maxM());
    M = newM;
  }

  double energy;
  double getEnergy() const { return energy; }
  void setEnergy()
  {
    energy = 13.60569253 * double(Z * Z) / double(N * N);
  }
};

#endif
