@interface testMarker1 {}
@interface testMarker2 {}
@interface testMarker3 {}
@interface testMarker4 {}
@interface testMarker5 {}

@testMarker1 @testMarker2 @testMarker3 @testMarker4 @testMarker5 interface testInter {}

public class Annot10 {

    public static void main(String argv[])   {
        System.out.println(run());
    }

    public static int run() {
        return 0;
    }
}
