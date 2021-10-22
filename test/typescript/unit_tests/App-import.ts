import myX, * as M from "./M";
import getx = M.getx;
console.log(myX, getx());
M.setx(3);
console.log(myX, getx());
