#include "../../../ast2cpp/runtime/include/ts2cpp.h"

using namespace t2crt;

class Ctor_Vehicle;
class Ctor_Car;
class Ctor_MyCar;

class Vehicle;
class Car;
class MyCar;

extern Object Vehicle_prototype;
extern Object Car_prototype;
extern Object MyCar_prototype;

extern Ctor_Vehicle  Vehicle_ctor;
extern Ctor_Car      Car_ctor;
extern Ctor_MyCar    MyCar_ctor;

class Vehicle : public Function {
  public:
    // Vehicle.prototype props (use static)

  public:
    // Vehicle instance props
    std::string name;   
};

class Car : public Vehicle {
  public:
    // Car.prototype props (use static)

  public:
    // Car instance props
};

// C++ Class def for instance props and prototype props 
class MyCar : public Car {
  public:
    // MyCar.prototype props (use static)

  public:
    // MyCar instance props
};

// Class def for function constructors

class Ctor_Vehicle : public Ctor {
  public:
    Ctor_Vehicle(Ctor* ctor, Object* proto, Object* prototype) : Ctor(ctor, proto, prototype) {}

    Vehicle* _new() {
      Vehicle* obj = new Vehicle();
      obj->_ctor  = this;
      obj->_proto = this->_prototype;
      return obj;
    }
    void operator()(Vehicle* obj , std::string name) {
      // add instance props to instance prop list
      ClassFld<std::string Vehicle::*> field(&Vehicle::name);
      obj->AddProp("name", field.NewProp(TY_String));
      // init instance props
      obj->name = name;
    }
};

class Ctor_Car : public Ctor {
  public:
    Ctor_Car(Ctor* ctor, Object* proto, Object* prototype) : Ctor(ctor, proto, prototype) {}

    Car* _new() {
      Car* obj = new Car();
      obj->_ctor = this;
      obj->_proto = this->_prototype;
      return obj;
    }

    void operator()(Car* obj , std::string name) {
      Vehicle_ctor(obj, name);
    }
};

class Ctor_MyCar : public Ctor {
  public:
    Ctor_MyCar(Ctor* ctor, Object* proto, Object* prototype) : Ctor(ctor, proto, prototype) {}

    MyCar* _new() {
      MyCar* obj  = new MyCar();
      obj->_ctor  = this;
      obj->_proto = this->_prototype;
      return obj;
    }

    void operator()(MyCar* obj , std::string name) {
      Car_ctor(obj, name);
    }
};

// Prototype property
Object    Vehicle_prototype ((Ctor *)&Vehicle_ctor, &Object_prototype);
Object    Car_prototype     ((Ctor *)&Car_ctor,     &Vehicle_prototype);
Object    MyCar_prototype   ((Ctor *)&MyCar_ctor,   &Car_prototype);

// Function constructors 
Ctor_MyCar    MyCar_ctor   (&Function_ctor, &Car_ctor,           &MyCar_prototype);
Ctor_Car      Car_ctor     (&Function_ctor, &Vehicle_ctor,       &Car_prototype);
Ctor_Vehicle  Vehicle_ctor (&Function_ctor, &Function_prototype, &Vehicle_prototype);

// Object instances
Car* car = Car_ctor._new();
MyCar* mycar = MyCar_ctor._new();
Array* arr = Array_ctor._new();
Object* obj = Object_ctor._new();

int main(int argc, char* argv[]) {
  // InitAllProtoTypeProps(); 

  Car_ctor(car, "Tesla");
  MyCar_ctor(mycar, "Tesla Model X");
  std::vector<Object*> objs = {obj, arr, &Array_ctor, &Vehicle_ctor, &Car_ctor, &MyCar_ctor, car, mycar, &Function_ctor, &Object_ctor};
  std::vector<std::string> names = {"obj", "arr", "Array", "Vehicle", "Car", "MyCar", "car", "mycar", "Function", "Object"};
  GenerateDOTGraph(objs, names);
  return 0;
}

