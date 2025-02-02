// ---------------------------------------------------------------------
//
// Copyright (C) 2023 by the deal.II authors
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE.md at
// the top level directory of deal.II.
//
// ---------------------------------------------------------------------

#include <deal.II/base/std_cxx20/functional.h>

#include "../tests.h"


using namespace dealii;

namespace
{
  //! Helper type which is move-only.
  struct MoveOnly
  {
    explicit MoveOnly(int i)
      : i{i}
    {}

    MoveOnly(const MoveOnly &)     = delete;
    MoveOnly(MoveOnly &&) noexcept = default;

    MoveOnly &
    operator=(const MoveOnly &) = delete;
    MoveOnly &
    operator=(MoveOnly &&) = default;

    ~MoveOnly() = default;

    int i;
  };


  // use an interesting signature with move-only types to test that perfect
  // forwarding works correctly
  void
  evaluate(const std::function<int(double, const MoveOnly &, MoveOnly &&)> &fn)
  {
    double   d{0.0};
    MoveOnly m1{1};
    MoveOnly m2{2};
    fn(d, m1, std::move(m2));
  }

  //! Test class which creates callable member functions and passes them to
  //! evaluate()
  class Test
  {
  public:
    void
    run()
    {
      auto f = std_cxx20::bind_front(&Test::member, this);
      f(0.0, MoveOnly{1}, MoveOnly{2});
      evaluate(std_cxx20::bind_front(&Test::member, this));
      evaluate(std_cxx20::bind_front(&Test::const_member, this));
    }

    void
    run_const() const
    {
      evaluate(std_cxx20::bind_front(&Test::const_member, this));
    }

  private:
    int
    member(double d, const MoveOnly &ref, const MoveOnly &mov)
    {
      deallog << d << " " << ref.i << " " << mov.i << std::endl;
      return 0;
    }

    int
    const_member(double d, const MoveOnly &ref, MoveOnly &&mov) const
    {
      deallog << d << " " << ref.i << " " << mov.i << std::endl;
      return 0;
    }
  };


  void
  f(int a, double b)
  {
    deallog << a << " "
            << " " << b << std::endl;
  }

  void
  f2(int a, MoveOnly &&b)
  {
    deallog << a << " "
            << " " << b.i << std::endl;
  }

} // namespace

int
main()
{
  initlog();

  Test test{};
  test.run();
  test.run_const();

  std_cxx20::bind_front(f)(1, 2.0);
  std_cxx20::bind_front(f2, 1)(MoveOnly{2});

  return 0;
}
