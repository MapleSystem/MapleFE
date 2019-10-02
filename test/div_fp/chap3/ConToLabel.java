//Use continue with a label.
class ConToLabel {
  public static void main(String args[]) {

outerloop:
    for(int i=1; i < 9; i++) {
      System.out.println("\nOuter loop pass " + i +", Inner loop: ");
      for(int j =1; j < 9; j++) {
        if( j==4) continue outerloop; //continue outer loop
        System.out.print(j);
      }
    }
  }
}

