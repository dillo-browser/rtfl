/*
 * RTFL (originally part of dillo)
 *
 * Copyright 2005-2007 Sebastian Geerken <sgeerken@dillo.org>
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
 */



#include "core.hh"

#include "../lout/msg.h"
#include "../lout/debug.hh"
#include "../lout/misc.hh"

using namespace lout;
using namespace lout::container;
using namespace lout::object;

namespace dw {
namespace core {

bool Layout::LayoutImgRenderer::readyToDraw ()
{
   return true;
}

void Layout::LayoutImgRenderer::getBgArea (int *x, int *y, int *width,
                                           int *height)
{
   // TODO Actually not padding area, but visible area?
   getRefArea (x, y, width, height);
}

void Layout::LayoutImgRenderer::getRefArea (int *xRef, int *yRef, int *widthRef,
                                            int *heightRef)
{
   *xRef = 0;
   *yRef = 0;
   *widthRef = misc::max (layout->viewportWidth
                          - (layout->canvasHeightGreater ?
                             layout->vScrollbarThickness : 0),
                          layout->canvasWidth);
   *heightRef = misc::max (layout->viewportHeight
                           - layout->hScrollbarThickness,
                           layout->canvasAscent + layout->canvasDescent);
}

style::StyleImage *Layout::LayoutImgRenderer::getBackgroundImage ()
{
   return layout->bgImage;
}

style::BackgroundRepeat Layout::LayoutImgRenderer::getBackgroundRepeat ()
{
   return layout->bgRepeat;
}

style::BackgroundAttachment
   Layout::LayoutImgRenderer::getBackgroundAttachment ()
{
   return layout->bgAttachment;
}

style::Length Layout::LayoutImgRenderer::getBackgroundPositionX ()
{
   return layout->bgPositionX;
}

style::Length Layout::LayoutImgRenderer::getBackgroundPositionY ()
{
   return layout->bgPositionY;
}

void Layout::LayoutImgRenderer::draw (int x, int y, int width, int height)
{
   layout->queueDraw (x, y, width, height);
}

// ----------------------------------------------------------------------

void Layout::Receiver::resizeQueued (bool extremesChanged)
{
}

void Layout::Receiver::canvasSizeChanged (int width, int ascent, int descent)
{
}

// ----------------------------------------------------------------------

bool Layout::Emitter::emitToReceiver (lout::signal::Receiver *receiver,
                                      int signalNo, int argc,
                                      lout::object::Object **argv)
{
   Receiver *layoutReceiver = (Receiver*)receiver;

   switch (signalNo) {
   case CANVAS_SIZE_CHANGED:
      layoutReceiver->canvasSizeChanged (((Integer*)argv[0])->getValue (),
                                         ((Integer*)argv[1])->getValue (),
                                         ((Integer*)argv[2])->getValue ());
      break;

   case RESIZE_QUEUED:
      layoutReceiver->resizeQueued (((Boolean*)argv[0])->getValue ());
      break;

   default:
      misc::assertNotReached ();
   }

   return false;
}

void Layout::Emitter::emitResizeQueued (bool extremesChanged)
{
   Boolean ec (extremesChanged);
   Object *argv[1] = { &ec };
   emitVoid (RESIZE_QUEUED, 1, argv);
}

void Layout::Emitter::emitCanvasSizeChanged (int width,
                                             int ascent, int descent)
{
   Integer w (width), a (ascent), d (descent);
   Object *argv[3] = { &w, &a, &d };
   emitVoid (CANVAS_SIZE_CHANGED, 3, argv);
}

// ----------------------------------------------------------------------

bool Layout::LinkReceiver::enter (Widget *widget, int link, int img,
                                  int x, int y)
{
   return false;
}

bool Layout::LinkReceiver::press (Widget *widget, int link, int img,
                                  int x, int y, EventButton *event)
{
   return false;
}

bool Layout::LinkReceiver::release (Widget *widget, int link, int img,
                                    int x, int y, EventButton *event)
{
   return false;
}

bool Layout::LinkReceiver::click (Widget *widget, int link, int img,
                                    int x, int y, EventButton *event)
{
   return false;
}

// ----------------------------------------------------------------------

bool Layout::LinkEmitter::emitToReceiver (lout::signal::Receiver *receiver,
                                          int signalNo, int argc,
                                          lout::object::Object **argv)
{
   LinkReceiver *linkReceiver = (LinkReceiver*)receiver;

   switch (signalNo) {
   case ENTER:
      return linkReceiver->enter ((Widget*)argv[0],
                                  ((Integer*)argv[1])->getValue (),
                                  ((Integer*)argv[2])->getValue (),
                                  ((Integer*)argv[3])->getValue (),
                                  ((Integer*)argv[4])->getValue ());

   case PRESS:
      return linkReceiver->press ((Widget*)argv[0],
                                  ((Integer*)argv[1])->getValue (),
                                  ((Integer*)argv[2])->getValue (),
                                  ((Integer*)argv[3])->getValue (),
                                  ((Integer*)argv[4])->getValue (),
                                  (EventButton*)argv[5]);

   case RELEASE:
      return linkReceiver->release ((Widget*)argv[0],
                                    ((Integer*)argv[1])->getValue (),
                                    ((Integer*)argv[2])->getValue (),
                                    ((Integer*)argv[3])->getValue (),
                                    ((Integer*)argv[4])->getValue (),
                                    (EventButton*)argv[5]);

   case CLICK:
      return linkReceiver->click ((Widget*)argv[0],
                                  ((Integer*)argv[1])->getValue (),
                                  ((Integer*)argv[2])->getValue (),
                                  ((Integer*)argv[3])->getValue (),
                                  ((Integer*)argv[4])->getValue (),
                                  (EventButton*)argv[5]);

   default:
      misc::assertNotReached ();
   }
   return false;
}

bool Layout::LinkEmitter::emitEnter (Widget *widget, int link, int img,
                                     int x, int y)
{
   Integer ilink (link), iimg (img), ix (x), iy (y);
   Object *argv[5] = { widget, &ilink, &iimg, &ix, &iy };
   return emitBool (ENTER, 5, argv);
}

bool Layout::LinkEmitter::emitPress (Widget *widget, int link, int img,
                                     int x, int y, EventButton *event)
{
   Integer ilink (link), iimg (img), ix (x), iy (y);
   Object *argv[6] = { widget, &ilink, &iimg, &ix, &iy, event };
   return emitBool (PRESS, 6, argv);
}

bool Layout::LinkEmitter::emitRelease (Widget *widget, int link, int img,
                                       int x, int y, EventButton *event)
{
   Integer ilink (link), iimg (img), ix (x), iy (y);
   Object *argv[6] = { widget, &ilink, &iimg, &ix, &iy, event };
   return emitBool (RELEASE, 6, argv);
}

bool Layout::LinkEmitter::emitClick (Widget *widget, int link, int img,
                                     int x, int y, EventButton *event)
{
   Integer ilink (link), iimg (img), ix (x), iy (y);
   Object *argv[6] = { widget, &ilink, &iimg, &ix, &iy, event };
   return emitBool (CLICK, 6, argv);
}

// ---------------------------------------------------------------------

Layout::Anchor::~Anchor ()
{
   free(name);
}

// ---------------------------------------------------------------------

Layout::ScrollTarget::~ScrollTarget ()
{
}

Layout::ScrollTargetBase::ScrollTargetBase (HPosition hPos, VPosition vPos)
{
   this->hPos = hPos;
   this->vPos = vPos;
}

HPosition Layout::ScrollTargetBase::getHPos ()
{
   return hPos;
}

VPosition Layout::ScrollTargetBase::getVPos ()
{
   return vPos;
}

Layout::ScrollTargetFixed::ScrollTargetFixed (HPosition hPos, VPosition vPos,
                                      int x, int y, int width, int height) :
   ScrollTargetBase (hPos, vPos)
{
   this->x = x;
   this->y = y;
   this->width = width;
   this->height = height;
}

int Layout::ScrollTargetFixed::getX ()
{
   return x;
}

int Layout::ScrollTargetFixed::getY ()
{
   return y;
}

int Layout::ScrollTargetFixed::getWidth ()
{
   return width;
}

int Layout::ScrollTargetFixed::getHeight ()
{
   return height;
}

Layout::ScrollTargetWidget::ScrollTargetWidget (HPosition hPos, VPosition vPos,
                                                Widget *widget) :
   ScrollTargetBase (hPos, vPos)
{
   this->widget = widget;
}

int Layout::ScrollTargetWidget::getX ()
{
   return widget->allocation.x;
}

int Layout::ScrollTargetWidget::getY ()
{
   return widget->allocation.y;
}

int Layout::ScrollTargetWidget::getWidth ()
{
   return widget->allocation.width;
}

int Layout::ScrollTargetWidget::getHeight ()
{
   return widget->allocation.ascent + widget->allocation.descent;
}

// ----------------------------------------------------------------------

Layout::Layout (Platform *platform)
{
   this->platform = platform;
   view = NULL;
   topLevel = NULL;
   widgetAtPoint = NULL;

   queueQueueResizeList = new typed::Stack<QueueResizeItem> (true);
   queueResizeList = new typed::Vector<Widget> (4, false);

   DBG_OBJ_CREATE ("dw::core::Layout");

   bgColor = NULL;
   bgImage = NULL;
   cursor = style::CURSOR_DEFAULT;

   canvasWidth = canvasAscent = canvasDescent = 0;

   usesViewport = false;
   drawAfterScrollReq = false;
   scrollX = scrollY = 0;
   viewportWidth = viewportHeight = 0;
   hScrollbarThickness = vScrollbarThickness = 0;

   DBG_OBJ_SET_NUM ("viewportWidth", viewportWidth);
   DBG_OBJ_SET_NUM ("viewportHeight", viewportHeight);
   DBG_OBJ_SET_NUM ("hScrollbarThickness", hScrollbarThickness);
   DBG_OBJ_SET_NUM ("vScrollbarThickness", vScrollbarThickness);

   requestedAnchor = NULL;
   scrollTarget = NULL;
   scrollIdleId = -1;
   scrollIdleNotInterrupted = false;

   anchorsTable =
      new container::typed::HashTable <object::String, Anchor> (true, true);

   resizeIdleId = -1;

   textZone = new misc::ZoneAllocator (16 * 1024);

   DBG_OBJ_ASSOC_CHILD (&findtextState);
   DBG_OBJ_ASSOC_CHILD (&selectionState);

   platform->setLayout (this);

   selectionState.setLayout(this);

   queueResizeCounter = sizeAllocateCounter = sizeRequestCounter =
      getExtremesCounter = 0;

   layoutImgRenderer = NULL;

   resizeIdleCounter = queueResizeCounter = sizeAllocateCounter
      = sizeRequestCounter = getExtremesCounter = 0;
}

Layout::~Layout ()
{
   widgetAtPoint = NULL;

   if (layoutImgRenderer) {
      if (bgImage)
         bgImage->removeExternalImgRenderer (layoutImgRenderer);
      delete layoutImgRenderer;
   }

   if (scrollIdleId != -1)
      platform->removeIdle (scrollIdleId);
   if (resizeIdleId != -1)
      platform->removeIdle (resizeIdleId);
   if (bgColor)
      bgColor->unref ();
   if (bgImage)
      bgImage->unref ();
   if (topLevel) {
      detachWidget (topLevel);
      Widget *w = topLevel;
      topLevel = NULL;
      delete w;
   }

   delete queueQueueResizeList;
   delete queueResizeList;
   delete platform;
   delete view;
   delete anchorsTable;
   delete textZone;

   if (requestedAnchor)
      free (requestedAnchor);
   if (scrollTarget)
      delete scrollTarget;

   DBG_OBJ_DELETE ();
}

void Layout::detachWidget (Widget *widget)
{
   // Called form ~Layout. Sometimes, the widgets (not only the toplevel widget)
   // do some stuff after the layout has been deleted, so *all* widgets have to
   // be detached, and check "layout != NULL" at relevant points.

   // Could be replaced by a virtual method in Widget, like getWidgetAtPoint,
   // if performace were really a problem.

   widget->layout = NULL;
   Iterator *it =
      widget->iterator ((Content::Type)
                        (Content::WIDGET_IN_FLOW | Content::WIDGET_OOF_CONT),
                        false);
   while (it->next ())
      detachWidget (it->getContent()->widget);

   it->unref ();
}

void Layout::addWidget (Widget *widget)
{
   if (topLevel) {
      MSG_WARN("widget already set\n");
      return;
   }

   topLevel = widget;
   widget->layout = this;
   widget->container = NULL;
   DBG_OBJ_SET_PTR_O (widget, "container", widget->container);

   queueResizeList->clear ();
   widget->notifySetAsTopLevel ();

   findtextState.setWidget (widget);

   canvasHeightGreater = false;
   DBG_OBJ_SET_SYM ("canvasHeightGreater",
                    canvasHeightGreater ? "true" : "false");

   // Do not directly call Layout::queueResize(), but
   // Widget::queueResize(), so that all flags are set properly,
   // queueResizeList is filled, etc.
   topLevel->queueResize (-1, false);
}

void Layout::removeWidget ()
{
   /**
    * \bug Some more attributes must be reset here.
    */
   topLevel = NULL;
   queueResizeList->clear ();
   widgetAtPoint = NULL;
   canvasWidth = canvasAscent = canvasDescent = 0;
   scrollX = scrollY = 0;

   view->setCanvasSize (canvasWidth, canvasAscent, canvasDescent);
   if (view->usesViewport ())
      view->setViewportSize (viewportWidth, viewportHeight, 0, 0);
   view->queueDrawTotal ();

   setAnchor (NULL);
   updateAnchor ();

   emitter.emitCanvasSizeChanged (canvasWidth, canvasAscent, canvasDescent);

   findtextState.setWidget (NULL);
   selectionState.reset ();

   updateCursor ();
}

void Layout::setWidget (Widget *widget)
{
   DBG_OBJ_ASSOC_CHILD (widget);

   widgetAtPoint = NULL;
   if (topLevel) {
      Widget *w = topLevel;
      topLevel = NULL;
      delete w;
   }
   textZone->zoneFree ();
   addWidget (widget);

   updateCursor ();
}

/**
 * \brief Attach a view to the layout.
 *
 * It will become a child of the layout,
 * and so it will be destroyed, when the layout will be destroyed.
 */
void Layout::attachView (View *view)
{
   if (this->view)
      MSG_ERR("attachView: Multiple views for layout!\n");

   DBG_OBJ_ASSOC_CHILD (view);

   this->view = view;
   platform->attachView (view);

   /*
    * The layout of the view is set later, first, we "project" the current
    * state of the layout into the new view. A view must handle this without
    * a layout. See also at the end of this function.
    */
   if (bgColor)
      view->setBgColor (bgColor);
   view->setCursor (cursor);
   view->setCanvasSize (canvasWidth, canvasAscent, canvasDescent);

   if (view->usesViewport ()) {
      if (usesViewport) {
         view->scrollTo (scrollX, scrollY);
         view->setViewportSize (viewportWidth, viewportHeight,
                                hScrollbarThickness, vScrollbarThickness);
         hScrollbarThickness = misc::max (hScrollbarThickness,
                                          view->getHScrollbarThickness ());
         vScrollbarThickness = misc::max (vScrollbarThickness,
                                          view->getVScrollbarThickness ());
      }
      else {
         usesViewport = true;
         scrollX = scrollY = 0;
         viewportWidth = viewportHeight = 100; // random values
         hScrollbarThickness = view->getHScrollbarThickness ();
         vScrollbarThickness = view->getVScrollbarThickness ();
      }

      DBG_OBJ_SET_NUM ("viewportWidth", viewportWidth);
      DBG_OBJ_SET_NUM ("viewportHeight", viewportHeight);
      DBG_OBJ_SET_NUM ("hScrollbarThickness", hScrollbarThickness);
      DBG_OBJ_SET_NUM ("vScrollbarThickness", vScrollbarThickness);
   }

   /*
    * This is the last call within this function, so that it is safe for
    * the implementation of dw::core::View::setLayout, to call methods
    * of dw::core::Layout.
    */
   view->setLayout (this);
}

void Layout::detachView (View *view)
{
   if (this->view != view)
      MSG_ERR("detachView: this->view: %p view %p\n", this->view, view);

   view->setLayout (NULL);
   platform->detachView (view);
   this->view = NULL;
   /**
    * \todo Actually, viewportMarkerWidthDiff and
    *       viewportMarkerHeightDiff have to be recalculated here, since the
    *       effective (i.e. maximal) values may change, after the view has been
    *       detached. Same applies to the usage of viewports.
    */
}

void Layout::scroll(ScrollCommand cmd)
{
   if (view->usesViewport ())
      view->scroll(cmd);
}

/**
 * \brief Scrolls all viewports, so that the region [x, y, width, height]
 *    is seen, according to hpos and vpos.
 */
void Layout::scrollTo (HPosition hpos, VPosition vpos,
                       int x, int y, int width, int height)
{
   scrollTo0 (new ScrollTargetFixed (hpos, vpos, x, y, width, height), true);
}

void Layout::scrollToWidget (HPosition hpos, VPosition vpos, Widget *widget)
{
   scrollTo0 (new ScrollTargetWidget (hpos, vpos, widget), true);
}

void Layout::scrollTo0 (ScrollTarget *scrollTarget, bool scrollingInterrupted)
{
   if (usesViewport) {
      _MSG("scrollTo (%d, %d, %s)\n",
           x, y, scrollingInterrupted ? "true" : "false");

      if (this->scrollTarget)
         delete this->scrollTarget;

      this->scrollTarget = scrollTarget;

      if (scrollIdleId == -1) {
         scrollIdleId = platform->addIdle (&Layout::scrollIdle);
         scrollIdleNotInterrupted = true;
      }

      scrollIdleNotInterrupted =
         scrollIdleNotInterrupted || !scrollingInterrupted;
   }
}

void Layout::scrollIdle ()
{
   DBG_OBJ_ENTER0 ("scroll", 0, "scrollIdle");

   bool xChanged = true;
   switch (scrollTarget->getHPos ()) {
   case HPOS_LEFT:
      scrollX = scrollTarget->getX ();
      break;
   case HPOS_CENTER:
      scrollX =
         scrollTarget->getX () - (viewportWidth - currVScrollbarThickness()
                                  - scrollTarget->getWidth ()) / 2;
      break;
   case HPOS_RIGHT:
      scrollX =
         scrollTarget->getX () - (viewportWidth - currVScrollbarThickness()
                                  - scrollTarget->getWidth ());
      break;
   case HPOS_INTO_VIEW:
      xChanged = calcScrollInto (scrollTarget->getX (),
                                 scrollTarget->getWidth (), &scrollX,
                                 viewportWidth - currVScrollbarThickness());
      break;
   case HPOS_NO_CHANGE:
      xChanged = false;
      break;
   }

   bool yChanged = true;
   switch (scrollTarget->getVPos ()) {
   case VPOS_TOP:
      scrollY = scrollTarget->getY ();
      break;
   case VPOS_CENTER:
      scrollY =
         scrollTarget->getY () - (viewportHeight - currHScrollbarThickness()
                                  - scrollTarget->getHeight ()) / 2;
      break;
   case VPOS_BOTTOM:
      scrollY =
         scrollTarget->getY () - (viewportHeight - currHScrollbarThickness()
                                  - scrollTarget->getHeight ());
      break;
   case VPOS_INTO_VIEW:
      yChanged = calcScrollInto (scrollTarget->getY (),
                                 scrollTarget->getHeight (), &scrollY,
                                 viewportHeight - currHScrollbarThickness());
      break;
   case VPOS_NO_CHANGE:
      yChanged = false;
      break;
   }

   DBG_OBJ_MSGF ("scroll", 1, "xChanged = %s, yChanged = %s",
                 xChanged ? "true" : "false", yChanged ? "true" : "false");

   if (xChanged || yChanged) {
      adjustScrollPos ();
      view->scrollTo (scrollX, scrollY);
   }

   // The following code was once inside the block above ("if
   // (xChanged || yChanged)"). I'm not sure but it looks that this
   // should be outside, or at least setting drawAfterScrollReq in
   // Layout::draw should consider whether the scroll position will
   // change (but this would be rather difficult). --SG

   if (drawAfterScrollReq) {
      drawAfterScrollReq = false;
      view->queueDrawTotal ();
   }

   delete scrollTarget;
   scrollTarget = NULL;

   scrollIdleId = -1;

   DBG_OBJ_LEAVE ();
}

void Layout::adjustScrollPos ()
{
   scrollX = misc::min (scrollX,
      canvasWidth - (viewportWidth - vScrollbarThickness));
   scrollX = misc::max (scrollX, 0);

   scrollY = misc::min (scrollY,
      canvasAscent + canvasDescent - (viewportHeight - hScrollbarThickness));
   scrollY = misc::max (scrollY, 0);

   _MSG("adjustScrollPos: scrollX=%d scrollY=%d\n", scrollX, scrollY);
}

bool Layout::calcScrollInto (int requestedValue, int requestedSize,
                             int *value, int viewportSize)
{
   if (requestedSize > viewportSize) {
      // The viewport size is smaller than the size of the region which will
      // be shown. If the region is already visible, do not change the
      // position. Otherwise, show the left/upper border, this is most likely
      // what is needed.
      if (*value >= requestedValue &&
          *value + viewportSize < requestedValue + requestedSize)
         return false;
      else
         requestedSize = viewportSize;
   }

   if (requestedValue < *value) {
      *value = requestedValue;
      return true;
   } else if (requestedValue + requestedSize > *value + viewportSize) {
      *value = requestedValue - viewportSize + requestedSize;
      return true;
   } else
      return false;
}

void Layout::draw (View *view, Rectangle *area)
{
   DBG_OBJ_ENTER ("draw", 0, "draw", "%d, %d, %d * %d",
                  area->x, area->y, area->width, area->height);

   Rectangle widgetArea, intersection, widgetDrawArea;

   // First of all, draw background image. (Unlike background *color*,
   // this is not a feature of the views.)
   if (bgImage != NULL && bgImage->getImgbufSrc() != NULL)
      style::drawBackgroundImage (view, bgImage, bgRepeat, bgAttachment,
                                  bgPositionX, bgPositionY,
                                  area->x, area->y, area->width,
                                  area->height, 0, 0,
                                  // Reference area: maximum of canvas size and
                                  // viewport size.
                                  misc::max (viewportWidth
                                             - (canvasHeightGreater ?
                                                vScrollbarThickness : 0),
                                             canvasWidth),
                                  misc::max (viewportHeight
                                             - hScrollbarThickness,
                                             canvasAscent + canvasDescent));

   if (scrollIdleId != -1) {
      /* scroll is pending, defer draw until after scrollIdle() */
      drawAfterScrollReq = true;
      DBG_OBJ_MSGF ("draw", 1, "scrollIdleId = %d => delayed", scrollIdleId);
   } else if (topLevel) {
      /* Draw the top level widget. */
      widgetArea.x = topLevel->allocation.x;
      widgetArea.y = topLevel->allocation.y;
      widgetArea.width = topLevel->allocation.width;
      widgetArea.height = topLevel->getHeight ();

      if (area->intersectsWith (&widgetArea, &intersection)) {
         DBG_OBJ_MSG ("draw", 1, "drawing toplevel widget");

         view->startDrawing (&intersection);

         /* Intersection in widget coordinates. */
         widgetDrawArea.x = intersection.x - topLevel->allocation.x;
         widgetDrawArea.y = intersection.y - topLevel->allocation.y;
         widgetDrawArea.width = intersection.width;
         widgetDrawArea.height = intersection.height;

         topLevel->draw (view, &widgetDrawArea);

         view->finishDrawing (&intersection);
      } else
         DBG_OBJ_MSG ("draw", 1, "no intersection");
   } else
      DBG_OBJ_MSG ("draw", 1, "no toplevel widget");

   DBG_OBJ_LEAVE ();
}

int Layout::currHScrollbarThickness()
{
   return (canvasWidth > viewportWidth) ? hScrollbarThickness : 0;
}

int Layout::currVScrollbarThickness()
{
   return (canvasAscent + canvasDescent > viewportHeight) ?
          vScrollbarThickness : 0;
}

/**
 * Sets the anchor to scroll to.
 */
void Layout::setAnchor (const char *anchor)
{
   _MSG("setAnchor (%s)\n", anchor);

   if (requestedAnchor)
      free (requestedAnchor);
   requestedAnchor = anchor ? strdup (anchor) : NULL;
   updateAnchor ();
}

/**
 * Used, when the widget is not allocated yet.
 */
char *Layout::addAnchor (Widget *widget, const char* name)
{
   return addAnchor (widget, name, -1);
}

char *Layout::addAnchor (Widget *widget, const char* name, int y)
{
   String key (name);
   if (anchorsTable->contains (&key))
      return NULL;
   else {
      Anchor *anchor = new Anchor ();
      anchor->name = strdup (name);
      anchor->widget = widget;
      anchor->y = y;

      anchorsTable->put (new String (name), anchor);
      updateAnchor ();

      return anchor->name;
   }
}

void Layout::changeAnchor (Widget *widget, char* name, int y)
{
   String key (name);
   Anchor *anchor = anchorsTable->get (&key);
   assert (anchor);
   assert (anchor->widget == widget);
   anchor->y = y;
   updateAnchor ();
}

void Layout::removeAnchor (Widget *widget, char* name)
{
   String key (name);
   anchorsTable->remove (&key);
}

void Layout::updateAnchor ()
{
   Anchor *anchor;
   if (requestedAnchor) {
      String key (requestedAnchor);
      anchor = anchorsTable->get (&key);
   } else
      anchor = NULL;

   if (anchor == NULL) {
      /** \todo Copy comment from old docs. */
      if (scrollIdleId != -1 && !scrollIdleNotInterrupted) {
         platform->removeIdle (scrollIdleId);
         scrollIdleId = -1;
      }
   } else
      if (anchor->y != -1)
         scrollTo0 (new ScrollTargetFixed (HPOS_NO_CHANGE, VPOS_TOP,
                                           0, anchor->y, 0, 0), false);
}

void Layout::setCursor (style::Cursor cursor)
{
   if (cursor != this->cursor) {
      this->cursor = cursor;
      view->setCursor (cursor);
   }
}

void Layout::updateCursor ()
{
   if (widgetAtPoint && widgetAtPoint->style)
      setCursor (widgetAtPoint->style->cursor);
   else
      setCursor (style::CURSOR_DEFAULT);
}

void Layout::setBgColor (style::Color *color)
{
   color->ref ();

   if (bgColor)
      bgColor->unref ();

   bgColor = color;

   if (view)
      view->setBgColor (bgColor);
}

void Layout::setBgImage (style::StyleImage *bgImage,
                         style::BackgroundRepeat bgRepeat,
                         style::BackgroundAttachment bgAttachment,
                         style::Length bgPositionX, style::Length bgPositionY)
{
   if (layoutImgRenderer && this->bgImage)
      this->bgImage->removeExternalImgRenderer (layoutImgRenderer);

   if (bgImage)
      bgImage->ref ();

   if (this->bgImage)
      this->bgImage->unref ();

   this->bgImage = bgImage;
   this->bgRepeat = bgRepeat;
   this->bgAttachment = bgAttachment;
   this->bgPositionX = bgPositionX;
   this->bgPositionY = bgPositionY;

   if (bgImage) {
      // Create instance of LayoutImgRenderer when needed. Until this
      // layout is deleted, "layoutImgRenderer" will be kept, since it
      // is not specific to the style, but only to this layout.
      if (layoutImgRenderer == NULL)
         layoutImgRenderer = new LayoutImgRenderer (this);
      bgImage->putExternalImgRenderer (layoutImgRenderer);
   }
}


void Layout::resizeIdle ()
{
   DBG_OBJ_ENTER0 ("resize", 0, "resizeIdle");

   enterResizeIdle ();

   //static int calls = 0;
   //printf ("Layout::resizeIdle calls = %d\n", ++calls);

   assert (resizeIdleId != -1);

   for (typed::Iterator <Widget> it = queueResizeList->iterator();
        it.hasNext (); ) {
      Widget *widget = it.getNext ();

      //printf ("   the %stop-level %s %p was queued (extremes changed: %s)\n",
      //        widget->parent ? "non-" : "", widget->getClassName(), widget,
      //        widget->extremesQueued () ? "yes" : "no");

      if (widget->resizeQueued ()) {
         widget->setFlags (Widget::NEEDS_RESIZE);
         widget->unsetFlags (Widget::RESIZE_QUEUED);
      }

      if (widget->allocateQueued ()) {
         widget->setFlags (Widget::NEEDS_ALLOCATE);
         widget->unsetFlags (Widget::ALLOCATE_QUEUED);
      }

      if (widget->extremesQueued ()) {
         widget->setFlags (Widget::EXTREMES_CHANGED);
         widget->unsetFlags (Widget::EXTREMES_QUEUED);
      }
   }
   queueResizeList->clear ();

   // Reset already here, since in this function, queueResize() may be
   // called again.
   resizeIdleId = -1;

   // If this method is triggered by a viewport change, we can save
   // time when the toplevel widget is not affected (as for a toplevel
   // image resource).
   if (topLevel && (topLevel->needsResize () || topLevel->needsAllocate ())) {
      Requisition requisition;
      Allocation allocation;

      topLevel->sizeRequest (&requisition);
      DBG_OBJ_MSGF ("resize", 1, "toplevel size: %d * (%d + %d)",
                    requisition.width, requisition.ascent, requisition.descent);

      // This method is triggered by Widget::queueResize, which will,
      // in any case, set NEEDS_ALLOCATE (indirectly, as ALLOCATE_QUEUED).
      // This assertion helps to find inconsistences. (Cases where
      // this method is triggered by a viewport change, but the
      // toplevel widget is not affected, are filtered out some lines
      // above: "if (topLevel && topLevel->needsResize ())".)
      assert (topLevel->needsAllocate ());

      allocation.x = allocation.y = 0;
      allocation.width = requisition.width;
      allocation.ascent = requisition.ascent;
      allocation.descent = requisition.descent;
      topLevel->sizeAllocate (&allocation);

      canvasWidth = requisition.width;
      canvasAscent = requisition.ascent;
      canvasDescent = requisition.descent;

      emitter.emitCanvasSizeChanged (canvasWidth, canvasAscent, canvasDescent);

      // Tell the view about the new world size.
      view->setCanvasSize (canvasWidth, canvasAscent, canvasDescent);
      //  view->queueDrawTotal (false);

      if (usesViewport) {
         int currHThickness = currHScrollbarThickness();
         int currVThickness = currVScrollbarThickness();

         if (!canvasHeightGreater &&
             canvasAscent + canvasDescent  > viewportHeight - currHThickness) {
            canvasHeightGreater = true;
            DBG_OBJ_SET_SYM ("canvasHeightGreater",
                             canvasHeightGreater ? "true" : "false");
            containerSizeChanged ();
         }

         // Set viewport sizes.
         view->setViewportSize (viewportWidth, viewportHeight,
                                currHThickness, currVThickness);
      }

      // views are redrawn via Widget::resizeDrawImpl ()
   }

   updateAnchor ();

   DBG_OBJ_MSGF ("resize", 1,
                 "after resizeIdle: resizeIdleId = %d", resizeIdleId);
   DBG_OBJ_LEAVE ();

   leaveResizeIdle ();
}

void Layout::queueDraw (int x, int y, int width, int height)
{
   DBG_OBJ_ENTER ("draw", 0, "queueDrawArea", "%d, %d, %d, %d",
                  x, y, width, height);

   Rectangle area;
   area.x = x;
   area.y = y;
   area.width = width;
   area.height = height;

   if (!area.isEmpty ())
      view->queueDraw (&area);

   DBG_OBJ_LEAVE ();
}

void Layout::queueDrawExcept (int x, int y, int width, int height,
   int ex, int ey, int ewidth, int eheight) {

   if (x == ex && y == ey && width == ewidth && height == eheight)
      return;

   // queueDraw() the four rectangles within rectangle (x, y, width, height)
   // around rectangle (ex, ey, ewidth, eheight).
   // Some or all of these may be empty.

   // upper left corner of the intersection rectangle
   int ix1 = misc::max (x, ex);
   int iy1 = misc::max (y, ey);
   // lower right corner of the intersection rectangle
   int ix2 = misc::min (x + width, ex + ewidth);
   int iy2 = misc::min (y + height, ey + eheight);

   queueDraw (x, y, width, iy1 - y);
   queueDraw (x, iy2, width, y + height - iy2);
   queueDraw (x, iy1, ix1 - x, iy2 - iy1);
   queueDraw (ix2, iy1, x + width - ix2, iy2 - iy1);
}

void Layout::queueResize (bool extremesChanged)
{
   DBG_OBJ_ENTER ("resize", 0, "queueResize", "%s",
                  extremesChanged ? "true" : "false");

   if (resizeIdleId == -1) {
      view->cancelQueueDraw ();

      resizeIdleId = platform->addIdle (&Layout::resizeIdle);
      DBG_OBJ_MSGF ("resize", 1, "setting resizeIdleId = %d", resizeIdleId);
   }

   emitter.emitResizeQueued (extremesChanged);

   DBG_OBJ_LEAVE ();
}


// Views

bool Layout::buttonEvent (ButtonEventType type, View *view, int numPressed,
                          int x, int y, ButtonState state, int button)

{
   EventButton event;

   moveToWidgetAtPoint (x, y, state);

   event.xCanvas = x;
   event.yCanvas = y;
   event.state = state;
   event.button = button;
   event.numPressed = numPressed;

   return processMouseEvent (&event, type);
}

/**
 * \brief This function is called by a view, to delegate a motion notify
 * event.
 *
 * Arguments are similar to dw::core::Layout::buttonPress.
 */
bool Layout::motionNotify (View *view,  int x, int y, ButtonState state)
{
   EventButton event;

   moveToWidgetAtPoint (x, y, state);

   event.xCanvas = x;
   event.yCanvas = y;
   event.state = state;

   return processMouseEvent (&event, MOTION_NOTIFY);
}

/**
 * \brief This function is called by a view, to delegate a enter notify event.
 *
 * Arguments are similar to dw::core::Layout::buttonPress.
 */
void Layout::enterNotify (View *view, int x, int y, ButtonState state)
{
   Widget *lastWidget;
   EventCrossing event;

   lastWidget = widgetAtPoint;
   moveToWidgetAtPoint (x, y, state);

   if (widgetAtPoint) {
      event.state = state;
      event.lastWidget = lastWidget;
      event.currentWidget = widgetAtPoint;
      widgetAtPoint->enterNotify (&event);
   }
}

/**
 * \brief This function is called by a view, to delegate a leave notify event.
 *
 * Arguments are similar to dw::core::Layout::buttonPress.
 */
void Layout::leaveNotify (View *view, ButtonState state)
{
#if 0
   Widget *lastWidget;
   EventCrossing event;

   lastWidget = widgetAtPoint;
   moveOutOfView (state);

   if (lastWidget) {
      event.state = state;
      event.lastWidget = lastWidget;
      event.currentWidget = widgetAtPoint;
      lastWidget->leaveNotify (&event);
   }
#else
   moveOutOfView (state);
#endif
}

/*
 * Return the widget at position (x, y). Return NULL, if there is no widget.
 */
Widget *Layout::getWidgetAtPoint (int x, int y)
{
   _MSG ("------------------------------------------------------------\n");
   _MSG ("widget at (%d, %d)\n", x, y);
   if (topLevel && topLevel->wasAllocated ())
      return topLevel->getWidgetAtPoint (x, y, 0);
   else
      return NULL;
}


/*
 * Emit the necessary crossing events, when the mouse pointer has moved to
 * the given widget (by mouse or scrolling).
 */
void Layout::moveToWidget (Widget *newWidgetAtPoint, ButtonState state)
{
   Widget *ancestor, *w;
   Widget **track;
   int trackLen, i, i_a;
   EventCrossing crossingEvent;

   _MSG("moveToWidget: wap=%p nwap=%p\n",widgetAtPoint,newWidgetAtPoint);
   if (newWidgetAtPoint != widgetAtPoint) {
      // The mouse pointer has been moved into another widget.
      if (newWidgetAtPoint && widgetAtPoint)
         ancestor =
            newWidgetAtPoint->getNearestCommonAncestor (widgetAtPoint);
      else if (newWidgetAtPoint)
         ancestor = newWidgetAtPoint->getTopLevel ();
      else
         ancestor = widgetAtPoint->getTopLevel ();

      // Construct the track.
      trackLen = 0;
      if (widgetAtPoint)
         // first part
         for (w = widgetAtPoint; w != ancestor; w = w->getParent ())
            trackLen++;
      trackLen++; // for the ancestor
      if (newWidgetAtPoint)
         // second part
         for (w = newWidgetAtPoint; w != ancestor; w = w->getParent ())
            trackLen++;

      track = new Widget* [trackLen];
      i = 0;
      if (widgetAtPoint)
         /* first part */
         for (w = widgetAtPoint; w != ancestor; w = w->getParent ())
            track[i++] = w;
      i_a = i;
      track[i++] = ancestor;
      if (newWidgetAtPoint) {
         /* second part */
         i = trackLen - 1;
         for (w = newWidgetAtPoint; w != ancestor; w = w->getParent ())
            track[i--] = w;
      }
#if 0
      MSG("Track: %s[ ", widgetAtPoint ? "" : "nil ");
      for (i = 0; i < trackLen; i++)
         MSG("%s%p ", i == i_a ? ">" : "", track[i]);
      MSG("] %s\n", newWidgetAtPoint ? "" : "nil");
#endif

      /* Send events to the widgets on the track */
      for (i = 0; i < trackLen; i++) {
         crossingEvent.state = state;
         crossingEvent.currentWidget = widgetAtPoint; // ???
         crossingEvent.lastWidget = widgetAtPoint; // ???
         if (i < i_a) {
            track[i]->leaveNotify (&crossingEvent);
         } else if (i == i_a) { /* ancestor */
            /* Don't touch ancestor unless:
             *   - moving into/from NULL,
             *   - ancestor becomes the newWidgetAtPoint */
            if (i_a == trackLen-1 && !newWidgetAtPoint)
               track[i]->leaveNotify (&crossingEvent);
            else if ((i_a == 0 && !widgetAtPoint) ||
                     (i_a == trackLen-1 && newWidgetAtPoint))
               track[i]->enterNotify (&crossingEvent);
         } else {
            track[i]->enterNotify (&crossingEvent);
         }
      }

      delete[] track;

      widgetAtPoint = newWidgetAtPoint;
      updateCursor ();
   }
}

/**
 * \brief Common processing of press, release and motion events.
 *
 * This function depends on that move_to_widget_at_point()
 * has been called before.
 */
bool Layout::processMouseEvent (MousePositionEvent *event,
                                ButtonEventType type)
{
   Widget *widget;

   /*
    * If the event is outside of the visible region of the canvas, treat it
    * as occurring at the region's edge. Notably, this helps when selecting
    * text.
    */
   if (event->xCanvas < scrollX)
      event->xCanvas = scrollX;
   else {
      int maxX = scrollX + viewportWidth - currVScrollbarThickness() - 1;

      if (event->xCanvas > maxX)
         event->xCanvas = maxX;
   }
   if (event->yCanvas < scrollY)
      event->yCanvas = scrollY;
   else {
      int maxY = misc::min(scrollY + viewportHeight -currHScrollbarThickness(),
                           canvasAscent + canvasDescent) - 1;

      if (event->yCanvas > maxY)
         event->yCanvas = maxY;
   }

   widget = getWidgetAtPoint(event->xCanvas, event->yCanvas);

   for (; widget; widget = widget->getParent ()) {
      if (widget->isButtonSensitive ()) {
         event->xWidget = event->xCanvas - widget->getAllocation()->x;
         event->yWidget = event->yCanvas - widget->getAllocation()->y;

         switch (type) {
         case BUTTON_PRESS:
            return widget->buttonPress ((EventButton*)event);

         case BUTTON_RELEASE:
            return widget->buttonRelease ((EventButton*)event);

         case MOTION_NOTIFY:
            return widget->motionNotify ((EventMotion*)event);

         default:
            misc::assertNotReached ();
         }
      }
   }
   if (type == BUTTON_PRESS)
      return emitLinkPress (NULL, -1, -1, -1, -1, (EventButton*)event);
   else if (type == BUTTON_RELEASE)
      return emitLinkRelease(NULL, -1, -1, -1, -1, (EventButton*)event);

   return false;
}

/*
 * This function must be called by a view, when the user has manually changed
 * the viewport position. It is *not* called, when the layout has requested the
 * position change.
 */
void Layout::scrollPosChanged (View *view, int x, int y)
{
   if (x != scrollX || y != scrollY) {
      scrollX = x;
      scrollY = y;

      setAnchor (NULL);
      updateAnchor ();
   }
}

/*
 * This function must be called by a viewport view, when its viewport size has
 * changed. It is *not* called, when the layout has requested the size change.
 */
void Layout::viewportSizeChanged (View *view, int width, int height)
{
   DBG_OBJ_ENTER ("resize", 0, "viewportSizeChanged", "%p, %d, %d",
                 view, width, height);

   /* If the width has become higher, we test again, whether the vertical
    * scrollbar (so to speak) can be hidden again. */
   if (usesViewport && width > viewportWidth) {
      canvasHeightGreater = false;
      DBG_OBJ_SET_SYM ("canvasHeightGreater",
                       canvasHeightGreater ? "true" : "false");
   }

   /* if size changes, redraw this view.
    * TODO: this is a resize call (redraw/resize code needs a review). */
   if (viewportWidth != width || viewportHeight != height) {
      if (topLevel)
         // similar to addWidget()
         topLevel->queueResize (-1, false);
      else
         queueResize (false);
   }

   viewportWidth = width;
   viewportHeight = height;

   DBG_OBJ_SET_NUM ("viewportWidth", viewportWidth);
   DBG_OBJ_SET_NUM ("viewportHeight", viewportHeight);

   containerSizeChanged ();

   DBG_OBJ_LEAVE ();
}

void Layout::containerSizeChanged ()
{
   DBG_OBJ_ENTER0 ("resize", 0, "containerSizeChanged");

   if (topLevel) {
      topLevel->containerSizeChanged ();
      queueResize (true);
   }

   DBG_OBJ_LEAVE ();
}

} // namespace core
} // namespace dw
