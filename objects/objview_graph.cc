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

#include "objview_graph.hh"

#include <fnmatch.h>
#include <unistd.h>

using namespace lout::object;
using namespace lout::container;
using namespace lout::container::typed;
using namespace lout::misc;
using namespace dw::core;
using namespace dw::core::style;
using namespace rtfl::dw;

namespace rtfl {

namespace objects {

void ObjViewGraphListener::close (ObjViewStacktraceWindow *window)
{
   graph->stacktraceWindows->removeRef (window);
}

// ----------------------------------------------------------------------

OVGAttributesList::OVGAttributesList (OVGAttributesList *parent)
{
   this->parent = parent;

   childNoCount = numChildrenShown = 0;
   childrenShown = new BitSet (8);
   shown = true;
   
   if (parent)
      childNo = parent->registerChild ();
   else
      childNo = -1;

   attributes = new HashTable<String, OVGAttribute> (true, true);
}

OVGAttributesList::~OVGAttributesList ()
{
   if (parent)
      parent->unregisterChild (childNo);

   delete childrenShown;

   delete attributes;
}

void OVGAttributesList::initWidgets (Style *widgetStyle, dw::Box *parent,
                                     bool showLarge)
{
   toggle = new Toggle (showLarge);
   toggle->setStyle (widgetStyle);
   parent->addChild (toggle);
     
   vbox = new VBox (false);
   vbox->setStyle (widgetStyle);
   toggle->setLarge (vbox);
}

int OVGAttributesList::registerChild ()
{
   int childNo = childNoCount++;

   //printf ("   %p (%p) / %d -- registerChild => %d\n",
   //        this, parent, this->childNo, childNo);

   childShown (childNo);
   return childNo;
}

void OVGAttributesList::unregisterChild (int childNo)
{
   // Should (could?) actually destroy this attribute (if this is an
   // attribute) when no registered children are left.
   childHidden (childNo);
}

void OVGAttributesList::childShown (int childNo)
{
   if (!childrenShown->get (childNo)) {
      childrenShown->set (childNo, true);
      numChildrenShown++;
      checkVisibility ();
   }
}

void OVGAttributesList::childHidden (int childNo)
{
   if (childrenShown->get (childNo)) {
      childrenShown->set (childNo, false);
      numChildrenShown--;
      checkVisibility ();
   }
}

void OVGAttributesList::checkVisibility ()
{
   //printf ("   %p (%p) / %d -- %d children shown\n",
   //        this, parent, childNo, numChildrenShown);

   if (shown) {
      if (numChildrenShown <= 0) {
         shown = false;
         hide ();
         if (parent)
            parent->childHidden (childNo);
      }
   } else {
      if (numChildrenShown > 0) {
         shown = true;
         show ();
         if (parent)
            parent->childShown (childNo);
      }
   }
}

// ----------------------------------------------------------------------

OVGTopAttributes::OVGTopAttributes () : OVGAttributesList (NULL)
{
   sortedList = new Vector<String> (4, false);
}

OVGTopAttributes::~OVGTopAttributes ()
{
   delete sortedList;
}

int OVGTopAttributes::add (String *key, OVGAttribute *attribute)
{
   int newPos = sortedList->insertSorted (key);
   attributes->put (key, attribute);
   return newPos;
}

void OVGTopAttributes::show ()
{
   // Keep the top list visible.
}

void OVGTopAttributes::hide ()
{
   // Keep the top list visible.
}

// ----------------------------------------------------------------------

OVGAttribute::OVGAttribute (const char *name, OVGAttributesList *parent) :
   OVGAttributesList (parent)
{
   this->name = strdup (name);
}

OVGAttribute::~OVGAttribute ()
{
   free (name);
}

void OVGAttribute::initWidgets (Style *widgetStyle, dw::Box *parent,
                                bool showLarge, int newPos)
{
   hbox = new HBox (false);
   hbox->setStyle (widgetStyle);
   parent->addChild (hbox, newPos);

   label = new Label (name);
   label->setStyle (widgetStyle);
   hbox->addChild (label);

   OVGAttributesList::initWidgets (widgetStyle, hbox, showLarge);
}

int OVGAttribute::add (String *key, OVGAttribute *attribute)
{
   attributes->put (key, attribute);
   return -1;
}

void OVGAttribute::show ()
{
   hbox->show ();
}

void OVGAttribute::hide ()
{
   hbox->hide ();
}

// ----------------------------------------------------------------------

ObjViewGraph::GraphObject::GraphObject (ObjViewGraph *graph, const char *id)
{
   this->graph = graph;
   this->id = strdup (id);

   className = NULL;
   node = NULL;
   attributes = NULL;
   messageStyle = graph->noBorderStyle;
   messageStyle->ref ();
}

ObjViewGraph::GraphObject::~GraphObject ()
{
   // Simplified, when the whole graph is deleted.
   if (node && !graph->inDestructor)
      delete node;

   free (id);
   if (className)
      free (className);
   if (attributes)
      delete attributes;
   if (messageStyle)
      messageStyle->unref ();
}

// ----------------------------------------------------------------------

int ObjViewGraph::ColorComparator::specifity (const char *pattern)
{
   int s = 0;
   for (const char *p = pattern; *p; p++) {
      if (*p == '*')
         s -= 100;
      else if (*p == '?')
         s += 1;
      else
         s += 100;
   }
   return s;
}

int ObjViewGraph::ColorComparator::compare (Object *o1, Object *o2)
{
   return specifity (((Color*)o2)->identifier)
      - specifity (((Color*)o1)->identifier);
}

// ----------------------------------------------------------------------

ObjViewGraph::Color::Color (const char *identifier, const char *color,
                            Layout *layout)
{
   this->identifier = strdup (identifier);

   this->color = NULL;
   if (*color == '#') {
      char *end;
      int c = (int)strtol (color + 1, &end, 16);
      if (*end == 0) {
         if (end - color == 7)
            this->color = style::Color::create (layout, c);
         else if (end - color == 4)
            this->color =
               style::Color::create (layout,
                                     (c & 0xf00) >> 8 | (c && 0xf0) >> 4
                                     | (c & 0xf));
         this->color->ref ();
      }
   }
   
   if (this->color == NULL)
      fprintf (stderr, "WARNING: invalid color '%s'.\n", color);
}

ObjViewGraph::Color::~Color ()
{
   free (identifier);

   if (this->color)
      this->color->unref ();
}

// ----------------------------------------------------------------------

ObjViewGraph::ObjViewGraph (ObjViewFilterTool *filterTool)
{
   inDestructor = false;

   this->filterTool = filterTool;

   commands = new Vector<OVGCommand> (4, true);
   navigableCommands = new Vector<OVGCommand> (4, false);

   objectsById = new HashTable<String, GraphObject> (true, false);
   allObjects = new Vector<GraphObject> (1, true);
   classColors = new Vector<Color> (1, true);
   objectColors = new Vector<Color> (1, true);
   enterCommands = new Stack<OVGEnterCommand> (false);      
   startCommands = new Stack<OVGIncIndentCommand> (false);

   navigableCommandsPos = hiddenBefore = hiddenAfter = -1;
   numVisibleCommands = 0;
   objectContents = objectMessages = true;
   codeViewer = strdup ("xterm -e 'vi +%n %p' &");

   stacktraceWindows = new List <ObjViewStacktraceWindow> (true);
   stacktraceListener = new ObjViewGraphListener (this);

   nodeStyle = noBorderStyle = topBorderStyle = bottomBorderStyle =
      leftBorderStyle = NULL;
}

ObjViewGraph::~ObjViewGraph ()
{
   inDestructor = true;

   free (codeViewer);
   delete commands;
   delete navigableCommands;

   delete objectsById;
   delete allObjects;
   delete classColors;
   delete objectColors;
   delete enterCommands;
   delete startCommands;

   delete stacktraceWindows;
   delete stacktraceListener;

   if (nodeStyle)
      nodeStyle->unref ();
   if (noBorderStyle)
      noBorderStyle->unref ();
   if (topBorderStyle)
      topBorderStyle->unref ();
   if (bottomBorderStyle)
      bottomBorderStyle->unref ();
   if (leftBorderStyle)
      leftBorderStyle->unref ();
}

void ObjViewGraph::initStyles (const char *fontName, int fontSize, int fgColor,
                               int graphBgColor, int objBgColor,
                               int borderThickness, int graphMargin)
{
   StyleAttrs styleAttrs;
   styleAttrs.initValues ();

   FontAttrs fontAttrs;
   fontAttrs.name = fontName;
   fontAttrs.size = fontSize;
   fontAttrs.weight = 400;
   fontAttrs.style = FONT_STYLE_NORMAL;
   fontAttrs.letterSpacing = 0;
   fontAttrs.fontVariant = FONT_VARIANT_NORMAL;
   styleAttrs.font = style::Font::create (layout, &fontAttrs);

   styleAttrs.padding.setVal (graphMargin);
   styleAttrs.color = style::Color::create (layout, fgColor);
   styleAttrs.setBorderColor (styleAttrs.color);
   styleAttrs.setBorderStyle (BORDER_SOLID);
   styleAttrs.backgroundColor = style::Color::create (layout, graphBgColor);
   Style *graphStyle = Style::create (&styleAttrs);
   setStyle (graphStyle);

   styleAttrs.padding.setVal (0);
   styleAttrs.backgroundColor = style::Color::create (layout, objBgColor);
   styleAttrs.borderWidth.setVal (1);
   nodeStyle = Style::create (&styleAttrs);
   setRefStyle (nodeStyle);

   styleAttrs.backgroundColor = NULL;
   styleAttrs.borderWidth.setVal (0);
   noBorderStyle = Style::create (&styleAttrs);

   styleAttrs.borderWidth.setVal (0);
   styleAttrs.borderWidth.top = 1;
   topBorderStyle = Style::create (&styleAttrs);

   styleAttrs.borderWidth.setVal (0);
   styleAttrs.borderWidth.bottom = 1;
   bottomBorderStyle = Style::create (&styleAttrs);

   styleAttrs.borderWidth.setVal (0);
   styleAttrs.borderWidth.left = 1;
   leftBorderStyle = Style::create (&styleAttrs);
}

ObjViewGraph::GraphObject *ObjViewGraph::ensureObject (const char *id)
{
   String key (id);
   if (!objectsById->contains (&key)) {
      GraphObject *obj = new GraphObject (this, id);
      
      obj->node = new Toggle (objectContents);
      applyClassOrObjectStyle (obj);
      addNode (obj->node);
      
      obj->id1 = new Label (id);
      obj->id1->setStyle (noBorderStyle);
      obj->node->setSmall (obj->id1);
         
      VBox *large = new VBox (false);
      large->setStyle (leftBorderStyle);
      obj->node->setLarge (large);
         
      obj->id2 = new Label (id);
      obj->id2->setStyle (bottomBorderStyle);
      large->addChild (obj->id2);
         
      obj->attributes = new OVGTopAttributes ();
      obj->attributes->initWidgets (noBorderStyle, large, true);
         
      Toggle *mToggle = new Toggle (objectMessages);
      mToggle->setStyle (topBorderStyle);
      large->addChild (mToggle);
         
      obj->messages = new VBox (false);
      obj->messages->setStyle (noBorderStyle);
      mToggle->setLarge (obj->messages);

      objectsById->put (new String (id), obj);
      allObjects->put (obj);
   }

   return objectsById->get (&key);
}

void ObjViewGraph::addMessage (const char *id, const char *message,
                               HBox **mainBox, Label **indexLabel,
                               Label **mainLabel)
{
   GraphObject *obj = ensureObject (id);

   HBox *hbox = new HBox (false);
   hbox->setStyle (noBorderStyle);
   obj->messages->addChild (hbox);
   
   Label *label1 = new Label ("???: ");
   label1->setStyle (obj->messageStyle);
   hbox->addChild (label1);
      
   Label *label2 = new Label (message);
   label2->setStyle (noBorderStyle);
   hbox->addChild (label2);

   if (mainBox)
      *mainBox = hbox;
   if (indexLabel)
      *indexLabel = label1;
   if (mainLabel)
      *mainLabel = label2;
}

bool ObjViewGraph::incIndent (const char *id)
{
   GraphObject *obj = ensureObject (id);

   Style *oldMessageStyle = obj->messageStyle;
   StyleAttrs styleAttrs = *(obj->messageStyle);
   styleAttrs.padding.left += 3 * obj->messageStyle->font->spaceWidth;
   obj->messageStyle = Style::create (&styleAttrs);
   oldMessageStyle->unref ();

   return true; // just for symmetry with decIndent
}

bool ObjViewGraph::decIndent (const char *id)
{
   GraphObject *obj = ensureObject (id);

   Style *oldMessageStyle = obj->messageStyle;
   int indentWidth = 3 * obj->messageStyle->font->spaceWidth;
   if (oldMessageStyle->padding.left >= indentWidth) {
      StyleAttrs styleAttrs = *(obj->messageStyle);
      styleAttrs.padding.left -= indentWidth;
      obj->messageStyle = Style::create (&styleAttrs);
      oldMessageStyle->unref ();
      return true;
   } else
      return false;
}

OVGAttribute *ObjViewGraph::addAttribute (const char *id, const char *name,
                                          const char *value, Label **smallLabel,
                                          HBox **histBox, Label **indexLabel,
                                          Label **histLabel)
{
   GraphObject *obj = ensureObject (id);

   int numDots = 0;
   for (const char *s = name; *s; s++)
      if (*s == '.')
         numDots++;

   //printf ("'%s' contains %d dots\n", name, numDots);
   char **parts = new char*[numDots + 2];
   
   int i = 0;
   const char *s = name;
   for (const char *e = s; i < numDots + 1; e++)
      if (*e == '.') {
         parts[i] = strndup (s, e - s);
         //printf ("%d of %d: '%s'\n", i, numDots + 1, parts[i]);
         s = e + 1;
         i++;
      } else if (*e == 0) {
         parts[i] = strdup (s);
         //printf ("%d of %d (last): '%s'\n", i, numDots + 1, parts[i]);
         s = e;
         i++;
      }

   //printf ("finished\n");

   parts[numDots + 1] = NULL;

   OVGAttribute *attribute = addAttribute (obj->attributes, parts, value,
                                           smallLabel, histBox, indexLabel,
                                           histLabel);

   for (char **p  = parts; *p; p++)
      free (*p);

   delete[] parts;

   return attribute;
}

// Returns "large" label.
OVGAttribute *ObjViewGraph::addAttribute (OVGAttributesList *attributesList,
                                          char **parts, const char *value,
                                          Label **smallLabel, HBox **histBox,
                                          Label **indexLabel, Label **histLabel)
{
   if (*parts) {
      String key (*parts);
      OVGAttribute *attribute = attributesList->get (&key);
      if (attribute == NULL) {
         String *str = new String (*parts);
         attribute = new OVGAttribute (*parts, attributesList);
         int newPos = attributesList->add (str, attribute);
         attribute->initWidgets (noBorderStyle, attributesList->vbox, false,
                                 newPos);
      }

      OVGAttribute *subAttribute = addAttribute (attribute, parts + 1, value,
                                                 smallLabel, histBox,
                                                 indexLabel, histLabel);
      return subAttribute ? subAttribute : attribute;
   } else {
      HBox *hbox = new HBox (false);
      hbox->setStyle (noBorderStyle);
      attributesList->vbox->addChild (hbox);

      Label *label1 = new Label ("???: ");
      label1->setStyle (noBorderStyle);
      hbox->addChild (label1);
      
      Label *label2 = new Label (value);
      label2->setStyle (noBorderStyle);
      hbox->addChild (label2);

      if (attributesList->toggle->getSmall () != NULL) {
         assert (attributesList->toggle->getSmall()
                    ->instanceOf (Label::CLASS_ID));
         ((Label*)attributesList->toggle->getSmall())->setText (value);
      } else {
         Label *cur = new Label (value);
         cur->setStyle (noBorderStyle);
         attributesList->toggle->setSmall (cur);
      }

      if (smallLabel)
         *smallLabel = (Label*)attributesList ->toggle->getSmall ();
      if (histBox)
         *histBox = hbox;
      if (indexLabel)
         *indexLabel = label1;
      if (histLabel)
         *histLabel = label2;

      // Here, no attribute is created; the caller must discard this
      // return value.
      return NULL;
   }
}

void ObjViewGraph::setClassName (const char *id, const char *className)
{
   GraphObject *obj = ensureObject (id);

   if (obj->className)
      free (obj->className);
   obj->className = strdup (className);

   int bufLen = strlen (id) + 2 + strlen (className) + 1;
   char *buf = new char[bufLen];
   snprintf (buf, bufLen, "%s: %s", id, className);

   obj->id1->setText (buf);
   obj->id2->setText (buf);

   delete[] buf;

   applyClassOrObjectStyle (obj);
}

style::Color *ObjViewGraph::getObjectColor (GraphObject *obj)
{
   style::Color *objectColor = NULL;   

   // TODO Identities are currently not considered
   for (int i = 0; objectColor == NULL && i < objectColors->size (); i++) {
      Color *color = objectColors->get (i);
      if (color->color && strcmp (color->identifier, obj->id) == 0)
         objectColor = color->color;
   }

   if (obj->className) {
      for (int i = 0; objectColor == NULL && i < classColors->size (); i++) {
         Color *color = classColors->get (i);
         if (color->color &&
             fnmatch (color->identifier, obj->className, 0) == 0)
            objectColor = color->color;
      }
   }

   return objectColor;
}

void ObjViewGraph::applyClassOrObjectStyle (GraphObject *obj)
{
   style::Color *usedColor = getObjectColor (obj);
  
   if (usedColor) {
      // TODO Perhaps it is better to create styles initially (and
      // change them when setStyle is called?). Should especially
      // reduce memory usage.
      StyleAttrs attrs = *nodeStyle;
      attrs.backgroundColor = usedColor;
      Style *newStyle = Style::create (&attrs);
      obj->node->setStyle (newStyle);
      newStyle->unref ();
   } else
      obj->node->setStyle (nodeStyle);
}


void ObjViewGraph::applyClassOrObjectStyles ()
{
   for (typed::Iterator<String> it = objectsById->iterator ();
        it.hasNext (); ) {
      String *key = it.getNext ();
      GraphObject *obj = (GraphObject*) objectsById->get(key);
      applyClassOrObjectStyle (obj);
   }
}

int ObjViewGraph::getObjectColor (const char *id)
{
   String key (id);
   GraphObject *obj = (GraphObject*) objectsById->get (&key);
   if (obj) {
      style::Color *objectColor = getObjectColor (obj);
      return objectColor ?
         objectColor->getColor () : nodeStyle->backgroundColor->getColor ();
   } else
      return nodeStyle->backgroundColor->getColor ();
}

void ObjViewGraph::addIdentity (const char *id1, const char *id2)
{
   // Done in ObjIdentController.
}

void ObjViewGraph::addAssoc (const char *id1, const char *id2)
{
   GraphObject *obj1 = ensureObject (id1);
   GraphObject *obj2 = ensureObject (id2);
   addEdge (obj1->node, obj2->node);
}

void ObjViewGraph::setClassColor (const char *klass, const char *color)
{
   ColorComparator cmp;
   classColors->insertSorted (new Color (klass, color, layout), &cmp);
   applyClassOrObjectStyles ();
}

void ObjViewGraph::setObjectColor (const char *id, const char *color)
{
   objectColors->put (new Color (id, color, layout));
   applyClassOrObjectStyles ();
}

void ObjViewGraph::addCommand (OVGCommand *command, bool navigable)
{
   commands->put (command);
   command->exec (this);

   if (navigable) {
      navigableCommands->put (command);
      command->setNavigableCommandsIndex (this, navigableCommands->size () -1);

      if (command->calcVisibility (this)) {
         command->show (this);
         command->setVisibleIndex (this, numVisibleCommands);
         numVisibleCommands++;
      } else
         command->hide (this);
   }
}

void ObjViewGraph::clearSelection ()
{
   if (navigableCommandsPos != -1)
      navigableCommands->get(navigableCommandsPos)->unselect (this);
}

void ObjViewGraph::unclearSelection ()
{
   if (navigableCommandsPos != - 1) {
      navigableCommands->get(navigableCommandsPos)->select (this);
      navigableCommands->get(navigableCommandsPos)->scrollTo (this);
   }
}

int ObjViewGraph::firstCommand ()
{
   if (hiddenBefore != -1)
      return hiddenBefore;
   else
      return navigableCommands->size () > 0 ? 0 : -1;
}

int ObjViewGraph::lastCommand ()
{
   if (hiddenAfter != -1)
      return hiddenAfter;
   else
      return navigableCommands->size () - 1; // may be -1
}

void ObjViewGraph::previousCommand ()
{
   clearSelection ();

   if (numVisibleCommands == 0)
      // No visible command;
      navigableCommandsPos = -1;
   else
      do {
         if (navigableCommandsPos == -1)
            navigableCommandsPos = lastCommand ();
         else {
            navigableCommandsPos--;
            if (navigableCommandsPos < firstCommand ())
               navigableCommandsPos = lastCommand ();
         }
      } while (!navigableCommands->get(navigableCommandsPos)->isVisible (this));

   unclearSelection ();
}

void ObjViewGraph::nextCommand ()
{
   clearSelection ();

   if (numVisibleCommands == 0)
      // No visible command;
      navigableCommandsPos = -1;
   else {
      do {
         if (navigableCommandsPos == -1)
            navigableCommandsPos = firstCommand ();
         else {
            navigableCommandsPos++;
            if (navigableCommandsPos > lastCommand ())
               navigableCommandsPos = firstCommand ();
         }
      } while (!navigableCommands->get(navigableCommandsPos)->isVisible (this));
   }
   
   unclearSelection ();
}

void ObjViewGraph::viewCodeOfCommand ()
{
   if (navigableCommandsPos != -1) {
      OVGCommand *command = navigableCommands->get (navigableCommandsPos);

      char lineNoBuf[32];
      char partBuf[32] = { 0, 0 };
      StringBuffer sb;

      snprintf (lineNoBuf, 32, "+%d", command->getLineNo ());

      for (char *s = codeViewer; *s; s++) {
         if (*s == '%' && (s[1] == 'p' || s[1] == 'n')) {
            s++;
            switch (*s) {
            case 'p':
               sb.append (command->getFileName ());
               break;

            case 'n':
               sb.append (lineNoBuf);
               break;
            }
         } else {
            *partBuf = *s;
            sb.append (partBuf);
         }
      }

      system (sb.getChars());
   }
}

void ObjViewGraph::showStackTraceOfCommand ()
{
   if (navigableCommandsPos != -1) {
      OVGCommand *command = navigableCommands->get (navigableCommandsPos);
      ObjViewStacktraceWindow *stWin =
         new ObjViewStacktraceWindow (command->getFunction (),
                                      stacktraceListener);
      stacktraceWindows->append (stWin);
      stWin->show ();
      stWin->update ();
   }
}

void ObjViewGraph::switchBetweenRelatedCommands ()
{
   if (navigableCommandsPos != -1) {
      OVGCommand *command = navigableCommands->get (navigableCommandsPos);
      OVGCommand *relatedCommand = command->getRelatedCommand ();
      if (relatedCommand && relatedCommand->isVisible (this)) {
         clearSelection ();
         navigableCommandsPos =
            relatedCommand->getNavigableCommandsIndex (this);
         unclearSelection ();
      }
   }
}

void ObjViewGraph::setCommand (int index)
{
   clearSelection ();
   navigableCommandsPos = index;
   unclearSelection ();
}

void ObjViewGraph::hideBeforeCommand ()
{
   // Note: The selected command cannot be hidden, so there are never
   // elements to show again.
   
   if (navigableCommandsPos != -1) {
      hiddenBefore = navigableCommandsPos;
      for (int i = 0; i < hiddenBefore; i++)
         navigableCommands->get(i)->hide (this);
   }

   recalculateCommandsVisibility (); // Could be faster
}

void ObjViewGraph::hideAfterCommand ()
{
   // See also note in hideBeforeCommand().
   
   if (navigableCommandsPos != -1) {
      hiddenAfter = navigableCommandsPos;
      for (int i = hiddenAfter + 1; i < navigableCommands->size (); i++)
         navigableCommands->get(i)->hide (this);
   }
   
   recalculateCommandsVisibility (); // Could be faster
}

void ObjViewGraph::hideAllCommands ()
{
   navigableCommandsPos = -1;
   hiddenBefore = navigableCommands->size ();
   for (int i = 0; i < navigableCommands->size (); i++)
      navigableCommands->get(i)->hide (this);
   
   recalculateCommandsVisibility (); // Necessary?
}

void ObjViewGraph::showBeforeCommand ()
{
   if (hiddenBefore != -1) {
      hiddenBefore = -1;
      recalculateCommandsVisibility (); // Could be faster
   }      
}

void ObjViewGraph::showAfterCommand ()
{
   if (hiddenAfter != -1) {
      hiddenAfter = -1;
      recalculateCommandsVisibility (); // Could be faster
   }      
}

void ObjViewGraph::showAllCommands ()
{
   if (hiddenBefore != -1 || hiddenAfter != -1) {
      hiddenBefore = hiddenAfter = -1;
      recalculateCommandsVisibility (); // Could be faster
   }      
}

void ObjViewGraph::setCodeViewer (const char *codeViewer)
{
   free (this->codeViewer);
   this->codeViewer = strdup (codeViewer);
}

void ObjViewGraph::recalculateCommandsVisibility ()
{
   numVisibleCommands = 0;
   for (int i = max (firstCommand (), 0); i <= lastCommand (); i++) {
      OVGCommand *command = navigableCommands->get (i);

      if (command->calcVisibility (this)) {
         command->show (this);
         command->setVisibleIndex (this, numVisibleCommands);       
         numVisibleCommands++;
      } else
         command->hide (this);
   }

   if (navigableCommandsPos != -1) {
      if (!navigableCommands->get(navigableCommandsPos)->isVisible (this))
         // Selected command has become invisible. Select next one. (May
         // also be the first one, if, e. g., the selected one was the last
         // one. Could test this and instead select the last visible
         // *before*.)
         nextCommand ();
      else
         // May have become out of view.
         navigableCommands->get(navigableCommandsPos)->scrollTo (this);
   }

   // Update stacktrace windows (hidden commands should not be selectable).
   for (lout::container::typed::Iterator <ObjViewStacktraceWindow> it =
           stacktraceWindows->iterator (); it.hasNext (); ) {
      ObjViewStacktraceWindow *stWin = it.getNext ();
      stWin->update ();
   }
}

} // namespace objects

} // namespace rtfl
