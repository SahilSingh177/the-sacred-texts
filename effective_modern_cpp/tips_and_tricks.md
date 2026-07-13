# Moving to Modern C++
### Uses of {} and ()
```cpp
Widget w1; // Calls default constructor
Widget w1 = w2; // Not assignment; calls copy ctor(constructor)
w1 = w2; // Assignment; calls copy operator=
```

Braced intialization
```cpp
vector<int> v{1,3,5}
```

Non-static data members
```cpp
class W{
    private:
        int x{1}; // x's default value is 1
        int y = 1; // fine
        int z(0); // wrong (error!)
};
```

Uncopyable objects
```cpp
atomic<int> a1{0};
atomic<int> a2(0);
atomic<int> a3 = 0; //error
```

Thus, braced initializations are called "uniform" as they can be used everywhere.

Prohibition of implicit narrowing conversions among built-in types:
```cpp
double x,y,z;
int sum1{x+y+z}; // error! sum of doubles may not be accessible as int
int sum2(x+y+z); // ok! value of expression truncated as int
int sum3 = x+y+z; // same as (). ok!
```

```cpp
Widget w1(10); // calls Widget ctor with arg 10
Widget w2(); // vexing parse! no args in () thus, it declares a function named w2 that returns a widget!
Widget w3{}; // calls widget ctor with no args
```

Drawbacks of {}

```cpp
class Widget{
    public:
        Widget(int i, bool b);
        Widget(int i, double d);
};

Widget w1(10, true); // calls first ctor
Widget w2{10, true}; // also calls first ctor
Widget w3(10, 5.0); // calls second ctor
Widget w4{10, 5.0}; // also calls second ctor
```

But, if we include intializer_lists, things change vastly.

Suppose we have class structure like this:

```cpp
class Widget{
    public:
        Widget(int i, bool b);
        Widget(int i, double d);
        Widget(std::initializer_list<long double> il);
};

Widget w1(10, true); // calls first ctor
Widget w2{10, true}; // calls initializer_list (10 and true converts to long double)
Widget w3(10, 5.0); // calls second ctor
Widget w4{10, 5.0}; // calls initializer_list (10 and 5.0 converts to long double)
```

```cpp
class Widget{
    public:
        Widget(int i, bool b);
        Widget(int i, double d);
        Widget(std::initializer_list<long double> il);
        operator float() const;
};

Widget w1(w4); // calls copy ctor
Widget w2{w4}; // calls initializer_list ctor (w4 converts to float, and float converts to long double)
Widget w3(std::move(w4)); // calls move ctor
Widget w5{std::move(w4)}; // calls std::initializer_list ctor
```

```cpp
class Widget{
    public:
        Widget(int i, bool b);
        Widget(int i, double d);
        Widget(std::initializer_list<bool> il);
};

Widget w1(10, true); // calls first ctor
Widget w2{10, true}; // error! requires narrowing conversions as bool can't exactly represent either value.
```

```cpp
class Widget{
    public:
        Widget(int i, bool b);
        Widget(int i, double d);
        Widget(std::initializer_list<std::string> il);
};

Widget w1(10, true); // calls first ctor
Widget w2{10, true}; // calls first ctor as there's no way to convert ints and bools to std::strings thus compiler falls back on normal overload resolution.
```

```cpp
class Widget {
    public:
        Widget(); // Default ctor
        Widget(std::initializer_list<int> il); // Initializer list ctor
};

Widget w1{};   // Calls DEFAULT constructor, not the initializer_list!
Widget w2{{}}; // Calls initializer_list constructor with an empty list!
Widget w3      // Calls initializer_list constructor with an empty list!
```