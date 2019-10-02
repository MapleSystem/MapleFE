// Show the most display values
import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;
import java.util.Arrays;

public class ArrMajorVal1 {
  public static void main(String[] args) {
    int array1[] = {1, 2, 3, 8, 6, 7, 8, 8, 8, 8, 8, 8, 9};
//    int array1[] = { 1, 6, 6, 5, 7, 4, 1, 7, 7, 7, 7, 7, 7, 7, 2 };
    int r = MajorValues(array1);

    System.out.println("Array 1 : " + Arrays.toString(array1));

    if (r != -1) {
      System.out.println("Most display values is " + r);
    } else {
      System.out.println("There is no most display values exist");
    }
  }

  public static int MajorValues(int array1[]) {
    int x = array1.length;

    Map<Integer, Integer> map = new HashMap<Integer, Integer>();

    for (int y = 0; y < x; y++) {
      if (map.get(array1[y]) == null) {
        map.put(array1[y], 0);
      }
   
      map.put(array1[y], map.get(array1[y]) + 1);
    }
 
    Iterator it1 = map.entrySet().iterator();
    while (it1.hasNext()) {
      Map.Entry p = (Map.Entry)it1.next();
      if ((int)p.getValue() > x/2) {
        return (int)p.getKey();
      }
    
      it1.remove();
    }
    return -1;
  }
}
