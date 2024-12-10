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

#include "objview_graph.hh" // Because ObjViewGraph is needed here.

using namespace lout::object;
using namespace lout::container::typed;
using namespace lout::misc;
using namespace dw::core;
using namespace dw::core::style;
using namespace rtfl::dw;

namespace rtfl {

namespace objects {

static void scrollToLabel (Label *label)
{
   Widget *parent = label->getParent (), *child = label;

   while (parent != NULL && !parent->instanceOf (ObjViewGraph::CLASS_ID)) {
      if (parent->instanceOf (Toggle::CLASS_ID)) {
         Toggle *toggle = (Toggle*)parent;
         if (toggle->isLargeShown () && child == toggle->getSmall ())
            toggle->toggle (false);
         else if (!toggle->isLargeShown () && child == toggle->getLarge ())
            toggle->toggle (true);
      }

      child = parent;
      parent = parent->getParent ();
   }

   label->scrollTo (HPOS_INTO_VIEW, VPOS_INTO_VIEW);
}

// ----------------------------------------------------------------------

bool OVGCommonCommand::CommandLinkReceiver::click (Widget *widget, int link,
                                                   int img, int x, int y,
                                                   EventButton *event)
{
   // "link" is "navigableCommandsIndex".
   graph->setCommand (link);
   return true;
}

// ----------------------------------------------------------------------

OVGCommonCommand::OVGCommonCommand (ObjViewGraph *graph, const char *id,
                                    ObjViewFunction *function)
{
   this->id = strdup (id);
   this->graph = graph;
   this->function = function;
   relatedCommand = NULL;
   executed = false;
   visible = true;
}

OVGCommonCommand::~OVGCommonCommand ()
{
   free (id);
}

const char *OVGCommonCommand::getFileName ()
{
   // Not needed for non-navigable command.
   assertNotReached ();
   return NULL;
}

int OVGCommonCommand::getLineNo ()
{
   // Not needed for non-navigable command.
   assertNotReached ();
   return 0;
}

int OVGCommonCommand::getProcessId ()
{
   // Not yet clear what to do with this one.
   assertNotReached ();
   return 0;
}

void OVGCommonCommand::exec (ObjViewGraph *graph)
{
   assert (!executed);
   assert (graph == this->graph);
   doExec ();
   executed = true;
}

void OVGCommonCommand::show (ObjViewGraph *graph)
{
   assert (executed);
   assert (graph == this->graph);
   visible = true;
   doShow ();
}

void OVGCommonCommand::hide (ObjViewGraph *graph)
{
   assert (executed);
   assert (graph == this->graph);
   visible = false;
   doHide ();
}

void OVGCommonCommand::scrollTo (ObjViewGraph *graph)
{
   assert (executed);
   assert (graph == this->graph);
   doScrollTo ();
}

void OVGCommonCommand::select (ObjViewGraph *graph)
{
   assert (executed);
   assert (graph == this->graph);
   doSelect ();
}

void OVGCommonCommand::unselect (ObjViewGraph *graph)
{
   assert (executed);
   assert (graph == this->graph);
   doUnselect ();
}

// Most commands do not have to impelemt these:

void OVGCommonCommand::doShow ()
{
   assertNotReached ();
}

void OVGCommonCommand::doHide ()
{
   assertNotReached ();
}

void OVGCommonCommand::doScrollTo ()
{
   assertNotReached ();
}

void OVGCommonCommand::doSelect ()
{
   assertNotReached ();
}

void OVGCommonCommand::doUnselect ()
{
   assertNotReached ();
}

bool OVGCommonCommand::isVisible (ObjViewGraph *graph)
{
   return visible;
}

bool OVGCommonCommand::calcVisibility (ObjViewGraph *graph)
{
   return true;
}

int OVGCommonCommand::getNavigableCommandsIndex (ObjViewGraph *graph)
{
   assertNotReached ();
   return -1;
}

void OVGCommonCommand::setNavigableCommandsIndex (ObjViewGraph *graph,
                                                  int navigableCommandsIndex)
{
   assertNotReached ();
}

void OVGCommonCommand::setVisibleIndex (ObjViewGraph *graph,
                                        int visibleIndex)
{
   assertNotReached ();
}

// ----------------------------------------------------------------------

OVGNavigableCommand::OVGNavigableCommand (const char *fileName, int lineNo,
                                          ObjViewGraph *graph, const char *id,
                                          ObjViewFunction *function) :
   OVGCommonCommand (graph, id, function)
{
   this->fileName = strdup (fileName);
   this->lineNo = lineNo;

   navigableCommandsIndex = -1;
   label1 = label2 = NULL;
   hbox = NULL;
}

OVGNavigableCommand::~OVGNavigableCommand ()
{
   free (fileName);
}

const char *OVGNavigableCommand::getFileName ()
{
   return fileName;
}

int OVGNavigableCommand::getLineNo ()
{
   return lineNo;
}

void OVGNavigableCommand::initWidgets ()
{
   label1->setLink (navigableCommandsIndex);
   label1->connectLink (&linkReceiver);
   label2->setLink (navigableCommandsIndex);
   label2->connectLink (&linkReceiver);
}

void OVGNavigableCommand::removeWidgets ()
{
   delete hbox;

   label1 = label2 = NULL;
   hbox = NULL;
}

int OVGNavigableCommand::getNavigableCommandsIndex (ObjViewGraph *graph)
{
   return navigableCommandsIndex;
}

void OVGNavigableCommand::setNavigableCommandsIndex (ObjViewGraph *graph,
                                                     int navigableCommandsIndex)
{
   this->navigableCommandsIndex = navigableCommandsIndex;
   if (label1)
      label1->setLink (navigableCommandsIndex);
   if (label2)
      label2->setLink (navigableCommandsIndex);
}

void OVGNavigableCommand::setVisibleIndex (ObjViewGraph *graph,
                                              int visibleIndex)
{
   assert (label1 != NULL);

   // Assume 5 digits (99999) at max.
   char buf[15];
   snprintf (buf, 15, "<i>%d:</i> ", visibleIndex);
   label1->setText (buf);
}

void OVGNavigableCommand::doShow ()
{
   label1->show ();
   label2->show ();
}

void OVGNavigableCommand::doHide ()
{
   label1->hide ();
   label2->hide ();
}

void OVGNavigableCommand::doScrollTo ()
{
   scrollToLabel (label2);
}

void OVGNavigableCommand::doSelect ()
{
   label1->select ();
   label2->select ();
}

void OVGNavigableCommand::doUnselect ()
{
   label1->unselect ();
   label2->unselect ();
}

// ----------------------------------------------------------------------

OVGIncIndentCommand::OVGIncIndentCommand (const char *fileName, int lineNo,
                                          ObjViewGraph *graph, const char *id,
                                          ObjViewFunction *function) :
   OVGNavigableCommand (fileName, lineNo, graph, id, function)
{
   linkReceiver.setData (graph, this);
}

OVGIncIndentCommand::~OVGIncIndentCommand ()
{
}

bool OVGIncIndentCommand::calcVisibility (ObjViewGraph *graph)
{
   return graph->filterTool->isTypeSelected (OVG_COMMAND_INDENT);
}

void OVGIncIndentCommand::doExec ()
{
   graph->addMessage (id, "<i>start</i>", &hbox, &label1, &label2);
   initWidgets ();
   success = graph->incIndent (id);
}

// ----------------------------------------------------------------------

OVGDecIndentCommand::OVGDecIndentCommand (const char *fileName, int lineNo,
                                          ObjViewGraph *graph, const char *id,
                                          OVGIncIndentCommand *startCommand,
                                          ObjViewFunction *function) :
   OVGNavigableCommand (fileName, lineNo, graph, id, function)
{
   linkReceiver.setData (graph, this);

   setRelatedCommand (startCommand);
   startCommand->setRelatedCommand (this);
}

OVGDecIndentCommand::~OVGDecIndentCommand ()
{
}

bool OVGDecIndentCommand::calcVisibility (ObjViewGraph *graph)
{
   return graph->filterTool->isTypeSelected (OVG_COMMAND_INDENT);
}

void OVGDecIndentCommand::doExec ()
{
   success = graph->decIndent (id);
   graph->addMessage (id, "<i>end</i>", &hbox, &label1, &label2);
   initWidgets ();
}

// ----------------------------------------------------------------------

OVGEnterCommand::OVGEnterCommand (const char *fileName, int lineNo,
                                  ObjViewGraph *graph, const char *id,
                                  const char *aspect, int prio,
                                  const char *funname, const char *args,
                                  ObjViewFunction *function) :
   OVGNavigableCommand (fileName, lineNo, graph, id, function)
{
   linkReceiver.setData (graph, this);

   graph->filterTool->addAspect (aspect);
   graph->filterTool->addPriority (prio);

   this->aspect = strdup (aspect);
   this->prio = prio;
   this->funname = strdup (funname);
   this->args = strdup (args);
}

OVGEnterCommand::~OVGEnterCommand ()
{
   free (aspect);
   free (funname);
   free (args);
}

bool OVGEnterCommand::calcVisibility (ObjViewGraph *graph)
{
   return graph->filterTool->isTypeSelected (OVG_COMMAND_FUNCTION) &&
      graph->filterTool->isAspectSelected (aspect) &&
      graph->filterTool->isPrioritySelected (prio);
}

void OVGEnterCommand::doExec ()
{
   char *message = createMessage ("<i>enter</i>: <b>", "</b> (", ")", "");
   graph->addMessage (id, message, &hbox, &label1, &label2);
   freeMessage (message);
   initWidgets ();

   success = graph->incIndent (id);
}

char *OVGEnterCommand::createMessage (const char *c1, const char *c2,
                                      const char *c3a, const char *c3b)
{
   size_t l = strlen (c1) + strlen (funname) + strlen (c2) + strlen (args) +
      strlen (c3a) + strlen (c3b) + 1;
   char *message = new char[l];
   snprintf (message, l, "%s%s%s%s%s%s", c1, funname, c2, args, c3a, c3b);
   return message;
}

void OVGEnterCommand::freeMessage (char *message)
{
   delete[] message;
}

char *OVGEnterCommand::createName ()
{
   size_t l =
      1 + strlen (id) + 2 + strlen (funname) + 2 + strlen (args) + 1 + 1;
   char *message = new char[l];
   snprintf (message, l, "[%s] %s (%s)", id, funname, args);
   return message;
}

void OVGEnterCommand::freeName (char *name)
{
   delete[] name;
}

int OVGEnterCommand::getColor ()
{
   return graph->getObjectColor (id);
}

ObjViewFunction *OVGEnterCommand::getParent ()
{
   return getFunction ();
}

bool OVGEnterCommand::isSelectable ()
{
   return isVisible (graph);
}

void OVGEnterCommand::select ()
{
   graph->clearSelection ();
   graph->navigableCommandsPos = navigableCommandsIndex;
   graph->unclearSelection ();
}

// ----------------------------------------------------------------------

OVGLeaveCommand::OVGLeaveCommand (const char *fileName, int lineNo,
                                  ObjViewGraph *graph, const char *id,
                                  const char *vals,
                                  OVGEnterCommand *enterCommand) :
   OVGNavigableCommand (fileName, lineNo, graph, id, enterCommand)
{
   linkReceiver.setData (graph, this);

   this->enterCommand = enterCommand;
   this->vals = vals ? strdup (vals) : NULL;

   setRelatedCommand (enterCommand);
   enterCommand->setRelatedCommand (this);
}

OVGLeaveCommand::~OVGLeaveCommand ()
{
   if (vals)
      free (vals);
}

bool OVGLeaveCommand::calcVisibility (ObjViewGraph *graph)
{
   return enterCommand->calcVisibility (graph);
}

void OVGLeaveCommand::doExec ()
{
   success = graph->decIndent (id);

   char *message = vals ?
      enterCommand->createMessage ("<i>leave: ", "</i> (", ") ⇒ ",  vals) :
      enterCommand->createMessage ("<i>leave: ", "</i> (", ")", "");

   graph->addMessage (id, message, &hbox, &label1, &label2);
   enterCommand->freeMessage (message);
   initWidgets ();
}

// ----------------------------------------------------------------------

OVGAddMessageCommand::OVGAddMessageCommand (const char *fileName, int lineNo,
                                            ObjViewGraph *graph,
                                            const char *id, const char *aspect,
                                            int prio, const char *message,
                                            ObjViewFunction *function) :
   OVGNavigableCommand (fileName, lineNo, graph, id, function)
{
   linkReceiver.setData (graph, this);

   graph->filterTool->addAspect (aspect);
   graph->filterTool->addPriority (prio);

   this->aspect = strdup (aspect);
   this->prio = prio;
   this->message = strdup (message);
}

OVGAddMessageCommand::~OVGAddMessageCommand ()
{
   free (aspect);
   free (message);
}

bool OVGAddMessageCommand::calcVisibility (ObjViewGraph *graph)
{
   return graph->filterTool->isTypeSelected (OVG_COMMAND_MESSAGE) &&
      graph->filterTool->isAspectSelected (aspect) &&
      graph->filterTool->isPrioritySelected (prio);
}

void OVGAddMessageCommand::doExec ()
{
   graph->addMessage (id, message, &hbox, &label1, &label2);
   initWidgets ();
}

// ----------------------------------------------------------------------

OVGAddMarkCommand::OVGAddMarkCommand (const char *fileName, int lineNo,
                                      ObjViewGraph *graph,
                                      const char *id, const char *aspect,
                                      int prio, const char *mark,
                                      ObjViewFunction *function) :
   OVGNavigableCommand (fileName, lineNo, graph, id, function)
{
   linkReceiver.setData (graph, this);
   
   graph->filterTool->addAspect (aspect);
   graph->filterTool->addPriority (prio);

   this->aspect = strdup (aspect);
   this->prio = prio;
   this->mark = strdup (mark);
}

OVGAddMarkCommand::~OVGAddMarkCommand ()
{
   free (aspect);
   free (mark);
}

bool OVGAddMarkCommand::calcVisibility (ObjViewGraph *graph)
{
   return graph->filterTool->isTypeSelected (OVG_COMMAND_MARK) &&
      graph->filterTool->isAspectSelected (aspect) &&
      graph->filterTool->isPrioritySelected (prio);
}

void OVGAddMarkCommand::doExec ()
{
   size_t l = 13 + strlen (mark) + 1;
   char *message = new char[l];
   snprintf (message, l, "<i>mark:</i> %s", mark);

   graph->addMessage (id, message, &hbox, &label1, &label2);
   initWidgets ();

   delete[] message;
}

bool OVGAddMarkCommand::isSelectable ()
{
   return isVisible (graph);
}

void OVGAddMarkCommand::select ()
{
   graph->clearSelection ();
   graph->navigableCommandsPos = navigableCommandsIndex;
   graph->unclearSelection ();
}

// ----------------------------------------------------------------------

OVGCreateCommand::OVGCreateCommand (const char *fileName, int lineNo,
                                    ObjViewGraph *graph, const char *id,
                                    const char *className,
                                    ObjViewFunction *function):
   OVGNavigableCommand (fileName, lineNo, graph, id, function)
{
   this->className = strdup (className);
   linkReceiver.setData (graph, this);
}

OVGCreateCommand::~OVGCreateCommand ()
{
   free (className);
}

bool OVGCreateCommand::calcVisibility (ObjViewGraph *graph)
{
   return graph->filterTool->isTypeSelected (OVG_COMMAND_CREATE);
}

void OVGCreateCommand::doExec ()
{
   size_t l = 15 + strlen (className) + 1;
   char *message = new char[l];
   snprintf (message, l, "<i>create:</i> %s", className);

   graph->addMessage (id, message, &hbox, &label1, &label2);
   initWidgets ();

   delete[] message;

   graph->setClassName (id, className);
}

// ----------------------------------------------------------------------

OVGAddAssocCommand::OVGAddAssocCommand (const char *fileName, int lineNo,
                                        ObjViewGraph *graph,
                                        const char *id1, const char *id2,
                                        ObjViewFunction *function) :
   OVGNavigableCommand (fileName, lineNo, graph, id1, function)
{
   linkReceiver.setData (graph, this);
   this->id2 = strdup (id2);
}

OVGAddAssocCommand::~OVGAddAssocCommand ()
{
   free (id2);
}

bool OVGAddAssocCommand::calcVisibility (ObjViewGraph *graph)
{
   return graph->filterTool->isTypeSelected (OVG_COMMAND_ASSOC);
}

void OVGAddAssocCommand::doExec ()
{
   size_t l = 14 + strlen (id2) + 5 + 1;
   char *message = new char[l];
   snprintf (message, l, "<i>assoc → %s</i>", id2);

   graph->addMessage (id, message, &hbox, &label1, &label2);
   initWidgets ();

   delete[] message;

   graph->addAssoc (id, id2);
}

// ----------------------------------------------------------------------

OVGAddAttrCommand::OVGAddAttrCommand (const char *fileName, int lineNo,
                                      ObjViewGraph *graph, const char *id,
                                      const char *name,const char *value,
                                      ObjViewFunction *function) :
   OVGNavigableCommand (fileName, lineNo, graph, id, function)
{
   linkReceiver.setData (graph, this);

   this->name = strdup (name);
   this->value = strdup (value);

   smallLabel = NULL;
   attribute = NULL;
   childNo = -1;
}

OVGAddAttrCommand::~OVGAddAttrCommand ()
{
   free (name);
   free (value);
}

bool OVGAddAttrCommand::calcVisibility (ObjViewGraph *graph)
{
   return graph->filterTool->isTypeSelected (OVG_COMMAND_ADD_ATTR);
}

void OVGAddAttrCommand::doExec ()
{
   attribute = graph->addAttribute (id, name, value, &smallLabel, NULL, &label1,
                                    &label2);
   childNo = attribute->registerChild ();
   initWidgets ();
}

void OVGAddAttrCommand::doShow ()
{
   OVGNavigableCommand::doShow ();

   // Set the appropriate current value (seen when the history is
   // hidden). It can be assumed that doShow() is called in the order
   // of the creation, so the last visible command will stay; this
   // does indeed represent the current value.
   assert (smallLabel != NULL);
   smallLabel->setText (value);

   // ...
   assert (attribute != NULL);
   assert (childNo != -1);
   attribute->childShown (childNo);
}

void OVGAddAttrCommand::doHide ()
{
   OVGNavigableCommand::doHide ();

   // ...
   assert (attribute != NULL);
   assert (childNo != -1);
   attribute->childHidden (childNo);
}

// ----------------------------------------------------------------------

OVGDeleteCommand::OVGDeleteCommand (const char *fileName, int lineNo,
                                    ObjViewGraph *graph, const char *id,
                                    ObjViewFunction *function) :
   OVGNavigableCommand (fileName, lineNo, graph, id, function)
{
   linkReceiver.setData (graph, this);
}

OVGDeleteCommand::~OVGDeleteCommand ()
{
}

bool OVGDeleteCommand::calcVisibility (ObjViewGraph *graph)
{
   return graph->filterTool->isTypeSelected (OVG_COMMAND_DELETE);
}

void OVGDeleteCommand::doExec ()
{
   graph->addMessage (id, "<i>delete</i>", &hbox, &label1, &label2);
   initWidgets ();

   // Change appearance of window (simple experiment).
   //Widget *node = graph->ensureObject(id)->node;
   //StyleAttrs attrs = *(node->getStyle());
   //attrs.setBorderStyle (BORDER_DOTTED);
   //node->setStyle (Style::create (&attrs));

   // Furthermore, this message is added often several times, since this command
   // is printed in all destructors in the class hierarchie. Should be limited
   // to once.
}

} // namespace objects

} // namespace rtfl
