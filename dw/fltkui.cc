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



#include "fltkcore.hh"
#include "../lout/msg.h"
#include "../lout/misc.hh"

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Browser.H>

#include <stdio.h>

//----------------------------------------------------------------------------
/*
 * Local sub classes
 */

/*
 * Used to enable CTRL+{a,e,d,k} in form inputs (for start,end,del,cut)
 */
class CustInput2 : public Fl_Input {
public:
   CustInput2 (int x, int y, int w, int h, const char* l=0) :
      Fl_Input(x,y,w,h,l) {};
   int handle(int e);
};

int CustInput2::handle(int e)
{
   int k = Fl::event_key();

   _MSG("CustInput2::handle event=%d\n", e);

   // We're only interested in some flags
   unsigned modifier = Fl::event_state() & (FL_SHIFT | FL_CTRL | FL_ALT);

   if (e == FL_KEYBOARD) {
      if (k == FL_Page_Down || k == FL_Page_Up || k == FL_Up || k == FL_Down) {
         // Let them through for key commands and viewport motion.
         return 0;
      }
      if (modifier == FL_CTRL) {
         if (k == 'a' || k == 'e') {
            position(k == 'a' ? 0 : size());
            return 1;
         } else if (k == 'k') {
            cut(position(), size());
            return 1;
         } else if (k == 'd') {
            cut(position(), position()+1);
            return 1;
         } else if (k == 'h' || k == 'i' || k == 'j' || k == 'l' || k == 'm') {
            // Fl_Input wants to use ^H as backspace, and also "insert a few
            // selected control characters literally", but this gets in the way
            // of key commands.
            return 0;
         }
      }
   }
   return Fl_Input::handle(e);
}


/*
 * Used to handle some keystrokes as shortcuts to option menuitems
 * (i.e. jump to the next menuitem whose label starts with the pressed key)
 */
class CustChoice : public Fl_Choice {
public:
   CustChoice (int x, int y, int w, int h, const char* l=0) :
      Fl_Choice(x,y,w,h,l) {};
   int handle(int e);
};

int CustChoice::handle(int e)
{
   int k = Fl::event_key();
   unsigned modifier = Fl::event_state() & (FL_SHIFT|FL_CTRL|FL_ALT|FL_META);

   _MSG("CustChoice::handle %p e=%d active=%d focus=%d\n",
       this, e, active(), (Fl::focus() == this));
   if (Fl::focus() != this) {
      ; // Not Focused, let FLTK handle it
   } else if (e == FL_KEYDOWN && modifier == 0) {
      if (k == FL_Enter || k == FL_Down) {
         return Fl_Choice::handle(FL_PUSH); // activate menu

      } else if (isalnum(k)) { // try key as shortcut to menuitem
         int t = value()+1 >= size() ? 0 : value()+1;
         while (t != value()) {
             const Fl_Menu_Item *mi = &(menu()[t]);
             if (mi->submenu()) // submenu?
                ;
             else if (mi->label() && mi->active()) { // menu item?
                if (k == tolower(mi->label()[0])) {
                   value(mi);
                   return 1; // Let FLTK know we used this key
                }
             }
             if (++t == size())
                t = 0;
         }
      }
   }

   return Fl_Choice::handle(e);
}

//----------------------------------------------------------------------------

namespace dw {
namespace fltk {
namespace ui {

enum { RELIEF_X_THICKNESS = 3, RELIEF_Y_THICKNESS = 3 };

using namespace lout::object;
using namespace lout::container::typed;

FltkResource::FltkResource (FltkPlatform *platform)
{
   DBG_OBJ_CREATE ("dw::fltk::ui::FltkResource");

   this->platform = platform;

   allocation.x = 0;
   allocation.y = 0;
   allocation.width = 1;
   allocation.ascent = 1;
   allocation.descent = 0;

   style = NULL;

   enabled = true;
}

/**
 * This is not a constructor, since it calls some virtual methods, which
 * should not be done in a C++ base constructor.
 */
void FltkResource::init (FltkPlatform *platform)
{
   view = NULL;
   widget = NULL;
   platform->attachResource (this);
}

FltkResource::~FltkResource ()
{
   platform->detachResource (this);
   if (widget) {
      if (view) {
         view->removeFltkWidget(widget);
      }
      delete widget;
   }
   if (style)
      style->unref ();

   DBG_OBJ_DELETE ();
}

void FltkResource::attachView (FltkView *view)
{
   if (this->view)
      MSG_ERR("FltkResource::attachView: multiple views!\n");

   if (view->usesFltkWidgets ()) {
      this->view = view;

      widget = createNewWidget (&allocation);
      view->addFltkWidget (widget, &allocation);
      if (style)
         setWidgetStyle (widget, style);
      if (! enabled)
         widget->deactivate ();
   }
}

void FltkResource::detachView (FltkView *view)
{
   if (this->view != view)
      MSG_ERR("FltkResource::detachView: this->view: %p view: %p\n",
              this->view, view);
   this->view = NULL;
}

void FltkResource::sizeAllocate (core::Allocation *allocation)
{
   DBG_OBJ_ENTER ("resize", 0, "sizeAllocate", "%d, %d; %d * (%d + %d)",
                  allocation->x, allocation->y, allocation->width,
                  allocation->ascent, allocation->descent);

   this->allocation = *allocation;
   view->allocateFltkWidget (widget, allocation);

   DBG_OBJ_LEAVE ();
}

void FltkResource::draw (core::View *view, core::Rectangle *area)
{
   FltkView *fltkView = (FltkView*)view;
   if (fltkView->usesFltkWidgets () && this->view == fltkView) {
      fltkView->drawFltkWidget (widget, area);
   }
}

void FltkResource::setStyle (core::style::Style *style)
{
   if (this->style)
      this->style->unref ();

   this->style = style;
   style->ref ();

   setWidgetStyle (widget, style);
}

void FltkResource::setWidgetStyle (Fl_Widget *widget,
                                   core::style::Style *style)
{
   FltkFont *font = (FltkFont*)style->font;
   widget->labelsize (font->size);
   widget->labelfont (font->font);

   FltkColor *bg = (FltkColor*)style->backgroundColor;
   if (bg) {
      int normal_bg = bg->colors[FltkColor::SHADING_NORMAL];

      if (style->color) {
         int style_fg = ((FltkColor*)style->color)->colors
                                                   [FltkColor::SHADING_NORMAL];
         Fl_Color fg = fl_contrast(style_fg, normal_bg);

         widget->labelcolor(fg);
         widget->selection_color(fg);
      }

      widget->color(normal_bg);
   }
}

void FltkResource::setDisplayed(bool displayed)
{
   if (displayed)
      widget->show();
   else
      widget->hide();
}

bool FltkResource::displayed()
{
   bool ret = false;

   if (widget) {
      // visible() is not the same thing as being show()n exactly, but
      // show()/hide() set it appropriately for our purposes.
      ret = widget->visible();
   }
   return ret;
}

bool FltkResource::isEnabled ()
{
   return enabled;
}

void FltkResource::setEnabled (bool enabled)
{
   this->enabled = enabled;

   if (enabled)
      widget->activate ();
   else
      widget->deactivate ();
}

// ----------------------------------------------------------------------

template <class I> FltkSpecificResource<I>::FltkSpecificResource (FltkPlatform
                                                                  *platform) :
   FltkResource (platform)
{
   DBG_OBJ_CREATE ("dw::fltk::ui::FltkSpecificResource<>");
   DBG_OBJ_BASECLASS (I);
   DBG_OBJ_BASECLASS (FltkResource);
}

template <class I> FltkSpecificResource<I>::~FltkSpecificResource ()
{
   DBG_OBJ_DELETE ();
}

template <class I> void FltkSpecificResource<I>::sizeAllocate (core::Allocation
                                                               *allocation)
{
   FltkResource::sizeAllocate (allocation);
}

template <class I> void FltkSpecificResource<I>::draw (core::View *view,
                                                       core::Rectangle *area)
{
   FltkResource::draw (view, area);
}

template <class I> void FltkSpecificResource<I>::setStyle (core::style::Style
                                                           *style)
{
   FltkResource::setStyle (style);
}

template <class I> bool FltkSpecificResource<I>::isEnabled ()
{
   return FltkResource::isEnabled ();
}

template <class I> void FltkSpecificResource<I>::setEnabled (bool enabled)
{
   FltkResource::setEnabled (enabled);
}

// ----------------------------------------------------------------------

class EnterButton : public Fl_Button {
public:
   EnterButton (int x,int y,int w,int h, const char* label = 0) :
      Fl_Button (x,y,w,h,label) {};
   int handle(int e);
};

int EnterButton::handle(int e)
{
   if (e == FL_KEYBOARD && Fl::focus() == this && Fl::event_key() == FL_Enter){
      set_changed();
      simulate_key_action();
      do_callback();
      return 1;
   }
   return Fl_Button::handle(e);
}

FltkLabelButtonResource::FltkLabelButtonResource (FltkPlatform *platform,
                                                  const char *label):
   FltkSpecificResource <dw::core::ui::LabelButtonResource> (platform)
{
   this->label = strdup (label);
   init (platform);
}

FltkLabelButtonResource::~FltkLabelButtonResource ()
{
   free((char *)label);
}

Fl_Widget *FltkLabelButtonResource::createNewWidget (core::Allocation
                                                     *allocation)
{
   Fl_Button *button =
        new EnterButton (allocation->x, allocation->y, allocation->width,
                         allocation->ascent + allocation->descent, label);
   button->callback (widgetCallback, this);
   button->when (FL_WHEN_RELEASE);
   return button;
}

void FltkLabelButtonResource::sizeRequest (core::Requisition *requisition)
{
   DBG_OBJ_ENTER0 ("resize", 0, "sizeRequest");

   if (style) {
      FltkFont *font = (FltkFont*)style->font;
      fl_font(font->font,font->size);
      requisition->width =
         (int)fl_width (label, strlen (label))
         + 2 * RELIEF_X_THICKNESS;
      requisition->ascent = font->ascent + RELIEF_Y_THICKNESS;
      requisition->descent = font->descent + RELIEF_Y_THICKNESS;
   } else {
      requisition->width = 1;
      requisition->ascent = 1;
      requisition->descent = 0;
   }

   DBG_OBJ_MSGF ("resize", 1, "result: %d * (%d + %d)",
                 requisition->width, requisition->ascent, requisition->descent);
   DBG_OBJ_LEAVE ();
}

/*
 * Get FLTK state and translate to dw
 *
 * TODO: find a good home for this and the fltkviewbase.cc original.
 */
static core::ButtonState getDwButtonState ()
{
   int s1 = Fl::event_state ();
   int s2 = (core::ButtonState)0;

   if (s1 & FL_SHIFT)   s2 |= core::SHIFT_MASK;
   if (s1 & FL_CTRL)    s2 |= core::CONTROL_MASK;
   if (s1 & FL_ALT)     s2 |= core::META_MASK;
   if (s1 & FL_BUTTON1) s2 |= core::BUTTON1_MASK;
   if (s1 & FL_BUTTON2) s2 |= core::BUTTON2_MASK;
   if (s1 & FL_BUTTON3) s2 |= core::BUTTON3_MASK;

   return (core::ButtonState)s2;
}

static void setButtonEvent(dw::core::EventButton *event)
{
   event->xCanvas = Fl::event_x();
   event->yCanvas = Fl::event_y();
   event->state = getDwButtonState();
   event->button = Fl::event_button();
   event->numPressed = Fl::event_clicks() + 1;
}

void FltkLabelButtonResource::widgetCallback (Fl_Widget *widget,
                                              void *data)
{
   if (!Fl::event_button3()) {
      FltkLabelButtonResource *lbr = (FltkLabelButtonResource*) data;
      dw::core::EventButton event;
      setButtonEvent(&event);
      lbr->emitClicked(&event);
   }
}

const char *FltkLabelButtonResource::getLabel ()
{
   return label;
}


void FltkLabelButtonResource::setLabel (const char *label)
{
   free((char *)this->label);
   this->label = strdup (label);

   widget->label (this->label);
   queueResize (true);
}

// ----------------------------------------------------------------------

FltkEntryResource::FltkEntryResource (FltkPlatform *platform, int size,
                                      bool password, const char *label):
   FltkSpecificResource <dw::core::ui::EntryResource> (platform)
{
   this->size = size;
   this->password = password;
   this->label = label ? strdup(label) : NULL;
   this->label_w = 0;

   initText = NULL;
   editable = false;

   init (platform);
}

FltkEntryResource::~FltkEntryResource ()
{
   if (initText)
      free((char *)initText);
   if (label)
      free(label);
}

Fl_Widget *FltkEntryResource::createNewWidget (core::Allocation
                                                    *allocation)
{
   Fl_Input *input =
        new CustInput2(allocation->x, allocation->y, allocation->width,
                      allocation->ascent + allocation->descent);
   if (password)
      input->type(FL_SECRET_INPUT);
   input->callback (widgetCallback, this);
   input->when (FL_WHEN_ENTER_KEY_ALWAYS);

   if (label) {
      input->label(label);
      input->align(FL_ALIGN_LEFT);
   }
   if (initText)
      input->value (initText);

   return input;
}

void FltkEntryResource::setWidgetStyle (Fl_Widget *widget,
                                        core::style::Style *style)
{
   Fl_Input *in = (Fl_Input *)widget;

   FltkResource::setWidgetStyle(widget, style);

   in->textcolor(widget->labelcolor());
   in->cursor_color(in->textcolor());
   in->textsize(in->labelsize());
   in->textfont(in->labelfont());

   if (label) {
      int h;
      label_w = 0;
      widget->measure_label(label_w, h);
      label_w += RELIEF_X_THICKNESS;
   }
}

void FltkEntryResource::setDisplayed(bool displayed)
{
   FltkResource::setDisplayed(displayed);
   queueResize(true);
}

void FltkEntryResource::sizeRequest (core::Requisition *requisition)
{
   DBG_OBJ_ENTER0 ("resize", 0, "sizeRequest");

   if (displayed() && style) {
      FltkFont *font = (FltkFont*)style->font;
      fl_font(font->font,font->size);
      // WORKAROUND: A bug with fl_width(uint_t) on non-xft X was present in
      // 1.3.0 (STR #2688).
      requisition->width =
         (int)fl_width ("n")
         * (size == UNLIMITED_SIZE ? 10 : size)
         + label_w + (2 * RELIEF_X_THICKNESS);
      requisition->ascent = font->ascent + RELIEF_Y_THICKNESS;
      requisition->descent = font->descent + RELIEF_Y_THICKNESS;
   } else {
      requisition->width = 0;
      requisition->ascent = 0;
      requisition->descent = 0;
   }

   DBG_OBJ_MSGF ("resize", 1, "result: %d * (%d + %d)",
                 requisition->width, requisition->ascent, requisition->descent);
   DBG_OBJ_LEAVE ();
}

void FltkEntryResource::sizeAllocate (core::Allocation *allocation)
{
   if (!label) {
      FltkResource::sizeAllocate(allocation);
   } else {
      DBG_OBJ_MSGF ("resize", 0,
                    "<b>sizeAllocate</b> (%d, %d; %d * (%d + %d))",
                    allocation->x, allocation->y, allocation->width,
                    allocation->ascent, allocation->descent);

      this->allocation = *allocation;

      /* push the Fl_Input over to the right of the label */
      core::Allocation a = this->allocation;
      a.x += this->label_w;
      a.width -= this->label_w;
      view->allocateFltkWidget (widget, &a);
   }
}

void FltkEntryResource::widgetCallback (Fl_Widget *widget, void *data)
{
   ((FltkEntryResource*)data)->emitActivate ();
}

const char *FltkEntryResource::getText ()
{
   return ((Fl_Input*)widget)->value ();
}

void FltkEntryResource::setText (const char *text)
{
   if (initText)
      free((char *)initText);
   initText = strdup (text);

   ((Fl_Input*)widget)->value (initText);
}

bool FltkEntryResource::isEditable ()
{
   return editable;
}

void FltkEntryResource::setEditable (bool editable)
{
   this->editable = editable;
}

void FltkEntryResource::setMaxLength (int maxlen)
{
   ((Fl_Input *)widget)->maximum_size(maxlen);
}

// ----------------------------------------------------------------------

static int kf_backspace_word (int c, Fl_Text_Editor *e)
{
   int p1, p2 = e->insert_position();

   e->previous_word();
   p1 = e->insert_position();
   e->buffer()->remove(p1, p2);
   e->show_insert_position();
   e->set_changed();
   if (e->when() & FL_WHEN_CHANGED)
      e->do_callback();
   return 0;
}

FltkMultiLineTextResource::FltkMultiLineTextResource (FltkPlatform *platform,
                                                      int cols, int rows):
   FltkSpecificResource <dw::core::ui::MultiLineTextResource> (platform)
{
   buffer = new Fl_Text_Buffer;
   text_copy = NULL;
   editable = false;

   numCols = cols;
   numRows = rows;

   // Check values. Upper bound check is left to the caller.
   if (numCols < 1) {
      MSG_WARN("numCols = %d is set to 1.\n", numCols);
      numCols = 1;
   }
   if (numRows < 1) {
      MSG_WARN("numRows = %d is set to 1.\n", numRows);
      numRows = 1;
   }

   init (platform);
}

FltkMultiLineTextResource::~FltkMultiLineTextResource ()
{
   /* Free memory avoiding a double-free of text buffers */
   ((Fl_Text_Editor *) widget)->buffer (0);
   delete buffer;
   if (text_copy)
      free(text_copy);
}

Fl_Widget *FltkMultiLineTextResource::createNewWidget (core::Allocation
                                                            *allocation)
{
   Fl_Text_Editor *text =
      new Fl_Text_Editor (allocation->x, allocation->y, allocation->width,
                          allocation->ascent + allocation->descent);
   text->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
   text->buffer (buffer);
   text->remove_key_binding(FL_BackSpace, FL_TEXT_EDITOR_ANY_STATE);
   text->add_key_binding(FL_BackSpace, 0, Fl_Text_Editor::kf_backspace);
   text->add_key_binding(FL_BackSpace, FL_CTRL, kf_backspace_word);
   return text;
}

void FltkMultiLineTextResource::setWidgetStyle (Fl_Widget *widget,
                                                core::style::Style *style)
{
   Fl_Text_Editor *ed = (Fl_Text_Editor *)widget;

   FltkResource::setWidgetStyle(widget, style);

   ed->textcolor(widget->labelcolor());
   ed->cursor_color(ed->textcolor());
   ed->textsize(ed->labelsize());
   ed->textfont(ed->labelfont());
}

void FltkMultiLineTextResource::sizeRequest (core::Requisition *requisition)
{
   DBG_OBJ_ENTER0 ("resize", 0, "sizeRequest");

   if (style) {
      FltkFont *font = (FltkFont*)style->font;
      fl_font(font->font,font->size);
      // WORKAROUND: A bug with fl_width(uint_t) on non-xft X was present in
      // 1.3.0 (STR #2688).
      requisition->width =
         (int)fl_width ("n") * numCols + 2 * RELIEF_X_THICKNESS;
      requisition->ascent =
         RELIEF_Y_THICKNESS + font->ascent +
         (font->ascent + font->descent) * (numRows - 1);
      requisition->descent =
         font->descent +
         RELIEF_Y_THICKNESS;
   } else {
      requisition->width = 1;
      requisition->ascent = 1;
      requisition->descent = 0;
   }

   DBG_OBJ_MSGF ("resize", 1, "result: %d * (%d + %d)",
                 requisition->width, requisition->ascent, requisition->descent);
   DBG_OBJ_LEAVE ();
}

const char *FltkMultiLineTextResource::getText ()
{
   /* FLTK-1.3 insists upon returning a new copy of the buffer text, so
    * we have to keep track of it.
    */
   if (text_copy)
      free(text_copy);
   text_copy = buffer->text();
   return text_copy;
}

void FltkMultiLineTextResource::setText (const char *text)
{
   buffer->text (text);
}

bool FltkMultiLineTextResource::isEditable ()
{
   return editable;
}

void FltkMultiLineTextResource::setEditable (bool editable)
{
   this->editable = editable;
}

// ----------------------------------------------------------------------

template <class I>
FltkToggleButtonResource<I>::FltkToggleButtonResource (FltkPlatform *platform,
                                                       bool activated):
   FltkSpecificResource <I> (platform)
{
   initActivated = activated;
}


template <class I>
FltkToggleButtonResource<I>::~FltkToggleButtonResource ()
{
}


template <class I>
Fl_Widget *FltkToggleButtonResource<I>::createNewWidget (core::Allocation
                                                              *allocation)
{
   Fl_Button *button = createNewButton (allocation);
   button->value (initActivated);
   return button;
}

template <class I>
void FltkToggleButtonResource<I>::setWidgetStyle (Fl_Widget *widget,
                                                  core::style::Style *style)
{
   FltkResource::setWidgetStyle(widget, style);

   widget->selection_color(FL_BLACK);
}


template <class I>
void FltkToggleButtonResource<I>::sizeRequest (core::Requisition *requisition)
{
   DBG_OBJ_ENTER0 ("resize", 0, "sizeRequest");

   FltkFont *font = (FltkFont *)
      (this->FltkResource::style ? this->FltkResource::style->font : NULL);

   if (font) {
      fl_font(font->font, font->size);
      requisition->width = font->ascent + font->descent + 2*RELIEF_X_THICKNESS;
      requisition->ascent = font->ascent + RELIEF_Y_THICKNESS;
      requisition->descent = font->descent + RELIEF_Y_THICKNESS;
   } else {
      requisition->width = 1;
      requisition->ascent = 1;
      requisition->descent = 0;
   }

   DBG_OBJ_MSGF ("resize", 1, "result: %d * (%d + %d)",
                 requisition->width, requisition->ascent, requisition->descent);
   DBG_OBJ_LEAVE ();
}


template <class I>
bool FltkToggleButtonResource<I>::isActivated ()
{
   return ((Fl_Button*)this->widget)->value ();
}


template <class I>
void FltkToggleButtonResource<I>::setActivated (bool activated)
{
   initActivated = activated;
   ((Fl_Button*)this->widget)->value (initActivated);
}

// ----------------------------------------------------------------------

FltkCheckButtonResource::FltkCheckButtonResource (FltkPlatform *platform,
                                                  bool activated):
   FltkToggleButtonResource<dw::core::ui::CheckButtonResource> (platform,
                                                                activated)
{
   init (platform);
}


FltkCheckButtonResource::~FltkCheckButtonResource ()
{
}


Fl_Button *FltkCheckButtonResource::createNewButton (core::Allocation
                                                          *allocation)
{
   Fl_Check_Button *cb =
      new Fl_Check_Button (allocation->x, allocation->y, allocation->width,
                           allocation->ascent + allocation->descent);
   return cb;
}

// ----------------------------------------------------------------------

bool FltkRadioButtonResource::Group::FltkGroupIterator::hasNext ()
{
   return it.hasNext ();
}

dw::core::ui::RadioButtonResource
*FltkRadioButtonResource::Group::FltkGroupIterator::getNext ()
{
   return (dw::core::ui::RadioButtonResource*)it.getNext ();
}

void FltkRadioButtonResource::Group::FltkGroupIterator::unref ()
{
   delete this;
}


FltkRadioButtonResource::Group::Group (FltkRadioButtonResource
                                       *radioButtonResource)
{
   list = new lout::container::typed::List <FltkRadioButtonResource> (false);
   connect (radioButtonResource);
}

FltkRadioButtonResource::Group::~Group ()
{
   delete list;
}

void FltkRadioButtonResource::Group::connect (FltkRadioButtonResource
                                              *radioButtonResource)
{
   list->append (radioButtonResource);
}

void FltkRadioButtonResource::Group::unconnect (FltkRadioButtonResource
                                                *radioButtonResource)
{
   list->removeRef (radioButtonResource);
   if (list->isEmpty ())
      delete this;
}


FltkRadioButtonResource::FltkRadioButtonResource (FltkPlatform *platform,
                                                  FltkRadioButtonResource
                                                  *groupedWith,
                                                  bool activated):
   FltkToggleButtonResource<dw::core::ui::RadioButtonResource> (platform,
                                                                activated)
{
   init (platform);

   if (groupedWith) {
      group = groupedWith->group;
      group->connect (this);
   } else
      group = new Group (this);
}


FltkRadioButtonResource::~FltkRadioButtonResource ()
{
   group->unconnect (this);
}

dw::core::ui::RadioButtonResource::GroupIterator
*FltkRadioButtonResource::groupIterator ()
{
   return group->groupIterator ();
}

void FltkRadioButtonResource::widgetCallback (Fl_Widget *widget,
                                              void *data)
{
   if (widget->when () & FL_WHEN_CHANGED)
      ((FltkRadioButtonResource*)data)->buttonClicked ();
}

void FltkRadioButtonResource::buttonClicked ()
{
   for (Iterator <FltkRadioButtonResource> it = group->iterator ();
        it.hasNext (); ) {
      FltkRadioButtonResource *other = it.getNext ();
      other->setActivated (other == this);
   }
}

Fl_Button *FltkRadioButtonResource::createNewButton (core::Allocation
                                                     *allocation)
{
   /*
    * Groups of Fl_Radio_Button must be added to one Fl_Group, which is
    * not possible in this context. For this, we do the grouping ourself,
    * based on FltkRadioButtonResource::Group.
    *
    * What we actually need for this, is a widget, which behaves like a
    * check button, but looks like a radio button. The first depends on the
    * type, the second on the style. Since the type is simpler to change
    * than the style, we create a radio button, and then change the type
    * (instead of creating a check button, and changing the style).
    */

   Fl_Button *button =
      new Fl_Round_Button (allocation->x, allocation->y, allocation->width,
                           allocation->ascent + allocation->descent);
   button->when (FL_WHEN_CHANGED);
   button->callback (widgetCallback, this);
   button->type (FL_TOGGLE_BUTTON);

   return button;
}

// ----------------------------------------------------------------------

template <class I> dw::core::Iterator *
FltkSelectionResource<I>::iterator (dw::core::Content::Type mask, bool atEnd)
{
   /** \bug Implementation. */
   return new core::EmptyIterator (this->getEmbed (), mask, atEnd);
}

// ----------------------------------------------------------------------

FltkOptionMenuResource::FltkOptionMenuResource (FltkPlatform *platform):
   FltkSelectionResource <dw::core::ui::OptionMenuResource> (platform)
{
   /* Fl_Menu_ does not like multiple menu items with the same label, and
    * insert() treats some characters specially unless escaped, so let's
    * do our own menu handling.
    */
   itemsAllocated = 0x10;
   menu = new Fl_Menu_Item[itemsAllocated];
   memset(menu, 0, itemsAllocated * sizeof(Fl_Menu_Item));
   itemsUsed = 1; // menu[0].text == NULL, which is an end-of-menu marker.

   init (platform);
}

FltkOptionMenuResource::~FltkOptionMenuResource ()
{
   for (int i = 0; i < itemsUsed; i++) {
      if (menu[i].text)
         free((char *) menu[i].text);
   }
   delete[] menu;
}

void FltkOptionMenuResource::setWidgetStyle (Fl_Widget *widget,
                                             core::style::Style *style)
{
   Fl_Choice *ch = (Fl_Choice *)widget;

   FltkResource::setWidgetStyle(widget, style);

   ch->textcolor(widget->labelcolor());
   ch->textfont(ch->labelfont());
   ch->textsize(ch->labelsize());
}

Fl_Widget *FltkOptionMenuResource::createNewWidget (core::Allocation
                                                     *allocation)
{
   Fl_Choice *choice =
      new CustChoice (allocation->x, allocation->y,
                      allocation->width,
                      allocation->ascent + allocation->descent);
   choice->menu(menu);
   return choice;
}

void FltkOptionMenuResource::widgetCallback (Fl_Widget *widget,
                                             void *data)
{
}

int FltkOptionMenuResource::getMaxItemWidth()
{
   int i, max = 0;

   for (i = 0; i < itemsUsed; i++) {
      int width = 0;
      const char *str = menu[i].text;

      if (str) {
         width = fl_width(str);
         if (width > max)
            max = width;
      }
   }
   return max;
}

void FltkOptionMenuResource::sizeRequest (core::Requisition *requisition)
{
   DBG_OBJ_ENTER0 ("resize", 0, "sizeRequest");

   if (style) {
      FltkFont *font = (FltkFont*)style->font;
      fl_font(font->font, font->size);
      int maxItemWidth = getMaxItemWidth ();
      requisition->ascent = font->ascent + RELIEF_Y_THICKNESS;
      requisition->descent = font->descent + RELIEF_Y_THICKNESS;
      requisition->width = maxItemWidth
         + (requisition->ascent + requisition->descent)
         + 2 * RELIEF_X_THICKNESS;
   } else {
      requisition->width = 1;
      requisition->ascent = 1;
      requisition->descent = 0;
   }

   DBG_OBJ_MSGF ("resize", 1, "result: %d * (%d + %d)",
                 requisition->width, requisition->ascent, requisition->descent);
   DBG_OBJ_LEAVE ();
}

void FltkOptionMenuResource::enlargeMenu ()
{
   Fl_Choice *ch = (Fl_Choice *)widget;
   int selected = ch->value();
   Fl_Menu_Item *newMenu;

   itemsAllocated += 0x10;
   newMenu = new Fl_Menu_Item[itemsAllocated];
   memcpy(newMenu, menu, itemsUsed * sizeof(Fl_Menu_Item));
   memset(newMenu + itemsUsed, 0, 0x10 * sizeof(Fl_Menu_Item));
   delete[] menu;
   menu = newMenu;
   ch->menu(menu);
   ch->value(selected);
}

Fl_Menu_Item *FltkOptionMenuResource::newItem()
{
   Fl_Menu_Item *item;

   if (itemsUsed == itemsAllocated)
      enlargeMenu();

   item = menu + itemsUsed - 1;
   itemsUsed++;

   return item;
}

void FltkOptionMenuResource::addItem (const char *str,
                                      bool enabled, bool selected)
{
   Fl_Menu_Item *item = newItem();

   item->text = strdup(str);

   if (enabled == false)
      item->flags = FL_MENU_INACTIVE;

   if (selected)
      ((Fl_Choice *)widget)->value(item);

   queueResize (true);
}

void FltkOptionMenuResource::setItem (int index, bool selected)
{
   if (selected)
      ((Fl_Choice *)widget)->value(menu+index);
}

void FltkOptionMenuResource::pushGroup (const char *name, bool enabled)
{
   Fl_Menu_Item *item = newItem();

   item->text = strdup(name);

   if (enabled == false)
      item->flags = FL_MENU_INACTIVE;

   item->flags |= FL_SUBMENU;

   queueResize (true);
}

void FltkOptionMenuResource::popGroup ()
{
   /* Item with NULL text field closes the submenu */
   newItem();
   queueResize (true);
}

bool FltkOptionMenuResource::isSelected (int index)
{
   return index == ((Fl_Choice *)widget)->value();
}

int FltkOptionMenuResource::getNumberOfItems()
{
   return ((Fl_Choice*)widget)->size();
}

// ----------------------------------------------------------------------

class CustBrowser : public Fl_Browser {
public:
   CustBrowser(int x, int y, int w, int h) : Fl_Browser(x, y, w, h) {};
   int full_width() const;
   int full_height() const {return Fl_Browser::full_height();}
   int avg_height() {return size() ? Fl_Browser_::incr_height() : 0;}
};

/*
 * Fl_Browser_ has a full_width(), but it has a tendency to contain 0, so...
 */
int CustBrowser::full_width() const
{
   int max = 0;
   void *item = item_first();

   while (item) {
      int w = item_width(item);

      if (w > max)
         max = w;

      item = item_next(item);
   }
   return max;
}

FltkListResource::FltkListResource (FltkPlatform *platform,
                                    core::ui::ListResource::SelectionMode
                                    selectionMode, int rowCount):
   FltkSelectionResource <dw::core::ui::ListResource> (platform),
   currDepth(0)
{
   mode = selectionMode;
   showRows = rowCount;
   init (platform);
}

FltkListResource::~FltkListResource ()
{
}


Fl_Widget *FltkListResource::createNewWidget (core::Allocation *allocation)
{
   CustBrowser *b =
      new CustBrowser (allocation->x, allocation->y, allocation->width,
                      allocation->ascent + allocation->descent);

   b->type((mode == SELECTION_MULTIPLE) ? FL_MULTI_BROWSER : FL_HOLD_BROWSER);
   b->callback(widgetCallback, this);
   b->when(FL_WHEN_CHANGED);
   b->column_widths(colWidths);
   b->column_char('\a');   // I just chose a nonprinting character.

   return b;
}

void FltkListResource::setWidgetStyle (Fl_Widget *widget,
                                       core::style::Style *style)
{
   Fl_Browser *b = (Fl_Browser *)widget;

   FltkResource::setWidgetStyle(widget, style);

   b->textfont(widget->labelfont());
   b->textsize(widget->labelsize());
   b->textcolor(widget->labelcolor());

   colWidths[0] = b->textsize();
   colWidths[1] = colWidths[0];
   colWidths[2] = colWidths[0];
   colWidths[3] = 0;
}

void FltkListResource::widgetCallback (Fl_Widget *widget, void *data)
{
   Fl_Browser *b = (Fl_Browser *) widget;

   if (b->selected(b->value())) {
      /* If it shouldn't be selectable, deselect it again. It would be nice to
       * have a less unpleasant way to do this.
       */
      const char *inactive_code;
      if ((inactive_code = strstr(b->text(b->value()), "@N"))) {
         const char *ignore_codes = strstr(b->text(b->value()), "@.");

         if (inactive_code < ignore_codes)
            b->select(b->value(), 0);
      }
   }
}

void *FltkListResource::newItem (const char *str, bool enabled, bool selected)
{
   Fl_Browser *b = (Fl_Browser *) widget;
   int index = b->size() + 1;
   char *label = (char *)malloc(strlen(str) + 1 + currDepth + 4),
        *s = label;

   memset(s, '\a', currDepth);
   s += currDepth;
   if (!enabled) {
      // FL_INACTIVE_COLOR
      *s++ = '@';
      *s++ = 'N';
   }
   // ignore further '@' chars
   *s++ = '@';
   *s++ = '.';

   strcpy(s, str);

   b->add(label);
   free(label);

   if (selected) {
      b->select(index, selected);
      if (b->type() == FL_HOLD_BROWSER) {
         /* Left to its own devices, it sometimes has some suboptimal ideas
          * about how to scroll, and sometimes doesn't seem to show everything
          * where it thinks it is.
          */
         if (index > showRows) {
            /* bottomline() and middleline() don't work because the widget is
             * too tiny at this point for the bbox() call in
             * Fl_Browser::lineposition() to do what one would want.
             */
            b->topline(index - showRows + 1);
         } else {
            b->topline(1);
         }
      }
   }
   queueResize (true);
   return NULL;
}

void FltkListResource::addItem (const char *str, bool enabled, bool selected)
{
   // Fl_Browser_::incr_height() for item height won't do the right thing if
   // the first item doesn't have anything to it.
   if (!str || !*str)
      str = " ";
   newItem(str, enabled, selected);
}

void FltkListResource::setItem (int index, bool selected)
{
   Fl_Browser *b = (Fl_Browser *) widget;

   b->select(index + 1, selected);
}

void FltkListResource::pushGroup (const char *name, bool enabled)
{
   bool en = false;
   bool selected = false;

   // Fl_Browser_::incr_height() for item height won't do the right thing if
   // the first item doesn't have anything to it.
   if (!name || !*name)
      name = " ";

   // TODO: Proper disabling of item groups
   newItem(name, en, selected);

   if (currDepth < 3)
      currDepth++;
}

void FltkListResource::popGroup ()
{
   CustBrowser *b = (CustBrowser *) widget;

   newItem(" ", false, false);
   b->hide(b->size());

   if (currDepth)
      currDepth--;
}

int FltkListResource::getMaxItemWidth()
{
   return ((CustBrowser *) widget)->full_width();
}

void FltkListResource::sizeRequest (core::Requisition *requisition)
{
   DBG_OBJ_ENTER0 ("resize", 0, "sizeRequest");

   if (style) {
      CustBrowser *b = (CustBrowser *) widget;
      int height = b->full_height();
      requisition->width = getMaxItemWidth() + 4;

      if (showRows * b->avg_height() < height) {
         height = showRows * b->avg_height();
         b->has_scrollbar(Fl_Browser_::VERTICAL_ALWAYS);
         requisition->width += Fl::scrollbar_size();
      } else {
         b->has_scrollbar(0);
      }

      requisition->descent = style->font->descent + 2;
      requisition->ascent = height - style->font->descent + 2;
   } else {
      requisition->width = 1;
      requisition->ascent = 1;
      requisition->descent = 0;
   }

   DBG_OBJ_MSGF ("resize", 1, "result: %d * (%d + %d)",
                 requisition->width, requisition->ascent, requisition->descent);
   DBG_OBJ_LEAVE ();
}

int FltkListResource::getNumberOfItems()
{
   return ((Fl_Browser*)widget)->size();
}

bool FltkListResource::isSelected (int index)
{
   Fl_Browser *b = (Fl_Browser *) widget;

   return b->selected(index + 1) ? true : false;
}

} // namespace ui
} // namespace fltk
} // namespace dw

