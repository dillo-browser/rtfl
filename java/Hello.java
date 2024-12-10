package rtfl;

public class Hello
{
   int i1;
   Integer i2;
   String s1;

   public static void main (String[] args)
   {
      new Hello().hello();
   }

   public void hello()
   {
      System.out.println ("===== Hello Java! =====");
      i1 = 5;
      i2 = new Integer(7);
      s1 = "Hi!";
   }
}
