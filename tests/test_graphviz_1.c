#include <graphviz/gvc.h>

int main(int argc, char *argv[])
{
   Agnode_t *node1, *node2;
   Agedge_t *edge1;
   Agraph_t *graph;
   GVC_t *gvc;
   
   gvc = gvContext ();
   graph = agopen ("graph", Agdirected, NULL);

   node1 = agnode(graph, "node1", TRUE);
   agsafeset (node1, "width", "1", "");
   agsafeset (node1, "height", "1", "");
   
   node2 = agnode(graph, "node2", TRUE);
   agsafeset (node2, "width", "1", "");
   agsafeset (node2, "height", "1", "");

   edge1 = agedge(graph, node1, node2, "edge1", TRUE);

   puts ("---------- initially ----------");
   agwrite (graph, stdout);

   gvLayout (gvc, graph, "dot");
   gvRender(gvc, graph, "dot", NULL);
   gvFreeLayout(gvc, graph);

   puts ("---------- after first layouting ----------");
   agwrite (graph, stdout);

   agsafeset (node2, "height", "2", "");

   gvLayout (gvc, graph, "dot");
   gvRender(gvc, graph, "dot", NULL);
   gvFreeLayout(gvc, graph);

   puts ("---------- after second layouting ----------");
   agwrite (graph, stdout);

   agclose (graph);  
   gvFreeContext(gvc);

   return 0;
}
