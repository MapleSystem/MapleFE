enum ET {
    type = "type",
    with = "with"
}
abstract class Base<U extends ET, V extends ET> { f1: U; f2: V; }
declare class Klass extends Base<ET.type, ET.with> {}

