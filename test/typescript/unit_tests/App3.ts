import myX from "./export-default-as2";
import * as M from "./M";
import { X } from "./export-default-as";
console.log(myX, M.getx(), X);
M.setx(3);
console.log(myX, M.getx(), X);
