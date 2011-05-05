// PERMUTE_ARGS:

struct ArrayRet{
   int x;
}

int arrayRetTest(int z)
{
  ArrayRet[6] w;
  int q = (w[3].x = z);
  return q;
}

static assert(arrayRetTest(51)==51);

// Bugzilla 3842 -- must not segfault
int ice3842(int z)
{
   ArrayRet w;
   return arrayRetTest((*(&w)).x);
}

static assert(true || is(typeof(compiles!(ice3842(51)))));


int arrayret2(){

  int [5] a;
  int [3] b;
  b[] = a[1..$-1] = 5;
  return b[1];
}
static assert(arrayret2()==5);

struct DotVarTest
{
   ArrayRet z;
}

struct DotVarTest2
{
   ArrayRet z;
   DotVarTest p;
}

int dotvar1()
{
    DotVarTest w;
    w.z.x = 3;
    return w.z.x;
}

int dotvar2()
{
    DotVarTest2[4] m;
    m[2].z.x = 3;
    m[1].p.z.x = 5;
    return m[2].z.x + 7;
}

static assert(dotvar1()==3);
static assert(dotvar2()==10);


struct RetRefStruct{
   int x;
   char c;
}

// Return value reference tests, for D2 only.

ref RetRefStruct reffunc1(ref RetRefStruct a)
{
int y = a.x;
return a;
}


ref RetRefStruct reffunc2(ref RetRefStruct a)
{
   RetRefStruct z = a;
   return reffunc1(a);
}

ref int reffunc7(ref RetRefStruct aa)
{
   return reffunc1(aa).x;
}

ref int reffunc3(ref int a)
{
    return a;
}

struct RefTestStruct
{
  RetRefStruct r;

  ref RefTestStruct reffunc4(ref RetRefStruct[3] a)
  {
    return this;
  }

  ref int reffunc6()
  {
    return this.r.x;
  }
}

ref RetRefStruct reffunc5(ref RetRefStruct[3] a)
{
   int t = 1;
   for (int i=0; i<10; ++i)
   { if (i==7)  ++t;}
    return a[reffunc3(t)];
}

int retRefTest1()
{
    RetRefStruct b = RetRefStruct(0,'a');
    reffunc1(b).x =3;
    return b.x-1;
}

int retRefTest2()
{
    RetRefStruct b = RetRefStruct(0,'a');
    reffunc2(b).x =3;
    RetRefStruct[3] z;
    RefTestStruct w;
    w.reffunc4(z).reffunc4(z).r.x = 4;
    assert(w.r.x == 4);
    w.reffunc6() = 218;
    assert(w.r.x == 218);
    z[2].x = 3;
    int q=4;
    int u = reffunc5(z).x + reffunc3(q);
    assert(u==7);
    reffunc5(z).x += 7;
    assert(z[2].x == 10);
    RetRefStruct m = RetRefStruct(7, 'c');
    m.x = 6;
    reffunc7(m)+=3;
    assert(m.x==9);
    return b.x-1;
}

int retRefTest3()
{
    RetRefStruct b = RetRefStruct(0,'a');
    auto deleg = function (RetRefStruct a){ return a;};
    typeof(deleg)[3] z;
    z[] = deleg;
    auto y = deleg(b).x + 27;
    b.x = 5;
    assert(y == 27);
    y = z[1](b).x + 22;
    return y - 1;
}

int retRefTest4()
{
    RetRefStruct b = RetRefStruct(0,'a');
    reffunc3(b.x) = 218;
    assert(b.x == 218);
    return b.x;
}


static assert(retRefTest1()==2);
static assert(retRefTest2()==2);
static assert(retRefTest3()==26);
static assert(retRefTest4()==218);

int bug4389()
{
    string s;
    dchar c = '\u2348';
    s ~= c;
    assert(s.length==3);
    dchar d = 'D';
    s ~= d;
    assert(s.length==4);
    s = "";
    s ~= c;
    assert(s.length==3);
    s ~= d;
    assert(s.length==4);
    string z;
    wchar w = '\u0300';
    z ~= w;
    assert(z.length==2);
    z = "";
    z ~= w;
    assert(z.length==2);
    return 1;
}

static assert(bug4389());

// ICE(constfold.c)
int ice4389()
{
    string s;
    dchar c = '\u2348';
    s ~= c;
    s = s ~ "xxx";
   return 1;
}

static assert(ice4389());

// ICE(expression.c)
string ice4390()
{
    string s;
    dchar c = '`';
    s ~= c;
    s ~= c;
   return s;
}

static assert(mixin(ice4390()) == ``);

// bug 5248 (D1 + D2)
struct Leaf5248 {
    string Compile_not_ovloaded() {
        return "expression";
    }
};
struct Matrix5248 {
    Leaf5248 Right;

    string Compile() {
        return Right.Compile_not_ovloaded();
    }
};

static assert(Matrix5248().Compile());

// Interpreter code coverage tests
int cov1(int a)
{
   a %= 15382;
   a /= 5;
   a = ~ a;
   bool c = (a==0);
   bool b = true && c;
   assert(b==0);
   b = false && c;
   assert(b==0);
   b = false || c;
   assert(b==0);
   a ^= 0x45349;
   a = ~ a;
   a &= 0xFF3F;
   a >>>= 1;
   a = a ^ 0x7393;
   a = a >> 1;
   a = a >>> 1;
   a = a | 0x010101;
   return a;
}
static assert(cov1(534564) == 71589);

int cov2()
{
    int i = 0;
    do{
        goto DOLABEL;
    DOLABEL:
        if (i!=0) {
            goto IFLABEL;
    IFLABEL:
            switch(i) {
            case 3:
                break;
            case 6:
                goto SWITCHLABEL;
    SWITCHLABEL:
                i = 27;
                goto case 3;
            }
            return i;
        }
        i = 6;
    } while(true);
    return 88; // unreachable
}

static assert(cov2()==27);

template CovTuple(T...)
{
  alias T CovTuple;
}

alias CovTuple!(int, long) TCov3;

int cov3(TCov3 t)
{
    TCov3 s;
    s = t;
    assert(s[0] == 1);
    assert(s[1] == 2);
    return 7;
}

static assert(cov3(1, 2) == 7);

template compiles(int T)
{
   bool compiles = true;
}

int badassert1(int z)
{
   assert(z == 5, "xyz");
   return 1;
}

size_t badslice1(int[] z)
{
  return z[0..3].length;
}

size_t badslice2(int[] z)
{
  return z[0..badassert1(1)].length;
}

size_t badslice3(int[] z)
{
  return z[badassert1(1)..2].length;
}

static assert(!is(typeof(compiles!(badassert1(67)))));
static assert(is(typeof(compiles!(badassert1(5)))));
static assert(!is(typeof(compiles!(badslice1([1,2])))));
static assert(!is(typeof(compiles!(badslice2([1,2])))));
static assert(!is(typeof(compiles!(badslice3([1,2,3])))));

/*******************************************/

size_t bug5524(int x, int[] more...)
{
    int[0] zz;
    assert(zz.length==0);
    return 7 + more.length + x;
}

//static assert(bug5524(3) == 10);


// 5722

static assert( ("" ~ "\&copy;"[0]).length == 1 );
const char[] null5722 = null;
static assert( (null5722 ~ "\&copy;"[0]).length == 1 );
static assert( ("\&copy;"[0] ~ null5722).length == 1 );

/*******************************************
 * Tests for CTFE Array support.
 * Including bugs 1330, 3801, 3835, 4050,
 * 4051, 5147, and major functionality
 *******************************************/

char[] bug1330StringIndex()
{
    char [] blah = "foo".dup;
    assert(blah == "foo");
    char [] s = blah[0..2];
    blah[0] = 'h';
    assert(s== "ho");
    s[0] = 'm';
    return blah;
}

static assert(bug1330StringIndex()=="moo");
static assert(bug1330StringIndex()=="moo"); // check we haven't clobbered any string literals

int[] bug1330ArrayIndex()
{
    int [] blah = [1,2,3];
    int [] s = blah;
    s = blah[0..2];
    int z = blah[0] = 6;
    assert(z==6);
    assert(blah[0]==6);
    assert(s[0]==6);
    assert(s== [6, 2]);
    s[1] = 4;
    assert(z==6);
    return blah;
}

static assert(bug1330ArrayIndex()==[6,4,3]);
static assert(bug1330ArrayIndex()==[6,4,3]); // check we haven't clobbered any literals

char[] bug1330StringSliceAssign()
{
    char [] blah = "food".dup;
    assert(blah == "food");
    char [] s = blah[1..4];
    blah[0..2] = "hc";
    assert(s== "cod");
    s[0..2] = ['a', 'b'];   // Mix string + array literal
    assert(blah == "habd");
    s[0..2] = "mq";
    return blah;
}

static assert(bug1330StringSliceAssign()=="hmqd");
static assert(bug1330StringSliceAssign()=="hmqd");

int[] bug1330ArraySliceAssign()
{
    int [] blah = [1,2,3,4];
    int [] s = blah[1..4];
    blah[0..2] = [7, 9];
    assert(s == [9,3,4]);
    s[0..2] = [8, 15];
    return blah;
}

static assert(bug1330ArraySliceAssign()==[7, 8, 15, 4]);

int[] bug1330ArrayBlockAssign()
{
    int [] blah = [1,2,3,4,5];
    int [] s = blah[1..4];
    blah[0..2] = 17;
    assert(s == [17,3,4]);
    s[0..2] = 9;
    return blah;
}

static assert(bug1330ArrayBlockAssign()==[17, 9, 9, 4, 5]);

char[] bug1330StringBlockAssign()
{
    char [] blah = "abcde".dup;
    char [] s = blah[1..4];
    blah[0..2] = 'x';
    assert(s == "xcd");
    s[0..2] = 'y';
    return blah;
}

static assert(bug1330StringBlockAssign() == "xyyde");

int assignAA(int x) {
    int[int] aa;
    int[int] cc = aa;
    assert(cc.values.length==0);
    assert(cc.keys.length==0);
    aa[1] = 2;
    aa[x] = 6;
    int[int] bb = aa;
    assert(bb.keys.length==2);
    assert(cc.keys.length==0); // cc is not affected to aa, because it is null
    aa[500] = 65;
    assert(bb.keys.length==3); // but bb is affected by changes to aa
    return aa[1] + aa[x];
}
static assert(assignAA(12) == 8);

template Compileable(int z) { bool OK;}

int arraybounds(int j, int k)
{
    int [] xxx = [1, 2, 3, 4, 5];
    int [] s = xxx[1..$];
    s = s[j..k]; // slice of slice
    return s[$-1];
}

int arraybounds2(int j, int k)
{
    int [] xxx = [1, 2, 3, 4, 5];
    int [] s = xxx[j..k]; // direct slice
    return 1;
}
static assert( !is(typeof(Compileable!(arraybounds(1, 14)))));
static assert( !is(typeof(Compileable!(arraybounds(15, 3)))));
static assert( arraybounds(2,4) == 5);
static assert( !is(typeof(Compileable!(arraybounds2(1, 14)))));
static assert( !is(typeof(Compileable!(arraybounds2(15, 3)))));
static assert( arraybounds2(2,4) == 1);

int bug5147a() {
    int[1][2] a = 37;
    return a[0][0];
}

static assert(bug5147a()==37);

int bug5147b() {
    int[4][2][3][17] a = 37;
    return a[0][0][0][0];
}

static assert(bug5147b()==37);

int setlen()
{
    int[][] zzz;
    zzz.length = 2;
    zzz[0].length = 10;
    assert(zzz.length == 2);
    assert(zzz[0].length==10);
    assert(zzz[1].length==0);
    return 2;
}

static assert(setlen()==2);

int[1][1] bug5147() {
    int[1][1] a = 1;
    return a;
}
static assert(bug5147() == [[1]]);
enum int[1][1] enum5147 = bug5147();
static assert(enum5147 == [[1]]);
immutable int[1][1] bug5147imm = bug5147();


// Index referencing
int[2][2] indexref() {
    int[2][2] a = 2;
    a[0]=7;

    int[][] b = [null, null];
    b[0..$] = a[0][0..2];
    assert(b[0][0]==7);
    assert(b[0][1]==7);
    int [] w;
    w = a[0];
    assert(w[0]==7);
    w[0..$] = 5;
    assert(a[0]!=[7,7]);
    assert(a[0]==[5,5]);
    assert(b[0] == [5,5]);
    return a;
}
int[2][2] indexref2() {
    int[2][2] a = 2;
    a[0]=7;

    int[][2] b = null;
    b[0..$] = a[0];
    assert(b[0][0]==7);
    assert(b[0][1]==7);
    assert(b == [[7,7], [7,7]]);
    int [] w;
    w = a[0];
    assert(w[0]==7);
    w[0..$] = 5;
    assert(a[0]!=[7,7]);
    assert(a[0]==[5,5]);
    assert(b[0] == [5,5]);
    return a;
}
int[2][2] indexref3() {
    int[2][2] a = 2;
    a[0]=7;

    int[][2] b = [null, null];
    b[0..$] = a[0];
    assert(b[0][0]==7);
    assert(b[0][1]==7);
    int [] w;
    w = a[0];
    assert(w[0]==7);
    w[0..$] = 5;
    assert(a[0]!=[7,7]);
    assert(a[0]==[5,5]);
    assert(b[0] == [5,5]);
    return a;
}
int[2][2] indexref4() {
    int[2][2] a = 2;
    a[0]=7;

    int[][2] b =[[1,2,3],[1,2,3]]; // wrong code
    b[0] = a[0];
    assert(b[0][0]==7);
    assert(b[0][1]==7);
    int [] w;
    w = a[0]; //[0..$];
    assert(w[0]==7);
    w[0..$] = 5;
    assert(a[0]!=[7,7]);
    assert(a[0]==[5,5]);
    assert(b[0] == [5,5]);
    return a;
}

static assert(indexref() == [[5,5], [2,2]]);
static assert(indexref2() == [[5,5], [2,2]]);
static assert(indexref3() == [[5,5], [2,2]]);
static assert(indexref4() == [[5,5], [2,2]]);

int staticdynamic() {
    int[2][1] a = 2;
    assert( a == [[2,2]]);

    int[][1] b = a[0][0..1];
    assert(b[0] == [2]);
    auto k = b[0];
    auto m = a[0][0..1];
    assert(k == [2]);
    assert(m == k);
    return 0;
}
static assert(staticdynamic() == 0);

int[] crashing()
{
    int[12] cra;
    return (cra[2..$]=3);
}
static assert(crashing()[9]==3);

int chainassign()
{
    int[4] x = 6;
    int[] y = new int[4];
    auto k = (y[] = (x[] = 2));
    return k[0];
}
static assert(chainassign()==2);

// index assignment
struct S3801 {
char c;
  int[3] arr;

  this(int x, int y){
    c = 'x';
    arr[0] = x;
    arr[1] = y;
  }
}

int bug3801()
{
    S3801 xxx = S3801(17, 67);
    int[] w = xxx.arr;
    xxx.arr[1] = 89;
    assert(xxx.arr[0]==17);
    assert(w[1] == 89);
    assert(w == [17, 89, 0]);
    return xxx.arr[1];
}

enum : S3801 { bug3801e = S3801(17, 18) }
static assert(bug3801e.arr == [17, 18, 0]);
immutable S3801 bug3801u = S3801(17, 18);
static assert(bug3801u.arr == [17, 18, 0]);
static assert(bug3801()==89);

int bug3835() {
    int[4] arr;
    arr[]=19;
    arr[0] = 4;
    int kk;
    foreach (ref el; arr)
    {
        el += 10;
        kk = el;
    }
    assert(arr[2]==29);
    arr[0]+=3;
    return arr[0];
}
static assert(bug3835() == 17);

auto bug5852(const(string) s) {
    string [] r;
    r ~= s;
    assert(r.length == 1);
    return r[0].length;
}

static assert(bug5852("abc")==3);

/*******************************************
             Bug 5671
*******************************************/

static assert( ['a', 'b'] ~ "c" == "abc" );

/*******************************************
        Bug 5685
*******************************************/

string bug5685() {
  return "xxx";
}
struct Bug5865 {
    void test1(){
        enum file2 = (bug5685())[0..$]  ;
    }
}

/*******************************************
        Bug 5840
*******************************************/

struct Bug5840 {
    string g;
    int w;
}

int bug5840(int u)
{   // check for clobbering
    Bug5840 x = void;
    x.w = 4;
    x.g = "3gs";
    if (u==1) bug5840(2);
    if (u==2) {
        x.g = "abc";
        x.w = 3465;
    } else {
        assert(x.g == "3gs");
        assert(x.w == 4);
    }
    return 56;
}
static assert(bug5840(1)==56);

/*******************************************
    std.datetime ICE (30 April 2011)
*******************************************/

struct TimeOfDayZ
{
public:
    this(int hour) { }
    invariant() { }
}
const testTODsThrownZ = TimeOfDayZ(0);
