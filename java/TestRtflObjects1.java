package rtfl;

import java.util.LinkedList;
import java.util.List;

public class TestRtflObjects1
{
   private static class A
   {
      private A other = null;
      private List<Integer> numbers = new LinkedList<Integer>();
   
      public void setOther (A other)
      {
         this.other = other;
      }

      public int doSomething (int n)
      {
         int r = (int)(Math.random() * 251);
         numbers.add (new Integer (r));

         if (other != null && n > 0)
            other.doSomething (n - 1);

         return r;
      }
   }

   private static class B extends A
   {
   }

   private static class C extends A
   {
   }

   public static void main (String[] args)
   {
      A x = new A ();
      B y = new B ();
      C z = new C ();

      x.setOther (y);
      y.setOther (z);
      z.setOther (x);

      x.doSomething (8);
   }
}
