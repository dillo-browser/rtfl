#ifndef __DW_LAYOUT_HH__
#define __DW_LAYOUT_HH__

#ifndef __INCLUDED_FROM_DW_CORE_HH__
#   error Do not include this file directly, use "core.hh" instead.
#endif

namespace dw {
namespace core {

/**
 * \brief The central class for managing and drawing a widget tree.
 *
 * \sa\ref dw-overview, \ref dw-layout-widgets, \ref dw-layout-views
 */
class Layout: public lout::object::Object
{
   friend class Widget;

private:
   class LayoutImgRenderer: public style::StyleImage::ExternalImgRenderer
   {
      Layout *layout;

   public:
      LayoutImgRenderer (Layout *layout) { this->layout = layout; }

      bool readyToDraw ();
      void getBgArea (int *x, int *y, int *width, int *height);
      void getRefArea (int *xRef, int *yRef, int *widthRef, int *heightRef);
      style::StyleImage *getBackgroundImage ();
      style::BackgroundRepeat getBackgroundRepeat ();
      style::BackgroundAttachment getBackgroundAttachment ();
      style::Length getBackgroundPositionX ();
      style::Length getBackgroundPositionY ();
      void draw (int x, int y, int width, int height);
   };

   LayoutImgRenderer *layoutImgRenderer;

public:
   /**
    * \brief Receiver interface different signals.
    *
    * May be extended.
    */
   class Receiver: public lout::signal::Receiver
   {
   public:
      virtual void resizeQueued (bool extremesChanged);
      virtual void canvasSizeChanged (int width, int ascent, int descent);
   };

   class LinkReceiver: public lout::signal::Receiver
   {
   public:
      /**
       * \brief Called, when a link is entered, left, or the position has
       *    changed.
       *
       * When a link is entered, this method is called with the respective
       * arguments. When a link is left, this method is called with all
       * three arguments (\em link, \em x, \em y) set to -1.
       *
       * When coordinates are supported, a change of the coordinates also
       * causes emitting this signal.
       */
      virtual bool enter (Widget *widget, int link, int img, int x, int y);

      /**
       * \brief Called, when the user has pressed the mouse button on a
       *    link (but not yet released).
       *
       * The causing event is passed as \em event.
       */
      virtual bool press (Widget *widget, int link, int img, int x, int y,
                          EventButton *event);

      /**
       * \brief Called, when the user has released the mouse button on a
       *    link.
       *
       * The causing event is passed as \em event.
       */
      virtual bool release (Widget *widget, int link, int img, int x, int y,
                            EventButton *event);

      /**
       * \brief Called, when the user has clicked on a link.
       *
       * For mouse interaction, this is equivalent to "press" and "release"
       * on the same link. In this case, \em event contains the "release"
       * event.
       *
       *
       * When activating links via keyboard is supported, only a "clicked"
       * signal will be emitted, and \em event will be NULL.
       */
      virtual bool click (Widget *widget, int link, int img, int x, int y,
                          EventButton *event);
   };

   class LinkEmitter: public lout::signal::Emitter
   {
   private:
      enum { ENTER, PRESS, RELEASE, CLICK };

   protected:
      bool emitToReceiver (lout::signal::Receiver *receiver, int signalNo,
                           int argc, lout::object::Object **argv);

   public:
      inline void connectLink (LinkReceiver *receiver) { connect (receiver); }

      bool emitEnter (Widget *widget, int link, int img, int x, int y);
      bool emitPress (Widget *widget, int link, int img, int x, int y,
                      EventButton *event);
      bool emitRelease (Widget *widget, int link, int img, int x, int y,
                        EventButton *event);
      bool emitClick (Widget *widget, int link, int img, int x, int y,
                      EventButton *event);
   };

   LinkEmitter linkEmitter;

private:
   class Emitter: public lout::signal::Emitter
   {
   private:
      enum { RESIZE_QUEUED, CANVAS_SIZE_CHANGED };

   protected:
      bool emitToReceiver (lout::signal::Receiver *receiver, int signalNo,
                           int argc, lout::object::Object **argv);

   public:
      inline void connectLayout (Receiver *receiver) { connect (receiver); }

      void emitResizeQueued (bool extremesChanged);
      void emitCanvasSizeChanged (int width, int ascent, int descent);
   };

   Emitter emitter;

   class Anchor: public lout::object::Object
   {
   public:
      char *name;
      Widget *widget;
      int y;

      ~Anchor ();
   };

   class QueueResizeItem: public lout::object::Object
   {
   public:
      Widget *widget;
      int ref;
      bool extremesChanged, fast;

      inline QueueResizeItem (Widget *widget, int ref, bool extremesChanged,
                              bool fast)
      {
         this->widget = widget;
         this->ref = ref;
         this->extremesChanged = extremesChanged;
         this->fast = fast;
      }
   };

   /**
    * \brief An abstract scrolling target. The values are first
    *    calculated when they are needed in scrollIdle().
    *
    * (Note: perhaps a subclass should be uses for anchors.)
    */
   class ScrollTarget
   {
   public:
      virtual ~ScrollTarget ();

      virtual HPosition getHPos () = 0;
      virtual VPosition getVPos () = 0;
      virtual int getX () = 0;
      virtual int getY () = 0;
      virtual int getWidth () = 0;
      virtual int getHeight () = 0;
   };

   class ScrollTargetBase: public ScrollTarget
   {
      HPosition hPos;
      VPosition vPos;

   public:
      ScrollTargetBase (HPosition hPos, VPosition vPos);

      HPosition getHPos ();
      VPosition getVPos ();
   };

   /**
    * \brief Scrolling target with concrete values.
    */
   class ScrollTargetFixed: public ScrollTargetBase
   {
      int x, y, width, height;

   public:
      ScrollTargetFixed (HPosition hPos, VPosition vPos,
                         int x, int y, int width, int height);

      int getX ();
      int getY ();
      int getWidth ();
      int getHeight ();
   };

   /**
    * \brief Scrolling target for a widget allocation.
    *
    * If the widget is allocated between scrollToWidget() and
    * scrollIdle(), this is taken into account.
    */
   class ScrollTargetWidget: public ScrollTargetBase
   {
      Widget *widget;

   public:
      ScrollTargetWidget (HPosition hPos, VPosition vPos, Widget *widget);

      int getX ();
      int getY ();
      int getWidth ();
      int getHeight ();
   };

   Platform *platform;
   View *view;
   Widget *topLevel, *widgetAtPoint;
   lout::container::typed::Stack<QueueResizeItem> *queueQueueResizeList;
   lout::container::typed::Vector<Widget> *queueResizeList;

   /* The state, which must be projected into the view. */
   style::Color *bgColor;
   style::StyleImage *bgImage;
   style::BackgroundRepeat bgRepeat;
   style::BackgroundAttachment bgAttachment;
   style::Length bgPositionX, bgPositionY;

   style::Cursor cursor;
   int canvasWidth, canvasAscent, canvasDescent;

   bool usesViewport, drawAfterScrollReq;
   int scrollX, scrollY, viewportWidth, viewportHeight;
   bool canvasHeightGreater;
   int hScrollbarThickness, vScrollbarThickness;

   ScrollTarget *scrollTarget;

   char *requestedAnchor;
   int scrollIdleId, resizeIdleId;
   bool scrollIdleNotInterrupted;

   /* Anchors of the widget tree */
   lout::container::typed::HashTable <lout::object::String, Anchor>
                                     *anchorsTable;

   SelectionState selectionState;
   FindtextState findtextState;

   enum ButtonEventType { BUTTON_PRESS, BUTTON_RELEASE, MOTION_NOTIFY };

   void detachWidget (Widget *widget);

   Widget *getWidgetAtPoint (int x, int y);
   void moveToWidget (Widget *newWidgetAtPoint, ButtonState state);

   /**
    * \brief Emit the necessary crossing events, when the mouse pointer has
    *    moved to position (\em x, \em );
    */
   void moveToWidgetAtPoint (int x, int y, ButtonState state)
   { moveToWidget (getWidgetAtPoint (x, y), state); }

   /**
    * \brief Emit the necessary crossing events, when the mouse pointer
    * has moved out of the view.
    */
   void moveOutOfView (ButtonState state) { moveToWidget (NULL, state); }

   bool processMouseEvent (MousePositionEvent *event, ButtonEventType type);
   bool buttonEvent (ButtonEventType type, View *view,
                     int numPressed, int x, int y, ButtonState state,
                     int button);
   void resizeIdle ();
   void setSizeHints ();
   void draw (View *view, Rectangle *area);

   void scrollTo0(ScrollTarget *scrollTarget, bool scrollingInterrupted);
   void scrollIdle ();
   void adjustScrollPos ();
   static bool calcScrollInto (int targetValue, int requestedSize,
                               int *value, int viewportSize);
   int currHScrollbarThickness();
   int currVScrollbarThickness();

   void updateAnchor ();

   /* Widget */

   char *addAnchor (Widget *widget, const char* name);
   char *addAnchor (Widget *widget, const char* name, int y);
   void changeAnchor (Widget *widget, char* name, int y);
   void removeAnchor (Widget *widget, char* name);
   void setCursor (style::Cursor cursor);
   void updateCursor ();
   void queueDraw (int x, int y, int width, int height);
   void queueDrawExcept (int x, int y, int width, int height,
      int ex, int ey, int ewidth, int eheight);
   void queueResize (bool extremesChanged);
   void removeWidget ();

   /* For tests regarding the respective Layout and (mostly) Widget
      methods. Accessed by respective methods (enter..., leave...,
      ...Entered) defined here and in Widget. */

   int resizeIdleCounter, queueResizeCounter, sizeAllocateCounter,
      sizeRequestCounter, getExtremesCounter;

   void enterResizeIdle () { resizeIdleCounter++; }
   void leaveResizeIdle () { resizeIdleCounter--; }

public:
   Layout (Platform *platform);
   ~Layout ();

   inline void connectLink (LinkReceiver *receiver)
   { linkEmitter.connectLink (receiver); }

   inline bool emitLinkEnter (Widget *w, int link, int img, int x, int y)
   { return linkEmitter.emitEnter (w, link, img, x, y); }

   inline bool emitLinkPress (Widget *w, int link, int img,
                              int x, int y, EventButton *event)
   { return linkEmitter.emitPress (w, link, img, x, y, event); }

   inline bool emitLinkRelease (Widget *w, int link, int img,
                                int x, int y, EventButton *event)
   { return linkEmitter.emitRelease (w, link, img, x, y, event); }

   inline bool emitLinkClick (Widget *w, int link, int img,
                              int x, int y, EventButton *event)
   { return linkEmitter.emitClick (w, link, img, x, y, event); }

   lout::misc::ZoneAllocator *textZone;

   void addWidget (Widget *widget);
   void setWidget (Widget *widget);

   void attachView (View *view);
   void detachView (View *view);

   inline bool getUsesViewport () { return usesViewport; }
   inline int getWidthViewport () { return viewportWidth; }
   inline int getHeightViewport ()  { return viewportHeight; }
   inline int getScrollPosX ()  { return scrollX; }
   inline int getScrollPosY ()  { return scrollY; }

   /* public */

   void scrollTo (HPosition hpos, VPosition vpos,
                  int x, int y, int width, int height);
   void scrollToWidget (HPosition hpos, VPosition vpos, Widget *widget);
   void scroll (ScrollCommand cmd);
   void setAnchor (const char *anchor);

   /* View */

   inline void expose (View *view, Rectangle *area) {
      DBG_OBJ_ENTER ("draw", 0, "expose", "%d, %d, %d * %d",
                     area->x, area->y, area->width, area->height);
      draw (view, area);
      DBG_OBJ_LEAVE ();
   }

   /**
    * \brief This function is called by a view, to delegate a button press
    * event.
    *
    * \em numPressed is 1 for simple presses, 2 for double presses etc. (more
    * that 2 is never needed), \em x and \em y the world coordinates, and
    * \em button the number of the button pressed.
    */
   inline bool buttonPress (View *view, int numPressed, int x, int y,
                            ButtonState state, int button)
   {
      return buttonEvent (BUTTON_PRESS, view, numPressed, x, y, state, button);
   }

   void containerSizeChanged ();

   /**
    * \brief This function is called by a view, to delegate a button press
    * event.
    *
    * Arguments are similar to dw::core::Layout::buttonPress.
    */
   inline bool buttonRelease (View *view, int numPressed, int x, int y,
                              ButtonState state, int button)
   {
      return buttonEvent (BUTTON_RELEASE, view, numPressed, x, y, state,
                          button);
   }

   bool motionNotify (View *view,  int x, int y, ButtonState state);
   void enterNotify (View *view, int x, int y, ButtonState state);
   void leaveNotify (View *view, ButtonState state);

   void scrollPosChanged (View *view, int x, int y);
   void viewportSizeChanged (View *view, int width, int height);

   inline Platform *getPlatform ()
   {
      return platform;
   }

   /* delegated */

   inline int textWidth (style::Font *font, const char *text, int len)
   {
      return platform->textWidth (font, text, len);
   }

   inline char *textToUpper (const char *text, int len)
   {
      return platform->textToUpper (text, len);
   }

   inline char *textToLower (const char *text, int len)
   {
      return platform->textToLower (text, len);
   }

   inline int nextGlyph (const char *text, int idx)
   {
      return platform->nextGlyph (text, idx);
   }

   inline int prevGlyph (const char *text, int idx)
   {
      return platform->prevGlyph (text, idx);
   }

   inline float dpiX ()
   {
      return platform->dpiX ();
   }

   inline float dpiY ()
   {
      return platform->dpiY ();
   }

   inline style::Font *createFont (style::FontAttrs *attrs, bool tryEverything)
   {
      return platform->createFont (attrs, tryEverything);
   }

   inline bool fontExists (const char *name)
   {
      return platform->fontExists (name);
   }

   inline style::Color *createColor (int color)
   {
      return platform->createColor (color);
   }

   inline style::Tooltip *createTooltip (const char *text)
   {
      return platform->createTooltip (text);
   }

   inline void cancelTooltip ()
   {
      return platform->cancelTooltip ();
   }

   inline Imgbuf *createImgbuf (Imgbuf::Type type, int width, int height,
                                double gamma)
   {
      return platform->createImgbuf (type, width, height, gamma);
   }

   inline void copySelection(const char *text)
   {
      platform->copySelection(text);
   }

   inline ui::ResourceFactory *getResourceFactory ()
   {
      return platform->getResourceFactory ();
   }

   inline void connect (Receiver *receiver) {
      emitter.connectLayout (receiver); }

   /** \brief See dw::core::FindtextState::search. */
   inline FindtextState::Result search (const char *str, bool caseSens,
                                        int backwards)
      { return findtextState.search (str, caseSens, backwards); }

   /** \brief See dw::core::FindtextState::resetSearch. */
   inline void resetSearch () { findtextState.resetSearch (); }

   void setBgColor (style::Color *color);
   void setBgImage (style::StyleImage *bgImage,
                    style::BackgroundRepeat bgRepeat,
                    style::BackgroundAttachment bgAttachment,
                    style::Length bgPositionX, style::Length bgPositionY);

   inline style::Color* getBgColor () { return bgColor; }
   inline style::StyleImage* getBgImage () { return bgImage; }
};

} // namespace core
} // namespace dw

#endif // __DW_LAYOUT_HH__
