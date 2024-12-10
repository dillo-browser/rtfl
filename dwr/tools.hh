#ifndef __DWR_TOOLS_HH__
#define __DWR_TOOLS_HH__

#include "dw/core.hh"

namespace rtfl {

namespace dw {

namespace tools {

void drawArrowHead (::dw::core::View *view, ::dw::core::style::Style *style,
                    int x1, int y1, int x2, int y2, int aheadlen);

void drawBSpline (::dw::core::View *view, ::dw::core::style::Style *style,
                  int degree, int numPoints, int *x, int *y);

} // namespace tools

} // namespace rtfl

} // namespace dw

#endif // __DWR_TOOLS_HH__
