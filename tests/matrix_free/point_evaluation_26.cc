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

/*
 * Test if FEPointEval::evaluate() and FEPointEval::test_and_sum() work with
 * FESystems with FE_Q and FE_DGQ.
 *
 * We check FEPointEval with n_components=2 and first_selected_component=0
 * and FEPointEval with n_components=1 and first_selected_component=1.
 */
#include <deal.II/fe/fe_dgq.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>

#include <deal.II/grid/grid_generator.h>

#include <deal.II/numerics/vector_tools.h>

#include "../tests.h"

using namespace dealii;

template <int dim>
class AnalyticalFunction : public Function<dim>
{
public:
  AnalyticalFunction(unsigned int n_components)
    : Function<dim>(n_components)
  {}

  double
  value(const Point<dim> &p, const unsigned int component) const final
  {
    return p[component];
  }
};

template <int first_selected_component = 0, int dim, typename Number = double>
void
test(const FiniteElement<dim> &fe)
{
  constexpr unsigned int n_components = dim;

  // setup tria (which is one reference cell)
  Triangulation<dim> tria;
  GridGenerator::hyper_cube(tria);

  MappingQ<dim> mapping(1);

  DoFHandler<dim> dof_handler(tria);
  dof_handler.distribute_dofs(fe);


  Vector<double> vector(dof_handler.n_dofs());
  VectorTools::interpolate(mapping,
                           dof_handler,
                           AnalyticalFunction<dim>(n_components),
                           vector);

  // define point inside cell
  // we want to obtain the exact same point by evaluating with FEPEval at the
  // point
  auto eval_p = Point<dim>{0.1, 0.2};

  // use FEPointEvaluation to get the point
  FEPointEvaluation<dim - first_selected_component, dim, dim, Number>
    fe_point_eval(mapping, fe, update_values, first_selected_component);

  std::vector<Number> buffer(fe.dofs_per_cell);

  const auto cell = *dof_handler.active_cell_iterators().begin();

  // since tria is unit cell real point equals unit point
  fe_point_eval.reinit(cell, std::vector<Point<dim>>{eval_p});
  cell->get_dof_values(vector, buffer.begin(), buffer.end());
  fe_point_eval.evaluate(buffer, EvaluationFlags::values);


  deallog << "Evaluate Result" << std::endl;
  deallog << fe_point_eval.get_value(0) << std::endl;
  deallog << std::endl;


  deallog << "DoF values after test_and_sum()" << std::endl;
  fe_point_eval.submit_value(fe_point_eval.get_value(0), 0);
  fe_point_eval.test_and_sum(buffer, EvaluationFlags::values);

  for (const auto &b : buffer)
    deallog << b << std::endl;

  deallog << std::endl;
}


int
main(int argc, char **argv)
{
  initlog();

  const unsigned int degree = 2;
  const unsigned int dim    = 2;

  deallog << "FE_Q" << std::endl;
  FESystem<dim> fe_q(FE_Q<dim>(degree), dim);
  deallog << "All components" << std::endl;
  test<0>(fe_q);
  deallog << "Second component" << std::endl;
  test<1>(fe_q);

  deallog << "FE_DGQ" << std::endl;
  FESystem<dim> fe_dgq(FE_DGQ<dim>(degree), dim);
  deallog << "All components" << std::endl;
  test<0>(fe_dgq);
  deallog << "Second component" << std::endl;
  test<1>(fe_dgq);

  return 0;
}
