# Moving to Modern C++

### Uses of {} and ()

```cpp
Widget w1; // Calls default constructor
Widget w1 = w2; // Not assignment; calls copy ctor(constructor)
w1 = w2; // Assignment; calls copy operator=
```

Braced initialization

```cpp
vector<int> v{1,3,5};
```

**Vector Exception:**

```cpp
std::vector<int> v1(10, 2); // Calls non-initializer_list ctor: 10 elements, all valued 2
std::vector<int> v2{10, 2}; // Calls initializer_list ctor: 2 elements, values 10 and 2
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

But, if we include initializer_lists, things change vastly.

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
Widget w2{10, true}; // calls first ctor as there's no way to convert ints and bools to std::strings 
// thus compiler falls back on normal overload resolution.
```

```cpp
class Widget {
    public:
        Widget(); // Default ctor
        Widget(std::initializer_list<int> il); // Initializer list ctor
};

Widget w1{};   // Calls DEFAULT constructor, not the initializer_list!
Widget w2{{}}; // Calls initializer_list constructor with an empty list!
Widget w3{ {} }; // Calls initializer_list constructor with an empty list!
```

### nullptr

```cpp
void f(int);
void f(bool);
void f(void*);

f(0); // calls f(int) not f(void*)
f(NULL); // might not compile, but usually calls f(int). Never calls f(void*)
f(nullptr); // calls f(void*)
```

### typedefs vs alias

```cpp
typedef void(*FP)(int, const std::string&);

// Prefer alias
using FP = void (*)(int, const std::string&);
```

### Enums

Unscoped enums vs Scoped enums:

```cpp
enum Color {black, white, red};
auto white = false; // error! white already declared
```

```cpp
enum class Color {black, white, red};
auto white = false; // fine

Color c = white; // error! no enum named "white"
Color c = Color::white; //fine 
```

```cpp
enum class UserInfoFields{uiName, uiEmail, uiReputation};

UserInfo uInfo;

template<typename E>
// This function takes an arbitrary enumerator and can return its value as a compile-time constant
constexpr auto toUType(E enumerator) noexcept{
    return static_cast<std::underlying_type_t<E>> (enumerator);
}

auto val = std::get<toUType(UserInfoFields::uiEmail)>(uiInfo);

// otherwise we would done
// auto val = std::get<static_cast<std::size_t>(UserInfoFields::uiEmail)>(uiInfo);
```

### Delete function

In order to prevent others from calling a particular function we can declare them delete instead of private.

Delete can be used in public access level.

```cpp
template<class charT, class traits = char_traits<charT>>
class basic_ios : public ios_base {
    public:
        ...
        basic_ios(const basic_ios& ) = delete;
        basic_ios& operator = (const basic_ios&) = delete;
        ...
};
```

Deleted functions can't be used in any way. Even code in member and friend functions(can access private functions)
will fail to compile if it tries to copy basic_ios objects.

### Override

Declaring functions override is a way of telling compiler that this function is derived from base and if the function
is not in base or it doesn't have the virtual keyword attached.

```cpp
class Base {
    public:
        virtual void f1();
        void f2(); // Not virtual!
};
class Derived: public Base {
    public:
        virtual void f1() override;
        virtual void f2() override; // ERROR! Base::f2 is not virtual.
};
```

### Use const_iterators when possible

To prevent modifying the value that an iterator points to, we must use const iterators

```cpp
    std::vector<int> values;
    auto it  = std::find(values.cbegin(),values.cend(), 1983);
    values.insert(it, 1998);
```

### constexpr

For variables: Always implicitly const, and values are known during compile time (translation).

For functions: Can be evaluated at compile time if arguments are constant, otherwise run at runtime. (In C++11,
only a single return statement was allowed. Post-C++14, loops, branches, and modifying local variables are perfectly legal).

```cpp
constexpr auto arrSiz = 10; // 10 is a compile-time constant.
std::array<int, arrSiz> data; // fine

constexpr int pow(int base, int exp) noexcept{
    auto result = 1;
    for(int i=0; i<exp; i++) result*=base;
    return result;
}
```

### Thread safety

Use mutex and atomic. Both are move-only type (can't be copied only moved), meaning that adding them to a class strips
that class of its default copy operations.

(Note: In actual practice, standard library implementers delete both copy and move operations for std::mutex because moving
a lock while threads are waiting on it would break reference addresses).

For tracking a single counter or variable requiring synchronization, std::atomic is preferred as it is lock-free (on most platforms)
and less expensive than a mutex.

For synchronizing two or more variables/memory locations that must change together as a single atomic unit, use a std::mutex.

```cpp
class Point{
    public:
        double dist() const noexcept{
            ++callCount; // Safe: atomic increment
            return std::sqrt((x*x)+(y*y));
        }
    private:
        mutable std::atomic<unsigned> callCount{0};
        double x,y;
};

class Widget{
    public:
        int magicVal() const{
            std::lock_guard<std::mutex> guard(m); // lock m
            if(cacheValid) return cachedValue;
            else{
                auto val1 = comp1();
                auto val2 = comp2();
                cachedValue = val1+val2;
                cacheValue = true;
                return cachedValue;
            }   
        }
    
    private:
        mutable std::mutex m;
        mutable int cachedValue;
        mutable bool cacheValid{false};
};
```

### Special member function generation

These are the functions that C++ is willing to generate on its own.

* **Default Constructor** - Generated only if the class contains no user-defined constructors.
* **Destructor** - `noexcept` by default.
* **Copy Constructor** - Performs memberwise copy construction of non-static data members. Generated only if the class lacks a
user-declared copy constructor. Deleted if the class declares a move operation.
* **Copy assignment operator** - Performs memberwise copy assignment of non-static data members. Generated only if the class
lacks a user-declared copy assignment operator. Deleted if the class declares a move operation.
* **Move constructor & Move assignment operator** - Each performs memberwise moving of non-static data members. Generated only
if the class contains **no declared copy operations, move operations, or destructor.**

Generated special member functions are implicitly `public`, `inline`, and non-virtual (unless it's a destructor in a derived
class inheriting from a base class with a virtual destructor).

**Memberwise move** consists of move operations on data members and base classes that support move operations, but silently
falls back to a copy operation for those that don't.

**Independence of operations:**

* The two copy operations are independent. If we declare a copy constructor but no copy assignment operator, the compiler will
still generate the copy assignment operator for us (though doing so is usually bad design).
* The move operations are **not** independent. Declaring a move constructor prevents the compiler from generating a move assignment
operator (and vice versa).

### `= default`

Because the rules above suppress function generation, we use `= default` to explicitly tell the compiler, "I know I declared one of
these, but generate the others anyway using your default memberwise logic."

By explicitly declaring a `virtual` destructor, we accidentally trigger the rule that suppresses move operations. To make the class
movable (and copyable) again, we must explicitly opt back in using `= default`:

```cpp
class Base {
public:
    // Declaring this destructor suppresses generated move operations
    virtual ~Base() = default; 

    // Explicitly resurrecting move operations
    Base(Base&&) = default; 
    Base& operator=(Base&&) = default;

    // Explicitly resurrecting copy operations 
    // (technically generation of copy ops is only deprecated, not deleted by the dtor, 
    // but specifying it is best practice).
    Base(const Base&) = default; 
    Base& operator=(const Base&) = default; // For both move ctor and op, the source still exists, but you must not assume it 
    // contains its old value.
    // Its main difference from the move constructor is that the destination already owns something, so it must deal with 
    // its current resource first.
};
```

# R-value References, Move Semantics, and Perfect Forwarding

## L-values and R-values

### L-value

An l-value expression refers to an object that has an identifiable location or identity.

Characteristics:

* Refers to an existing object.
* Its address can usually be taken.
* It can generally be used again in later expressions.
* An expression consisting of a variable's name is an l-value.

```cpp
int x = 10;

x = 20;       // x is an l-value expression
int* p = &x;  // Its address can be taken
```

Other examples:

```cpp
x
str
vec[0]
*ptr
```

Being able to appear on the left-hand side of an assignment is not the formal definition of an l-value. For example, a `const`
object is still an l-value even though it cannot be assigned to.

```cpp
const int value = 10;
// value = 20; // Error, but value is still an l-value
```

---

### R-value

An r-value expression represents either:

* A temporary or newly computed value, called a **prvalue**.
* An existing object whose resources may be reused, called an **xvalue**.

Examples of prvalues:

```cpp
10
x + 5
std::string("Hello")
foo() // If foo returns an object by value
```

Example of an xvalue:

```cpp
std::move(x)
```

An r-value does not necessarily lack identity. An xvalue refers to an existing object that still has identity.

```text
r-value
├── prvalue: temporary or newly computed value
└── xvalue: existing object treated as expiring or movable
```

---

## R-value References

An r-value reference has the form:

```cpp
T&&
```

It can bind to an r-value expression.

```cpp
std::string&& ref1 = std::string("hello");
```

`std::string("hello")` is a temporary expression and a prvalue.

```cpp
std::string name = "Hi";
std::string&& ref2 = std::move(name);
```

Here:

* `name` is an l-value expression.
* `std::move(name)` is an xvalue.
* `ref2` has type `std::string&&`.
* The expression `ref2` is an l-value because it is a named variable.

```cpp
process(ref2);             // Passes an l-value
process(std::move(ref2));  // Passes an xvalue
```

Important distinction:

```text
The type of ref2 is std::string&&.
The expression ref2 is an l-value.
```

---

## `std::move`

`std::move` does not perform a move.

It casts its argument into an xvalue, allowing move operations to be selected.

```cpp
std::string a = "hello";
std::string b = std::move(a);
```

Conceptually:

```cpp
static_cast<std::string&&>(a)
```

The actual resource transfer is performed by the receiving operation, such as:

* A move constructor.
* A move assignment operator.
* A function accepting and consuming an r-value reference.

This expression alone does not transfer anything:

```cpp
std::move(a);
```

But the result does not necessarily need to be stored. Passing it to a function can also consume it:

```cpp
process(std::move(a));
container.push_back(std::move(a));
```

A simplified implementation resembles:

```cpp
template <typename T>
constexpr std::remove_reference_t<T>&& move(T&& value) noexcept {
    return static_cast<std::remove_reference_t<T>&&>(value);
}
```

`std::remove_reference_t<T>` is used before adding `&&`.

Without removing the reference, reference collapsing could turn `T&&` into an l-value reference when `T` itself is an l-value reference.

For example:

```cpp
T = std::string&
T&& = std::string& && = std::string&
```

After removing the reference:

```cpp
std::remove_reference_t<T> = std::string
std::remove_reference_t<T>&& = std::string&&
```

Therefore, `std::move` unconditionally produces an xvalue expression.

### Moving from `const`

Move constructors commonly accept non-const r-value references:

```cpp
Widget(Widget&& other);
```

A `const Widget` cannot bind to `Widget&&`.

```cpp
const Widget a;
Widget b = std::move(a);
```

`std::move(a)` has type `const Widget&&`.

Because a normal move constructor cannot modify or steal resources from a const object, a copy constructor taking `const Widget&`
is often selected instead, if one exists.

Therefore, avoid making an object `const` when you intend to move resources from it.

However, it is not a language rule that every move request on a const object automatically becomes a copy. It depends on the available overloads.

---

## `std::forward`

`std::forward` is a conditional cast.

It is mainly used inside templates with forwarding references to preserve whether the caller originally supplied an l-value or an r-value.

```cpp
template <typename T>
void wrapper(T&& value) {
    process(std::forward<T>(value));
}
```

Here, `T&&` is a forwarding reference because:

* `T` is deduced by the function template.
* The parameter has the exact form `T&&`.

Examples:

```cpp
std::string name = "Hi!";

wrapper(name);
```

`name` is an l-value, so `T` is deduced as:

```cpp
T = std::string&
```

`std::forward<T>(value)` produces an l-value.

```cpp
wrapper(std::string("hello"));
```

The temporary is a prvalue. `T` is deduced as:

```cpp
T = std::string
```

`std::forward<T>(value)` produces an r-value.

```cpp
wrapper(std::move(name));
```

`std::move(name)` is an xvalue. `T` is deduced as:

```cpp
T = std::string
```

`std::forward<T>(value)` produces an r-value.

Inside `wrapper`, the expression `value` is always an l-value because it has a name.

Without forwarding:

```cpp
template <typename T>
void wrapper(T&& value) {
    process(value);
}
```

`process` receives an l-value in every case.

With forwarding:

```cpp
template <typename T>
void wrapper(T&& value) {
    process(std::forward<T>(value));
}
std::string name = "Hi!"; 
wrapper(name); // forwards as l-value 
wrapper(std::string("hello")); // forwards as r-value, creates a temporary object 
wrapper(std::move(name)); // forwards as r-value, xvalue (rvalue)
```

The caller's original value category is preserved.

---

## `std::move` vs `std::forward`

### `std::move`

```cpp
std::move(value)
```

* Requires only the expression being cast.
* Unconditionally treats the expression as an xvalue.
* Expresses that the current value may be moved from.
* Usually used when the programmer deliberately gives up the source object's current contents.

### `std::forward`

```cpp
std::forward<T>(value)
```

* Requires both a template type argument and an expression.
* Preserves the value category represented by `T`.
* Produces an l-value when `T` is an l-value reference.
* Produces an r-value when `T` is not an l-value reference.
* Usually used in generic forwarding code.

Note:

```text
std::move means: treat this object as movable.

std::forward means: preserve how the caller originally passed this object.
```

### rvalue reference vs universal(forwarding) reference

```cpp
void f(Widget&& param); // rvalue ref
Widget&& var1 = Widget(); // rvalue ref

auto&& var2 = var1; // forwarding ref. auto has type deduction

template<typename T>
void f(T&& param); // forwarding ref, type deduction of param.


template<typename T>
void f(std::vector<T>&& param); // rvalue ref. param's type declaration is vector<T>&&.

template<typename T>
void f(const T&& param); // rvalue ref. param's type declaration is const T&&.
```

---

`push_back vs emplace_back ref types`

```cpp
template<class T, class Allocator = allocator<T>>
class vector {
public:
    void push_back(T&& x); // rvalue ref.
    // Because by the time we call push_back, the class has already been instantiated and T is fully known. There is no type 
    // deduction happening when the function is called.
};

std::vector<Widget> v; 
// The compiler instantiates the class. The function signature becomes:
// void push_back(Widget&& x); 
// No deduction is happening here anymore!

template<class T, class Allocator = allocator<T>>
class vector {
public:
    template<class... Args>
    void emplace_back(Args&&... args); // forwarding reference! Args is deduced per call.
};
```

---

If we are in a function that returns by value, and we are returning an object bound to an rvalue reference or a forwarding
reference, we'll want to apply std::move or std::forward when we return the reference.

```cpp
Matrix operator+(Matrix&& lhs, const Matrix& rhs){
    lhs+=rhs;
    return std::move(lhs);
}
```

using std::move in the return statement yields more efficient code.

### Return Value Optimization (RVO)

Compilers may skip over the copying or moving of a local object in a function that returns by value if:

* the type of the local object is the same as that returned by the function.
* the local object is what's being returned.

```cpp
Widget makeWidget(){
    Widget w;
    ...
    return w; // "copy" w into return value.
}
```

If the compilers choose not to perform copy elision, the object being returned must be treated as an rvalue. Thus, either copy
elision takes place or std::move is implicitly applied to local objects being returned.

---

### Overloading on forwarding references

An exact match beats a match with a promotion, so the forwarding reference overload is invoked. Although, in situations where a
template instantiation and a non-template function(normal func) are equally good matches for a function call, the normal function is preferred.

```cpp
std::multiset<std::string> names;
std::string nameFromIdx(int idx); // Helper function

// Overload 1: forwarding Reference
template<typename T>
void logAndAdd(T&& name) {
    names.emplace(std::forward<T>(name));
}

// Overload 2: Normal Function (Non-template)
void logAndAdd(int idx) {
    names.emplace(nameFromIdx(idx));
}

// 1. Passes a string literal
logAndAdd("Hello"); 
// Resolves to Overload 1. (T is deduced as const char[6]). 

// 2. Passes an int
int x = 22;
logAndAdd(x); 
// Resolves to Overload 2. 
// WHY: Template instantiation logAndAdd<int&>(int&) and normal logAndAdd(int) 
// are both exact matches. Normal function is preferred!

// 3. Passes a short
short s = 5;
logAndAdd(s); 
// Resolves to Overload 1! (And causes a compile error inside the template).
// Reasons: 
// - Overload 1 creates an exact match: logAndAdd<short&>(short&).
// - Overload 2 requires a promotion: short -> int.
// An exact template match beats a normal function that requires a promotion.
// Inside the template, it tried to pass a short to names.emplace(), which expects a string, causing a massive compilation error.
```

### Tag Dispatch

If the forwarding reference is part of a parameter list containing other parameters that are not forwarding references, sufficiently
poor matches on the non-forwarding reference parameters can knock an overload with a forwarding reference out.

```cpp
template<typename T>
void logAndAdd(T&& name){
    logAndAddImpl(std::forward<T>(name), typename std::is_integral<std::remove_reference_t<T>>::type()); 
    // remove reference is used to deal with 
    // lvalue argument being passed to the forwarding reference name, as T will be deduced to be an lvalue reference. This removes the 
    // reference qualifier from a type.
}

template<typename T>
void logAndAddImpl(T&& name, std::false_type){ // we don't use true and false as they are runtime values, and since we need to use 
// overload resolution which is a compile-time phenomenon we use this.
    auto now = std::chrono::system_clock::now();
    log(now, "logAndAdd");
    names.emplace(std::forward<T>(name));
}

std::string nameFromIndex(int idx);

void logAndAddImpl(int idx, std::true_type){
    logAndAdd(nameFromIndex(idx));
}
```

`std::true_type` and `std::false_type` are 'tags' whose only purpose is to force overload resolution to go the way we want.

The same can't be used in forwarding constructor as compilers may generate copy and move constructors themselves,
so even if we write only one constructor and use tag dispatch within it, some constructor calls may be handled by compiler-generated functions
that bypasses the tag dispatch system.

When a base class declares a perfect-forwarding constructor, that constructor will typically be called when derived classes implement
their copy and move constructors in the conventional fashion, even though the correct behavior is for the base class's copy and move
constructors to be invoked.

`std::enable_if` helps us here as using this the template is enabled only if the condition is met.

`enable_if` works on the technology `SFINAE(Substitution Failure Is Not An Error)`. If the condition is `true`, it creates a valid
pathway for the compiler. If it is `false`, the pathway disappears forcing the compiler to look elsewhere.

`std::decay<T>::type` removes the references and cv-qualifiers(i.e `const` or `volatile` qualifiers). It also turns array and
function types into pointers.

To know whether a type was derived from some another type we use `std::is_base_of<T1,T2>::value`, its `true` if `T2` is derived from `T1`.
`std::is_base_of<T,T>::value` is true, as types are considered to be derived from themselves.

```cpp
class Person{
    public:
        template<typename T,
            typename = std::enable_if_t<!std::is_base_of<Person,std::decay_t<T>>::value>
        >
        explicit Person(T&& n);
        ...
};
```

Now in the above code if we wanted a overloaded function that handles int values we would want that the templatized constructor
is disabled for such arguments.

```cpp
class Person{
    public:
        template<typename T,
            typename = std::enable_if_t<
                !std::is_base_of<Person,std::decay_t<T>>::value 
                && !std::is_integral<std::remove_reference_t<T>>::value
            >
        >
        explicit Person(T&& n) // ctor for std::strings and args convertible to std::strings
            : name(std::forward<T>(n))
        {
        ...
        }

        explicit Person(int idx) // ctor for integral args
            : name(nameFromIdx(idx))
            {...}

    private:
        std::string name;
};
```

To check whether an object of one type can be constructed from an object (or set of objects) of a different type (or set of types),
we use `std::is_constructible` type trait that performs a compile-time test for the same.

```cpp
class Person{
    public:
        template<typename T,
            typename = std::enable_if_t<
                !std::is_base_of<Person,std::decay_t<T>>::value 
                && !std::is_integral<std::remove_reference_t<T>>::value
            >
        >
        explicit Person(T&& n) // ctor for std::strings and args convertible to std::strings
            : name(std::forward<T>(n))
        {
            static_assert(std::is_convertible<T, std::string>::value,"Parameter n can't be used to construct a std::string");
        }
};
```

---

### Reference collapsing

We are forbidden from declaring references to references, but compilers may produce them in particular contexts, template instantiation being among them. When compilers generate references to references, reference
collapsing dictates what happens next.

```text
If either reference is an lvalue reference, the result is an lvalue reference. Otherwise, (i.e both are rvalue references) the result is an rvalue reference.
```

**The Collapsing Rules:**

* `T&` + `&` $\rightarrow$ `T&`
* `T&` + `&&` $\rightarrow$ `T&`
* `T&&` + `&` $\rightarrow$ `T&`
* `T&&` + `&&` $\rightarrow$ `T&&`

```text
Reference collapsing occurs in four contexts: template instantiation, auto type generation, creation and use of typedefs and alias declarations, and decltype.
```

```cpp
// 1. Template Instantiation
template<typename T>
void func(T&& param); 

int x = 10;
func(x);  // x is lvalue: T deduced as int&. (int& && collapses to int&)
func(10); // 10 is rvalue: T deduced as int.  (int&& collapses to int&&)

// 2. auto Type Generation
int w = 20;
auto&& v1 = w;  // w is lvalue: auto deduced as int&. (int& && collapses to int&)
auto&& v2 = 20; // 20 is rvalue: auto deduced as int.  (int&& collapses to int&&)

// 3. typedefs and Alias Declarations
template<typename T>
struct Widget {
    using LvalueRefType = T&;
    using RvalueRefType = T&&;
};

Widget<int&> wid; 
// T is int&. 
// LvalueRefType becomes int& &  -> collapses to int&
// RvalueRefType becomes int& && -> collapses to int&

// 4. decltype
int z = 30;
decltype((z))&& ref = z; 
// decltype((z)) evaluates to int& (because (z) is an lvalue expression). 
// Thus we get int& &&, which collapses to int&.
```

---

## Perfect Forwarding

Perfect forwarding means not just forwarding objects, but also the salient characteristics like type, whether it's an lvalue or rvalue, and const or volatile.

Perfect forwarding fails if calling a function with a particular argument does one thing, but calling it via forwarding does something different.

Perfect forwarding fails when:

1. Compilers are unable to deduce a type.
2. Compilers deduce the wrong type.

### Failure Case 1: Overloaded Function Names and Template Names

Note: Manually specify the overload or instantiation you want to have forwarded.

```cpp
int processVal(int value);
int processVal(int value, int priority);

void f(int (*pf)(int)); // pf = processing function

template<typename T>
void fwd(T&& param){
    f(std::forward<T>(param));
}

f(processVal); //fine as compiler knows which processVal to match to!
fwd(processVal); // error! which processVal?

template<typename T>
T workOnVal(T param){
    // ...
}

fwd(workOnVal); // error! which workOnVal instantiation?

// fix for above errors
using ProcessFuncType = int (*)(int);

ProcessFuncType processValPtr = processVal; // specify needed signature for processVal

fwd(processValPtr); // fine!

fwd(static_cast<ProcessFuncType>(processVal)); // fine
```

---

### Failure Case 2: Braced Initializers

Compilers are forbidden from deducing the type `std::initializer_list` in a generic template context like perfect forwarding.

```cpp
void f(const std::vector<int>& v);

f({ 1, 2, 3 });   // Fine: implicitly converts to std::vector<int>
fwd({ 1, 2, 3 }); // Error! Cannot deduce type T for the braced list

// Fix: Declare a local variable first
auto il = { 1, 2, 3 }; // Deduces to std::initializer_list<int>
fwd(il);               // Fine!
```

### Failure Case 3: `0` or `NULL` as Null Pointers

If you try to forward `0` or `NULL` to a function expecting a pointer, perfect forwarding fails because `T` deduces them as integral types (`int`), not pointers.
**Fix:** Always use `nullptr` (which perfectly forwards as `std::nullptr_t`).

### Failure Case 4: Declaration-Only Integral `static const` Data Members

If a class has a `static const` data member that is only *declared* in the class but not *defined* in a `.cpp` file, passing it to `fwd` fails during linking. 

Forwarding requires taking a reference, and taking a reference to a variable requires it to have a physical memory address (a definition).

```cpp
class Widget {
public:
    static const std::size_t MinVals = 28; // Declaration
};

void f(std::size_t val);

f(Widget::MinVals);   // Fine: treated as a compile-time constant by value
fwd(Widget::MinVals); // Linker Error! fwd takes a reference, requiring an address

// Fix: Force a copy by value, or define MinVals in a .cpp file
fwd(static_cast<std::size_t>(Widget::MinVals)); // Fine
```

### Failure Case 5: Bitfields

You cannot take a non-const reference to a bitfield. Since perfect forwarding uses references (`T&&`), it is impossible to perfectly forward a bitfield directly.

```cpp
struct IPv4Header {
    std::uint32_t version:4,
                  IHL:4,
                  DSCP:6,
                  ECN:2,
                  totalLength:16;
};

void f(std::size_t sz); 
IPv4Header h;

f(h.totalLength);   // Fine: passed by value
fwd(h.totalLength); // Error! Cannot bind a reference to a bitfield

// Fix: Make a copy first
auto length = static_cast<std::uint16_t>(h.totalLength);
fwd(length);        // Fine
```

---

# Lambda Expressions

Each lambda causes compilers to generate a unique class called a `closure class`. The statements inside a lambda become executable instructions
in the member functions of its closure class. `closure` is the object that's passed at runtime.

```cpp
int x;
auto c1 = [x](int y){return x*y>25;}; // c1 is copy of the closure 
                                      // produced by the lambda
auto c2 = c1; // c2 is the copy of c1.
```

### Avoid default capture modes in lambda expression

**Why to avoid `[&]` (Dangling references):**

```cpp
auto getLambda() {
    int localVal = 10;
    return [&] { return localVal; }; // ERROR! localVal dies, closure holds a dangling reference.
}
```

**Why to avoid `[=]` (Accidental `this` capture):**
Using `[=]` inside a class member function doesn't capture the member variables directly; it silently captures the `this` pointer by value!

Do this instead:

```cpp
void Widget::addFilter() const{
    auto divisorCopy = divisor; // copy data member 

    filters.emplace_back([divisorCopy](int value){return value%divisorCopy==0;}); // capture the copy use the copy

    filters.emplace_back([=](int value){return value%divisorCopy==0;}); // fine! capture the copy use the copy
}
```

Another better way to do it (C++14 Init Capture):

```cpp
void Widget::addFilter() const{
    filters.emplace_back([divisor=divisor](int value){return value%divisor==0;}); // copy divisor to closure use the copy
}
```

---

Using an `init capture` makes it possible to specify:

1. The name of a data member in the closure class generated from the lambda.
2. an expression initializing that data member.

```cpp
class Widget{
    public:
        bool isValidated() const;
        bool isProcessed() const;
        bool isArchived() const;
};

auto pw = std::make_unique<Widget>(); // create widget
...
auto func = [pw = std::move(pw)]{return pw->isValidated() && pw->isArchived();}; // init data member in closure with std::move(pw)
```

---

`Generic lambdas` are lambdas that use auto in their params specifications.

```cpp
auto f = [](auto&& param){
    return func(normalize(std::forward<decltype(param)>(param)));
};
// decltype is being used to make it perfect forwarding
```

Note: Prefer lambdas to `std::bind` as it's more readable, expressive and may be more efficient.

```cpp
// Lambda (Clear and readable)
auto setSoundL = [](int b) { setAlarm(b, std::chrono::hours(1)); };

// std::bind (Obscure, requires placeholders, can be harder for compiler to inline)
auto setSoundB = std::bind(setAlarm, std::placeholders::_1, std::chrono::hours(1));
```