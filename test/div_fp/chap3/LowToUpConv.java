/*change the input from lower case to upper case or upper case to lower case.
  Print out the nubmer of changes taken place.
*/
class LowToUpConv {
  public static void main(String args[])
    throws java.io.IOException {
    int i=0, count = 0;
    System.out.println("Please input lower case or upper case letters:");
    System.out.println("Enter period sign to stop.");
   
      while (i !=46) {  //stop the program if peirod was entered.
            
        i=System.in.read();//get letters from keyboard
        if ( i>=65 && i <= 90) {
          i = i + 32;
          System.out.print((char)i);
          count++;
        }//change upper case letter to lower case letter

        else if ( i >=97 && i <= 122) {
          i=i-32;
          System.out.print((char)i);
          count++;
        }//change lower case letter to upper case letter
        else 
          System.out.print((char)i);          
      } 
        System.out.println();
        System.out.println("There are " + count + " case changes took place");   
      
   }
}

    
