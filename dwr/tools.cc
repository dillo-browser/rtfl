/*
 * RTFL
 *
 * Copyright 2013-2015 Sebastian Geerken <sgeerken@dillo.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version; with the following exception:
 *
 * The copyright holders of RTFL give you permission to link this file
 * statically or dynamically against all versions of the graphviz
 * library, which are published by AT&T Corp. under one of the following
 * licenses:
 *
 * - Common Public License version 1.0 as published by International
 *   Business Machines Corporation (IBM), or
 * - Eclipse Public License version 1.0 as published by the Eclipse
 *   Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ----------------------------------------------------------------------
 *
 * A part of the code was written by Keith Vertanen and has been
 * released to the public domain; see below.
 */

#include "tools.hh"

#include <math.h>

using namespace dw::core;
using namespace dw::core::style;

namespace rtfl {

namespace dw {

namespace tools {

// Used for b-splines, see below.   
struct point {
   double x;
   double y;
   double z;
};

static void bspline (int n, int t, point *control, point *output,
                     int num_output);

void drawArrowHead (View *view, Style *style,
                    int x1, int y1, int x2, int y2, int aheadlen)
{
   if (x1 != x2 || y1 != y2) { 
      // TODO: Use faster algorithm avoding floating point numbers. Also,
      // regard that using integers could cause overflow errors.

      int l = sqrt ((double)(x2 - x1) * (double)(x2 - x1) +
                    (double)(y2 - y1) * (double)(y2 - y1));
      int x3 = (aheadlen * x1 + (l - aheadlen) * x2) / l;
      int y3 = (aheadlen * y1 + (l - aheadlen) * y2) / l;
      
      int x4 = x3 - (y2 - y3) / 2;
      int y4 = y3 + (x2 - x3) / 2;
      view->drawLine (style->color, Color::SHADING_NORMAL, x2, y2, x4, y4);
   
      int x5 = x3 + (y2 - y3) / 2;
      int y5 = y3 - (x2 - x3) / 2;
      view->drawLine (style->color, Color::SHADING_NORMAL, x2, y2, x5, y5);
   }
}

void drawBSpline (::dw::core::View *view, ::dw::core::style::Style *style,
                  int degree, int numPoints, int *x, int *y)
{
   point *in = new point[numPoints];
   for (int i = 0; i < numPoints; i++) {
      in[i].x = x[i];
      in[i].y = y[i];
      in[i].z = 0;
   }
                                      
   int numOut = 5 * numPoints;
   point *out = new point[numOut];

   bspline(numPoints - 1, degree, in, out, numOut);

   for (int i = 0; i < numOut - 1; i++)
      view->drawLine (style->color, ::dw::core::style::Color::SHADING_NORMAL,
                      out[i].x, out[i].y,
                      out[i + 1].x, out[i + 1].y);

   delete[] in;
   delete[] out;
}

/* ----------------------------------------------------------------------
      The following code was copied from
      <ftp://ftp.grnet.gr/pub/lang/algorithms/c++/bspline.cpp>.

      It should be modified so that it is better adapted to our needs
      (no z coordinate, no unnecessary conversion between int and
      double, etc.)
   ---------------------------------------------------------------------- */
   
/*********************************************************************

 Simple b-spline curve algorithm

 Copyright 1994 by Keith Vertanen (vertankd@cda.mrs.umn.edu)

 Released to the public domain (your mileage may vary)

**********************************************************************/

static void compute_intervals(int *u, int n, int t);
static double blend(int k, int t, int *u, double v);
static void compute_point(int *u, int n, int t, double v, point *control,
		   point *output);

void bspline(int n, int t, point *control, point *output, int num_output)

/*********************************************************************

Parameters:
  n          - the number of control points minus 1
  t          - the degree of the polynomial plus 1
  control    - control point array made up of point stucture
  output     - array in which the calculate spline points are to be put
  num_output - how many points on the spline are to be calculated

Pre-conditions:
  n+2>t  (no curve results if n+2<=t)
  control array contains the number of points specified by n
  output array is the proper size to hold num_output point structures


**********************************************************************/

{
  int *u;
  double increment,interval;
  point calcxyz;
  int output_index;

  u=new int[n+t+1];
  compute_intervals(u, n, t);

  increment=(double) (n-t+2)/(num_output-1);  // how much parameter goes up each time
  interval=0;

  for (output_index=0; output_index<num_output-1; output_index++)
  {
    compute_point(u, n, t, interval, control, &calcxyz);
    output[output_index].x = calcxyz.x;
    output[output_index].y = calcxyz.y;
    output[output_index].z = calcxyz.z;
    interval=interval+increment;  // increment our parameter
  }
  output[num_output-1].x=control[n].x;   // put in the last point
  output[num_output-1].y=control[n].y;
  output[num_output-1].z=control[n].z;

  delete u;
}

double blend(int k, int t, int *u, double v)  // calculate the blending value
{
  double value;

  if (t==1)			// base case for the recursion
  {
    if ((u[k]<=v) && (v<u[k+1]))
      value=1;
    else
      value=0;
  }
  else
  {
    if ((u[k+t-1]==u[k]) && (u[k+t]==u[k+1]))  // check for divide by zero
      value = 0;
    else
    if (u[k+t-1]==u[k]) // if a term's denominator is zero,use just the other
      value = (u[k+t] - v) / (u[k+t] - u[k+1]) * blend(k+1, t-1, u, v);
    else
    if (u[k+t]==u[k+1])
      value = (v - u[k]) / (u[k+t-1] - u[k]) * blend(k, t-1, u, v);
    else
      value = (v - u[k]) / (u[k+t-1] - u[k]) * blend(k, t-1, u, v) +
	      (u[k+t] - v) / (u[k+t] - u[k+1]) * blend(k+1, t-1, u, v);
  }
  return value;
}

void compute_intervals(int *u, int n, int t)   // figure out the knots
{
  int j;

  for (j=0; j<=n+t; j++)
  {
    if (j<t)
      u[j]=0;
    else
    if ((t<=j) && (j<=n))
      u[j]=j-t+1;
    else
    if (j>n)
      u[j]=n-t+2;  // if n-t=-2 then we're screwed, everything goes to 0
  }
}

void compute_point(int *u, int n, int t, double v, point *control,
			point *output)
{
  int k;
  double temp;

  // initialize the variables that will hold our outputted point
  output->x=0;
  output->y=0;
  output->z=0;

  for (k=0; k<=n; k++)
  {
    temp = blend(k,t,u,v);  // same blend is used for each dimension coordinate
    output->x = output->x + (control[k]).x * temp;
    output->y = output->y + (control[k]).y * temp;
    output->z = output->z + (control[k]).z * temp;
  }
}

} // namespace tools

} // namespace rtfl

} // namespace dw
