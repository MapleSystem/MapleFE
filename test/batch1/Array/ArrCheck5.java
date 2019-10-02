// Show two indexes add up to be the given total 19
import java.util.*;

public class ArrCheck5 {
  public static ArrayList<Integer> target_sum(final List<Integer> x, int y) {
    HashMap<Integer, Integer> map = new HashMap<Integer, Integer>();
    ArrayList<Integer> total = new ArrayList<Integer>();
    total.add(0);
    total.add(1);
 
    for (int z = 0; z < x.size(); z++) {
      if (map.containsKey(x.get(z))) {
        int idx = map.get(x.get(z));
        total.set(0, idx);
        total.set(1, z);
        break;
      } else {
        map.put(y - x.get(z), z);
      }
    }     

    return total;
  }

  public static void main(String[] args) {
    ArrayList<Integer> array1 = new ArrayList<Integer>();
    array1.add(1);
    array1.add(8);
    array1.add(7);
    array1.add(55);
    array1.add(10);
    int t = 18;
    ArrayList<Integer> r = target_sum(array1, t);

    System.out.println("Array1 : " + array1);
    System.out.println("Which indexes add up to be total of 18");
    for (int x : r) {
      System.out.print("Index : " + x + " \n" );
    }
  }
}
