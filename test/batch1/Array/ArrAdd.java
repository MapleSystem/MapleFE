// Add two matric of the same size
import java.util.Scanner;

public class ArrAdd {
  public static void main(String[] args) {
    int row, col, x, y;
    Scanner in = new Scanner(System.in);

    System.out.println("Input number of rows");
    row = in.nextInt();
    System.out.println("Input number of columnes");
    col = in.nextInt();

    int arr1[][] = new int[row][col];
    int arr2[][] = new int[row][col];
    int sum[][] = new int[row][col];

    System.out.println("Input value of first matrix");

    for (x = 0; x < row; x++) {
      for (y = 0; y < col; y++) {
        arr1[x][y] = in.nextInt();
      }
    }

    System.out.println("Inpurt value of 2nd matrix");

    for (x = 0; x < row; x++) {
      for (y = 0; y < col; y++) {
        arr2[x][y] = in.nextInt();
      }
    }

    for (x = 0; x < row; x++) {
      for (y = 0; y < col; y++) {
        sum[x][y] = arr1[x][y] + arr2[x][y];
      }
    }

    System.out.println("===== Sum of the two matrix =====");

    for (x = 0; x < row; x++) {
      for (y =0; y < col; y++) {
        System.out.print(sum[x][y] + "\t");
      } 

      System.out.println();
    }
  }
}

