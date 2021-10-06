import myX, * as M from "./re-export2";
import {Y as y} from "./re-export2";
console.log(y);
console.log(myX, M.getx());
M.setx(3);
console.log(myX, M.getx());
console.log(y);
