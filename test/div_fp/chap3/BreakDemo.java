//Using break to exit a loop.
class BreakDemo {
  public static void main(String args[]) {
    int num = 99;
   // loop while i-square is less than num
    for(int i=0; i<num; i++){
      if(i*i >= num) break;//teminate loop if i*i >= 100
      System.out.print(i + " ");
    }
    System.out.println("Loop complete.");
  }
}

