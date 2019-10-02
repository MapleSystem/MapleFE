@interface testMarker {}

@interface testInter {
    testMarker [] id();
    int i();
    String str();
}

public class Annot8 {

    @testInter(id = {@testMarker, @testMarker}, i = 1, str = "Novosibirsk") public void m(){
    }

    public static void main(String argv[])   {
        System.out.println(run());
    }

    public static int run() {
        return 0;
    }
}

