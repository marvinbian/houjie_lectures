#include <cstddef>
#include <iostream>
#include <complex>
using namespace std;

namespace jj09
{

class allocator
{
private:
	struct obj {
		struct obj* next;  //embedded pointer
	};
public:
	void* allocate(size_t);
	void  deallocate(void*, size_t);
	void  check();

private:
	obj* freeStore = nullptr;
	const int CHUNK = 5; //小一點方便觀察
};

void* allocator::allocate(size_t size)
{
	obj* p;

	if (!freeStore) {
		//linked list 是空的，所以攫取一大塊 memory
		size_t chunk = CHUNK * size;
		freeStore = p = (obj*)malloc(chunk);

		//cout << "empty. malloc: " << chunk << "  " << p << endl;

		//將分配得來的一大塊當做 linked list 般小塊小塊串接起來
		for (int i = 0; i < (CHUNK - 1); ++i)	{  //沒寫很漂亮, 不是重點無所謂.
			p->next = (obj*)((char*)p + size);
			p = p->next;
		}
		p->next = nullptr;  //last
	}
	p = freeStore;
	freeStore = freeStore->next;

	//cout << "p= " << p << "  freeStore= " << freeStore << endl;

	return p;
}
void allocator::deallocate(void* p, size_t)
{
	//將 deleted object 收回插入 free list 前端
	((obj*)p)->next = freeStore;
	freeStore = (obj*)p;
}
void allocator::check()
{
	obj* p = freeStore;
	int count = 0;

	while (p) {
		cout << p << endl;
		p = p->next;
		count++;
	}
	cout << count << endl;
}
//--------------

// DECLARE_POOL_ALLOC -- used in class definition
#define DECLARE_POOL_ALLOC() \
public: \
  void* operator new(size_t size) { return myAlloc.allocate(size); } \
  void operator delete(void* p) { myAlloc.deallocate(p, 0); }\
protected: \
  static allocator myAlloc;

// INPLEMENT_POOL_ALLOC -- used in class inplementation file
#define IMPLEMENT_POOL_ALLOC(class_name) \
allocator class_name::myAlloc;

class Foo {
  DECLARE_POOL_ALLOC()
public:
  long L;
  std::string str;
public:
  Foo(long l) : L(l) {}
};
IMPLEMENT_POOL_ALLOC(Foo);

class Goo {
  DECLARE_POOL_ALLOC()
public:
  complex<double> c;
  std::string str;
public:
  Goo(const complex<double>& x) : c(x) {}
};
IMPLEMENT_POOL_ALLOC(Goo);

} //namespace
