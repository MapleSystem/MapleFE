// Print the following grid
public class ArrGrid {
  public static void main(String[] args) {
    int [][]x = new int[5][5];
 
    for (int i = 0; i < 5; i++) {
      for (int j = 0; j < 5; j++) {
        System.out.printf("%5d ", x[i][j]);
      }
      System.out.println();
    }  
  }
}
