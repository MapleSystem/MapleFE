// ECMAScript language types (aka primitive type, literal type, etc..)
console.log(typeof(undefined));    // undefined
console.log(typeof(null));         // object (javascrupt legacy)
console.log(typeof(true));         // boolean
console.log(typeof(false));        // boolean
console.log(typeof("abc"));        // string
console.log(typeof(Symbol()));     // symbol
console.log(typeof(123));          // number
//console.log(typeof(123n));         // bigint
console.log(typeof(BigInt(123)));  // bigint

// complex/compound type
console.log(typeof({}));           // object
console.log({} instanceof Object); // true
console.log(typeof([]));
console.log([] instanceof Array);
console.log(typeof(new Array));
console.log("====");
console.log(typeof Object());      // object
console.log(typeof new Object());  // object
console.log(Object() instanceof Object);    // true
console.log(new Object instanceof Object);  // true

// builtin objects
console.log(typeof(String()));           // string
console.log(typeof(Boolean()));          // boolean
console.log(typeof(Date()));             // string
console.log(typeof(Number()));           // number
console.log(typeof(BigInt(123)));        // bigint
console.log(typeof(new String()));       // object
console.log(typeof(new Boolean()));      // object
console.log(typeof(new Date()));         // object
console.log(typeof(new Number()));       // object

//console.log(String()  instanceof String); 
//console.log(Boolean() instanceof Boolean);
//console.log(Date()    instanceof Date);
//console.log(Number()  instanceof Number);
console.log(new String()  instanceof String);  // true
console.log(new Boolean() instanceof Boolean); // true
console.log(new Date()    instanceof Date);    // true
console.log(new Number()  instanceof Number);  // true

console.log(typeof(class {} ));          // function
console.log(class{} instanceof Function);// Function
console.log(typeof(new class {} ));      // object
console.log(new class {} instanceof Object); // object

