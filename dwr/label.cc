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
 */

#include "label.hh"

using namespace dw::core;
using namespace dw::core::style;
using namespace lout::misc;

namespace rtfl {

namespace dw {

int Label::CLASS_ID = -1;

// ----------------------------------------------------------------------

Label::LabelIterator::LabelIterator (Label *label, Content::Type mask,
                                     bool atEnd) : Iterator (label, mask, atEnd)
{
   index = atEnd ? label->words->size() : -1;
   content.type = atEnd ? Content::END : Content::START;
}

Label::LabelIterator::LabelIterator (Label *label, Content::Type mask,
                                     int index) : Iterator (label, mask, false)
{
   this->index = index;

   if (index < 0)
      content.type = Content::START;
   else if (index >= label->words->size ())
      content.type = Content::END;
   else {
      content.type = Content::TEXT;
      content.text = label->words->getRef(index)->text;
   }
}

lout::object::Object *Label::LabelIterator::clone()
{
   return new LabelIterator ((Label*)getWidget(), getMask(), index);
}

int Label::LabelIterator::compareTo(lout::object::Comparable *other)
{
   return index - ((LabelIterator*)other)->index;
}

bool Label::LabelIterator::next ()
{
   Label *label = (Label*)getWidget();

   if (content.type == Content::END)
      return false;

   // labels only contain widgets:
   if ((getMask() & Content::TEXT) == 0) {
      content.type = Content::END;
      return false;
   }

   index++;
   if (index >= label->words->size ()) {
      content.type = Content::END;
      return false;
   } else {
      content.type = Content::TEXT;
      content.text = label->words->getRef(index)->text;
      return true;
   }
}

bool Label::LabelIterator::prev ()
{
   Label *label = (Label*)getWidget();

   if (content.type == Content::START)
      return false;

   // labels only contain widgets:
   if ((getMask() & Content::TEXT) == 0) {
      content.type = Content::START;
      return false;
   }

   index--;
   if (index < 0) {
      content.type = Content::START;
      return false;
   } else {
      content.type = Content::TEXT;
      content.text = label->words->getRef(index)->text;
      return true;
   }
}

void Label::LabelIterator::highlight (int start, int end, HighlightLayer layer)
{
   /** \bug Not implemented. */
}

void Label::LabelIterator::unhighlight (int direction, HighlightLayer layer)
{
   /** \bug Not implemented. */
}

void Label::LabelIterator::getAllocation (int start, int end,
                                          Allocation *allocation)
{
   /** \bug Not implemented. */
}

// ----------------------------------------------------------------------

Label::Label (const char *text, int link)
{
   DBG_OBJ_CREATE ("rtfl::dw::Label");
   registerName ("rtfl::dw::Label", &CLASS_ID);

   words = new SimpleVector<Word> (1);

   for (int i = 0; i < 4; i++)
      styles[i] = NULL;

   selected = buttonDown = false;
   this->link = link;

   setText (text);
}

Label::~Label ()
{
   clearWords ();
   delete words;

   clearStyles ();

   DBG_OBJ_DELETE ();
}

void Label::setText (const char *text)
{
   clearWords ();

   DBG_OBJ_SET_STR ("text", text);

   // Parse text for tags <i>, </i>, <b>, </b>. Very simple, no stack.
   const char *start = text;
   int styleIndex = 0;
   while (*start) {
      const char *end = start;

      while (!(*end == 0 ||
               strncmp (end, "<i>", 3) == 0 || strncmp (end, "<b>", 3) == 0 ||
               strncmp (end, "</i>", 4) == 0 || strncmp (end, "</b>", 4) == 0))
         end++;

      //printf ("start: '%s', end: '%s'\n", start, end);

      if (end > start) {
         words->increase ();
         Word *word = words->getLastRef ();
         word->text = strndup (start, end - start);
         word->styleIndex = styleIndex;

         //printf ("   new word '%s' with attributes %c%c\n", word->text,
         //        (word->styleIndex & ITALIC) ? 'i' : '-',
         //        (word->styleIndex & BOLD) ? 'b' : '-');              
      }

      if (*end == 0)
         start = end;
      else if (strncmp (end, "<i>", 3) == 0) {
         start = end + 3;
         styleIndex |= ITALIC;
      } else if (strncmp (end, "<b>", 3) == 0) {
         start = end + 3;
         styleIndex |= BOLD;
      } else if (strncmp (end, "</i>", 4) == 0) {
         start = end + 4;
         styleIndex &= ~ITALIC;
      } else if (strncmp (end, "</b>", 4) == 0) {
         start = end + 4;
         styleIndex &= ~BOLD;
      } else
         assertNotReached ();
   }

   queueResize (0, true);
}

void Label::clearWords ()
{
   for (int i = 0; i < words->size (); i++)
      free (words->getRef(i)->text);
   words->setSize (0);
}

void Label::clearStyles ()
{
   for (int i = 0; i < 4; i++)
      if (styles[i]) {
         styles[i]->unref ();
         styles[i] = NULL;
      }
}

void Label::ensureStyles ()
{
   if (getStyle () && styles[0] == NULL) {
      styles[0] = getStyle ();
      styles[0]->ref ();

      for (int i = 1; i < 4; i++) {
         StyleAttrs styleAttrs = *getStyle ();
         FontAttrs fontAttrs = *(styleAttrs.font);
         fontAttrs.weight = (i & BOLD) ? 700 : 400;
         fontAttrs.style = (i & ITALIC) ? FONT_STYLE_ITALIC : FONT_STYLE_NORMAL;
         styleAttrs.font = Font::create (layout, &fontAttrs);
         styles[i] = Style::create (&styleAttrs);
      }
   }
}

void Label::sizeRequestImplImpl (Requisition *requisition)
{
   totalSize.width = totalSize.ascent = totalSize.descent = 0;

   if (getStyle ()) {
      ensureStyles ();

      for (int i = 0; i < words->size (); i++) {
         Word *word = words->getRef(i);
         Font *font = styles[(int)word->styleIndex]->font;
            
         word->size.width =
            layout->textWidth (font, word->text, strlen (word->text));
         word->size.ascent = font->ascent;
         word->size.descent = font->descent;
            
         totalSize.width += word->size.width;
         totalSize.ascent = max (totalSize.ascent, word->size.ascent);
         totalSize.descent = max (totalSize.descent, word->size.descent);
      }
   } else
      totalSize.width = totalSize.ascent = totalSize.descent = 0;
      
   requisition->width = totalSize.width + getStyle()->boxDiffWidth ();
   requisition->ascent = totalSize.ascent + getStyle()->boxOffsetY ();
   requisition->descent = totalSize.descent + getStyle()->boxRestHeight ();
}

void Label::getExtremesImplImpl (Extremes *extremes)
{
   // Not used within RTFL.
   assertNotReached ();
}

void Label::drawImpl (View *view, Rectangle *area)
{
   DBG_OBJ_ENTER ("drawImpl", 0, "draw", "%d, %d, %d * %d",
                  area->x, area->y, area->width, area->height);

   drawWidgetBox (view, area, selected);
      
   if (getStyle ()) {
      ensureStyles ();

      // Could adhere to style::textAlign. This can be used for
      // centered text:
      //
      // int x = getAllocation()->x
      //    + (getAllocation()->width - totalSize.width) / 2;
      //
      // Now, only left-aligned text is supported.

      int x = getAllocation()->x + getStyle()->boxOffsetX ();

      // Same for style::valign:
      //
      // int baseY = getAllocation()->y
      //    + (getHeight () - (totalSize.ascent + totalSize.descent)) / 2
      //    + totalSize.ascent;
         
      int baseY =
         getAllocation()->y + totalSize.ascent + getStyle()->boxOffsetY ();
         
      for (int i = 0; i < words->size (); i++) {
         Word *word = words->getRef(i);
         view-> drawText (styles[(int)word->styleIndex]->font,
                          styles[(int)word->styleIndex]->color,
                          selected ? Color::SHADING_INVERSE
                          : Color::SHADING_NORMAL,
                          x, baseY, word->text, strlen (word->text));
         x += word->size.width;
      }
   }

   DBG_OBJ_LEAVE ();
}

void Label::leaveNotifyImpl (EventCrossing *event)
{
   buttonDown = false;
}

bool Label::buttonPressImpl (EventButton *event)
{
   if (link != -1 && event->button == 1) {
      buttonDown = true;
      return true;
   } else
      return false;
}

bool Label::buttonReleaseImpl (EventButton *event)
{
   if (link != -1 && event->button == 1) {
      if (buttonDown)
         linkEmitter.emitClick (this, link, -1, -1, -1, event);
      return true;
   } else
      return false;
}

Iterator *Label::iterator (Content::Type mask, bool atEnd)
{
   return new LabelIterator (this, mask, atEnd);
}

void Label::setStyle (Style *style)
{
   Widget::setStyle (style);
   clearStyles ();
}

void Label::select ()
{
   selected = true;
   queueDraw ();
}

void Label::unselect ()
{
   selected = false;
   queueDraw ();
}

} // namespace rtfl

} // namespace dw
